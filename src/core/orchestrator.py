"""
VrindaAI - Unified Orchestrator (AAA Pipeline + Neural Link Version)
"""

import json
import logging
import time
import sys
import os
from pathlib import Path
from typing import Dict, Any, Optional, List, Callable
from enum import Enum
from datetime import datetime

# --- SYSTEM PATH FIX ---
sys.path.append(str(Path(__file__).resolve().parents[2]))

# --- NEURAL LINK IMPORTS ---
import grpc
try:
    from src.core.proto import vryndara_pb2, vryndara_pb2_grpc
except ImportError:
    vryndara_pb2 = None
    vryndara_pb2_grpc = None

# Import Engines & Managers
from src.engines.ffmpeg_engine import create_ffmpeg_engine
from src.core.asset_manager import create_asset_manager, AssetManager 
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
        
        kernel_addr = self.config.get("kernel_address", "localhost:50051")
        self.kernel = KernelClient(address=kernel_addr)
        
        self.workflow_status = {}
        self.execution_history = []
    
    def _load_default_config(self) -> Dict[str, Any]:
        return {
            "output_dir": "output",
            "paths": {"vault_cache": "C:/Vault"}, 
            "max_parallel_jobs": 1,
            "continue_on_error": False
        }

    # --- UNIVERSAL ROUTER (Entry Point) ---
    def process_request(self, request_json: str) -> str:
        try:
            data = json.loads(request_json)
            method = data.get("method")
            params = data.get("params", {})

            self.logger.info(f"Universal Link received method: {method}")

            if method == "init_project_content":
                return self.init_project_content(params)
            elif method == "create_project":
                # If path is provided, use init logic, else use legacy
                if "path" in params:
                    return self.init_project_content(params)
                return self.create_project(params.get("name"), params.get("prompt"), params.get("type", "game"))
            elif method == "status_check":
                return json.dumps({"status": "online", "engines": ["unreal", "blender", "cad"]})
            else:
                return json.dumps({"status": "error", "message": f"Unknown method: {method}"})

        except Exception as e:
            self.logger.error(f"Process Request Error: {e}")
            return json.dumps({"status": "error", "message": str(e)})

    # --- HYBRID CONTENT GENERATION (The Fix for Empty Folders) ---
    def init_project_content(self, params: Dict[str, Any]) -> str:
        """
        Populates the C++ created folder with geometry scripts and manifests.
        """
        project_name = params.get("name")
        project_type = params.get("type", "game")
        # Use absolute path from C++
        project_path = Path(params.get("path", f"output/{project_name}")) 
        prompt = params.get("prompt", "")

        self.logger.info(f"Populating project at: {project_path}")
        project_path.mkdir(parents=True, exist_ok=True)

        manifest = {
            "name": project_name,
            "type": project_type,
            "prompt": prompt,
            "created_by": "VrindaAI Hybrid",
            "assets": []
        }

        # 1. BLENDER / CAD GEOMETRY GENERATION
        if project_type in ["blender", "cad", "movie"]:
            scripts_dir = project_path / "scripts"
            scripts_dir.mkdir(parents=True, exist_ok=True)
            
            # Generate the Python script that Blender will run to create the mesh
            script_content = f"""
import bpy
# Auto-generated by VrindaAI for project: {project_name}
# Prompt: {prompt}

def create_scene():
    # Clear existing
    bpy.ops.object.select_all(action='SELECT')
    bpy.ops.object.delete()
    
    # Create Base Geometry (Cube Placeholder for now, AI logic goes here)
    bpy.ops.mesh.primitive_cube_add(size=2, location=(0, 0, 1))
    cube = bpy.context.active_object
    cube.name = "{project_name}_Base"
    
    # Save
    bpy.ops.wm.save_as_mainfile(filepath="{str(project_path).replace(os.sep, '/')}/{project_name}.blend")

if __name__ == "__main__":
    create_scene()
"""
            script_file = scripts_dir / "generate_geometry.py"
            with open(script_file, "w") as f:
                f.write(script_content)
            
            manifest["assets"].append({"type": "script", "path": str(script_file)})
            manifest["status"] = "Geometry Script Ready"

        # 2. UNREAL ENGINE PROJECT
        elif project_type == "game":
            uproject_content = {
                "FileVersion": 3,
                "EngineAssociation": "5.3",
                "Category": "",
                "Description": prompt,
                "Modules": []
            }
            with open(project_path / f"{project_name}.uproject", "w") as f:
                json.dump(uproject_content, f, indent=4)
            manifest["status"] = "Unreal Project File Created"

        # Save Manifest
        with open(project_path / "manifest.json", "w") as f:
            json.dump(manifest, f, indent=4)

        return json.dumps({
            "status": "success", 
            "path": str(project_path),
            "message": f"Project content initialized in {project_path}"
        })

    # --- LEGACY WORKFLOW EXECUTION ---
    def execute_workflow(self, task_spec: Dict[str, Any], mode: ExecutionMode = ExecutionMode.SEQUENTIAL, dry_run: bool = False) -> Dict[str, Any]:
        workflow_id = self._generate_workflow_id()
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

    def _generate_blender_manifests(self, task_spec: Dict[str, Any], workflow_id: str) -> List[Dict]:
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
        
        manifests.append({
            "id": "1_create_project",
            "engine": "unreal",
            "type": "project_create",
            "parameters": {"game_type": "game", "quality": "high"},
            "output": { "path": str(project_path) }
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
        
        if engine == "unreal":
            ue_wrapper: UnrealEngine = create_unreal_engine()
            if job["type"] == "project_create":
                target_path = Path(job["output"]["path"])
                # FIXED: Removed 'project_type' arg
                res = ue_wrapper.create_project(
                    project_name=target_path.name,
                    target_dir=str(target_path.parent)
                )
                if res["status"] == "success":
                    context["active_project"] = res["project_file"]
                return res

        elif engine == "blender":
            return {"status": "success", "message": "Blender job dispatched"}

        return {"status": "success"} # Default pass

    def _generate_workflow_id(self):
        import uuid
        return str(uuid.uuid4())[:8]

    def create_project(self, name: str, prompt: str, type: str = "game") -> str:
        task_spec = {"engine": "unreal", "description": prompt}
        result = self.execute_workflow(task_spec)
        return json.dumps(result, default=str)

def create_orchestrator(config: Optional[Dict] = None) -> Orchestrator:
    return Orchestrator(config)