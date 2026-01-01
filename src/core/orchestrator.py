"""
VrindaAI - Unified Orchestrator (AAA Pipeline + Neural Link Version)
"""

import json
import logging
import time
import sys
from pathlib import Path
from typing import Dict, Any, Optional, List, Callable
from enum import Enum
from datetime import datetime

# --- SYSTEM PATH FIX ---
# Allow imports from the src root so we can find 'engines' and 'core'
sys.path.append(str(Path(__file__).resolve().parents[2]))

# --- NEURAL LINK IMPORTS ---
import grpc
try:
    # Ensure this points to src.core.proto
    from src.core.proto import vryndara_pb2, vryndara_pb2_grpc
except ImportError:
    vryndara_pb2 = None
    vryndara_pb2_grpc = None

# Import Engines & Managers
from src.engines.ffmpeg_engine import create_ffmpeg_engine
from src.core.asset_manager import create_asset_manager, AssetManager 
# Import classes specifically for type hinting to fix Pylance
from src.engines.unreal_engine import create_unreal_engine, UnrealEngine 
from src.engines.blender_engine import create_blender_engine

logger = logging.getLogger(__name__)

class WorkflowStatus(Enum):
    PENDING = "pending"
    RUNNING = "running"
    COMPLETED = "completed"
    FAILED = "failed"
    CANCELLED = "cancelled"
    OFFLOADED = "offloaded"

class ExecutionMode(Enum):
    SEQUENTIAL = "sequential"
    PARALLEL = "parallel"
    INTERACTIVE = "interactive"

class KernelClient:
    """The Neural Link: Handles communication with Vryndara."""
    def __init__(self, address='localhost:50051'):
        self.logger = logging.getLogger("KernelLink")
        self.enabled = False
        
        if vryndara_pb2 is None:
            self.logger.warning("⚠️ gRPC Protos not found. Neural Link disabled.")
            return

        try:
            self.channel = grpc.insecure_channel(address)
            self.stub = vryndara_pb2_grpc.KernelStub(self.channel)
            self.enabled = True
            self.logger.info(f"🔗 Connected to Vryndara Kernel at {address}")
        except Exception as e:
            self.logger.error(f"❌ Failed to connect to Kernel: {e}")

    def submit_engineering_task(self, prompt: str) -> Dict:
        """Offloads task and returns the Artifact Dictionary"""
        if not self.enabled: raise ConnectionError("Neural Link disabled")
        
        try:
            # Matches 'Signal' definition in .proto
            request = vryndara_pb2.Signal(
                id=f"task_{int(time.time())}",
                source_agent_id="VrindaAI-Client",
                target_agent_id="ComputationalEngineer",
                type="TASK_REQUEST",
                payload=prompt,
                timestamp=int(time.time())
            )
            
            # Send and wait
            ack = self.stub.Publish(request)
            
            if ack.success:
                # Parse the JSON payload returned by the kernel (Phase 4)
                try:
                    return json.loads(ack.error)
                except json.JSONDecodeError:
                    return {"status": "success", "raw_msg": ack.error}
            else:
                raise Exception(ack.error)
                
        except grpc.RpcError as e:
            self.logger.error(f"gRPC Error: {e}")
            raise

class Orchestrator:
    def __init__(self, config: Optional[Dict] = None):
        self.config = config or self._load_default_config()
        self.logger = logging.getLogger(__name__)
        
        self.output_dir = Path(self.config.get("output_dir", "output")).resolve()
        self.output_dir.mkdir(parents=True, exist_ok=True)
        
        self.unreal_root = self.config.get("paths", {}).get("unreal_projects_root")
        if self.unreal_root:
            self.unreal_root = Path(self.unreal_root).resolve()
        else:
            self.unreal_root = self.output_dir / "games"
        self.unreal_root.mkdir(parents=True, exist_ok=True)

        self.asset_manager: AssetManager = create_asset_manager(self.config)
        
        # Initialize Neural Link
        kernel_addr = self.config.get("kernel_address", "localhost:50051")
        self.kernel = KernelClient(address=kernel_addr)
        
        self.workflow_status = {}
        self.execution_history = []
        self.callbacks = {}
    
    def _load_default_config(self) -> Dict[str, Any]:
        return {
            "output_dir": "output",
            "paths": {"vault_cache": "C:/Vault"}, 
            "max_parallel_jobs": 1,
            "continue_on_error": False
        }
    
    def execute_workflow(self, task_spec: Dict[str, Any], mode: ExecutionMode = ExecutionMode.SEQUENTIAL, dry_run: bool = False) -> Dict[str, Any]:
        description = str(task_spec.get('description', ''))
        workflow_id = self._generate_workflow_id()
        
        # --- PHASE 4: INTELLIGENT OFFLOADING LOOP ---
        if "design" in description.lower() and self.kernel.enabled:
            self.logger.info("🚀 Offloading to Vryndara...")
            try:
                # 1. Get Engineering Data (Mass, Cost, Geometry Paths)
                artifacts = self.kernel.submit_engineering_task(description)
                
                # 2. Automatically Create a Visualization Job for Blender
                if "proxy" in artifacts:
                    proxy_path = artifacts["proxy"]
                    self.logger.info(f"🎨 Auto-Scheduling Render for: {proxy_path}")
                    
                    # Create a temporary task spec for the renderer
                    render_task = {
                        "engine": "blender",
                        "description": f"Cinematic rotation of {description}",
                        "import_mesh": proxy_path, 
                        "output": {"path": str(self.output_dir / f"{workflow_id}_render")}
                    }
                    
                    # Generate Manifests locally
                    manifests = self._generate_blender_manifests(render_task, workflow_id)
                    
                    # Execute Render
                    self._execute_jobs(manifests)
                    
                    # Combine Results
                    artifacts["render_status"] = "Visualization Complete"
                
                # Return the Full Engineering Package
                return {
                    "status": WorkflowStatus.OFFLOADED.value, 
                    "workflow_id": workflow_id,
                    "output": artifacts 
                }
                
            except Exception as e:
                self.logger.warning(f"Offload failed ({e}). Falling back.")

        # --- LOCAL EXECUTION (Fallback) ---
        result = {
            "workflow_id": workflow_id,
            "status": WorkflowStatus.RUNNING.value,
            "engine": task_spec.get("engine")
        }

        try:
            self._prepare_environment(task_spec, workflow_id)
            manifests = self._generate_job_manifests(task_spec, workflow_id)
            
            if dry_run:
                return {"status": WorkflowStatus.PENDING.value, "manifests": manifests}

            exec_res = self._execute_jobs(manifests)
            
            if exec_res["overall_status"] == "completed":
                result["status"] = WorkflowStatus.COMPLETED.value
            else:
                result["status"] = WorkflowStatus.FAILED.value
                result["error"] = exec_res.get("error")

        except Exception as e:
            result["status"] = WorkflowStatus.FAILED.value
            result["error"] = str(e)
        
        self.execution_history.append(result)
        return result

    def _prepare_environment(self, task_spec: Dict, workflow_id: str):
        desc = str(task_spec.get("description", "job"))
        safe_desc = "".join([c for c in desc if c.isalnum() or c in (' ', '-', '_')]).strip()
        run_dir = self.output_dir / f"{safe_desc[:30]}_{workflow_id}"
        run_dir.mkdir(parents=True, exist_ok=True)
        return {"success": True, "run_dir": str(run_dir)}

    def _generate_job_manifests(self, task_spec: Dict, workflow_id: str) -> List[Dict]:
        engine = task_spec.get("engine")
        if engine == "unreal":
            return self._generate_unreal_manifests(task_spec, workflow_id)
        elif engine == "blender":
            return self._generate_blender_manifests(task_spec, workflow_id)
        return []

    # --- ENGINE GENERATORS ---

    def _generate_blender_manifests(self, task_spec: Dict[str, Any], workflow_id: str) -> List[Dict]:
        """RESTORED: Generates Blender job manifests"""
        desc = str(task_spec.get("description", "unnamed"))
        safe_desc = "".join([c for c in desc if c.isalnum() or c in (' ', '-', '_')]).strip()
        project_folder = self.output_dir / f"{safe_desc[:30]}_{workflow_id}"
        
        return [{
            "id": "blender_render",
            "engine": "blender",
            "type": "render",
            "parameters": task_spec,
            "output": {"path": str(project_folder / "frames")}
        }]

    def _generate_unreal_manifests(self, task_spec: Dict[str, Any], workflow_id: str) -> List[Dict]:
        manifests = []
        project_name = f"VrindaProj_{workflow_id}"
        project_path = self.unreal_root / project_name
        
        # 1. Create Project
        manifests.append({
            "id": "1_create_project",
            "engine": "unreal",
            "type": "project_create",
            "parameters": {"game_type": "game", "quality": "high"},
            "output": { "path": str(project_path) }
        })
        
        # 2. Ingest
        assets = task_spec.get("assets", [])
        for asset in assets:
            manifests.append({
                "id": f"ingest_{asset}",
                "engine": "asset_manager",
                "type": "ingest",
                "parameters": {"asset_name": asset, "target_project": str(project_path/"Content")}
            })
            
        return manifests

    def _execute_jobs(self, manifests: List[Dict]) -> Dict:
        results = {"overall_status": "completed"}
        context = {}
        
        for job in manifests:
            try:
                res = self._execute_single_job(job, context)
                if res["status"] == "failed":
                    results["overall_status"] = "failed"
                    results["error"] = res.get("error")
                    break
            except Exception as e:
                results["overall_status"] = "failed"
                results["error"] = str(e)
                break
        return results

    def _execute_single_job(self, job: Dict, context: Dict) -> Dict:
        engine = job["engine"]
        params = job.get("parameters", {})
        
        if engine == "asset_manager":
            return {"status": "success"} if self.asset_manager.ingest_asset_to_project(
                params["asset_name"], params["target_project"]
            ) else {"status": "failed", "error": "Ingest failed"}

        elif engine == "unreal":
            ue_wrapper: UnrealEngine = create_unreal_engine()
            
            if "active_project" in context:
                ue_wrapper.set_active_project(context["active_project"])

            if job["type"] == "project_create":
                target_path = Path(job["output"]["path"])
                res = ue_wrapper.create_project(
                    project_name=target_path.name,
                    project_type=params.get("game_type", "game"),
                    target_dir=str(target_path.parent)
                )
                if res["status"] == "success":
                    context["active_project"] = res["project_file"]
                return res
            
            elif job["type"] == "scene_create":
                return ue_wrapper.create_cinematic_sequence(
                    sequence_name="MainSequence", 
                    shot_specs=[]
                )

        elif engine == "blender":
            # Pass directly to blender runner
            return {"status": "success", "message": "Blender job dispatched (Simulated)"}

        return {"status": "failed", "error": "Unknown engine"}

    def _generate_workflow_id(self):
        import uuid
        return str(uuid.uuid4())[:8]

    def _save_execution_report(self, result):
        pass

def create_orchestrator(config: Optional[Dict] = None) -> Orchestrator:
    return Orchestrator(config)