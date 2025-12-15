"""
VrindaAI - Unified Orchestrator (AAA Pipeline Version)
Coordinates: Blender (Auto-Rig) -> AssetManager (Ingest) -> Unreal (Director) -> FFmpeg (Editor)
"""

import json
import logging
from typing import Dict, Any, Optional, List, Callable
from pathlib import Path
from enum import Enum
from datetime import datetime
import subprocess
import sys

# Import Engines & Managers
# Lazy imports are used inside methods to prevent circular dependency issues
from engines.ffmpeg_engine import create_ffmpeg_engine
from core.asset_manager import create_asset_manager

logger = logging.getLogger(__name__)

class WorkflowStatus(Enum):
    PENDING = "pending"
    RUNNING = "running"
    COMPLETED = "completed"
    FAILED = "failed"
    CANCELLED = "cancelled"

class ExecutionMode(Enum):
    SEQUENTIAL = "sequential"
    PARALLEL = "parallel"
    INTERACTIVE = "interactive"

class Orchestrator:
    """
    Main orchestration engine for VrindaAI
    Handles workflow coordination, job scheduling, and execution monitoring
    """
    
    def __init__(self, config: Optional[Dict] = None):
        """
        Initialize orchestrator
        """
        self.config = config or self._load_default_config()
        self.logger = logging.getLogger(__name__)
        # Ensure output_dir is absolute
        self.output_dir = Path(self.config.get("output_dir", "output")).resolve()
        self.output_dir.mkdir(parents=True, exist_ok=True)
        
        # Load Unreal Projects Root from config or default to output/games
        self.unreal_root = self.config.get("paths", {}).get("unreal_projects_root")
        if self.unreal_root:
            self.unreal_root = Path(self.unreal_root).resolve()
        else:
            self.unreal_root = self.output_dir / "games"
        
        self.unreal_root.mkdir(parents=True, exist_ok=True)

        # Initialize Core Services
        self.asset_manager = create_asset_manager(self.config)
        
        self.workflow_status = {}
        self.execution_history = []
        self.callbacks = {}
    
    def _load_default_config(self) -> Dict[str, Any]:
        """Load default configuration"""
        return {
            "output_dir": "output",
            "paths": {},
            "max_parallel_jobs": 1,
            "continue_on_error": False
        }
    
    def execute_workflow(
        self, 
        task_spec: Dict[str, Any], 
        mode: ExecutionMode = ExecutionMode.SEQUENTIAL, 
        dry_run: bool = False
    ) -> Dict[str, Any]:
        """Execute a complete workflow based on task specification"""
        
        description = str(task_spec.get('description', ''))
        self.logger.info(f"Starting workflow: {description[:50]}...")
        workflow_id = self._generate_workflow_id()
        
        result = {
            "workflow_id": workflow_id,
            "status": WorkflowStatus.RUNNING.value,
            "start_time": datetime.now().isoformat(),
            "engine": task_spec.get("engine"),
            "stages": [],
            "output": {}
        }
        
        try:
            # 1. Prepare Environment
            prep_res = self._prepare_environment(task_spec, workflow_id)
            if not prep_res.get("success"): 
                raise Exception(f"Environment preparation failed: {prep_res.get('error')}")
            
            # 2. Generate Job Manifests
            manifests = self._generate_job_manifests(task_spec, workflow_id)
            result["job_count"] = len(manifests)
            
            if dry_run:
                self.logger.info("DRY RUN: Stopping before execution")
                result["status"] = WorkflowStatus.PENDING.value
                result["manifests"] = manifests
                return result

            # 3. Execute Jobs
            exec_res = self._execute_jobs(manifests, mode)
            result["stages"] = list(exec_res.get("job_results", {}).values())
            
            if exec_res["overall_status"] != "completed":
                raise Exception(exec_res.get("error"))

            # 4. Post-Process Results
            post_result = self._post_process_results(task_spec, exec_res)
            result["output"] = post_result["outputs"]

            result["status"] = WorkflowStatus.COMPLETED.value
            result["end_time"] = datetime.now().isoformat()
            self.logger.info("✅ Workflow completed successfully")

        except Exception as e:
            self.logger.error(f"❌ Workflow failed: {e}")
            result["status"] = WorkflowStatus.FAILED.value
            result["error"] = str(e)
            result["end_time"] = datetime.now().isoformat()
        
        self._save_execution_report(result)
        self.execution_history.append(result)
        return result

    def _prepare_environment(self, task_spec: Dict[str, Any], workflow_id: str) -> Dict[str, Any]:
        """Prepare environment for execution"""
        desc = str(task_spec.get("description", "job"))
        safe_desc = "".join([c for c in desc if c.isalnum() or c in (' ', '-', '_')]).strip()
        run_dir = self.output_dir / f"{safe_desc[:30]}_{workflow_id}"
        run_dir.mkdir(parents=True, exist_ok=True)
        return {"success": True, "run_dir": str(run_dir)}

    def _generate_job_manifests(self, task_spec: Dict[str, Any], workflow_id: str) -> List[Dict]:
        """Generate job manifests based on engine type"""
        engine = task_spec.get("engine")
        
        if engine == "unreal":
            return self._generate_unreal_manifests(task_spec, workflow_id)
        elif engine == "blender":
            return self._generate_blender_manifests(task_spec, workflow_id)
        elif engine == "ffmpeg":
            return [] 
        else:
            self.logger.warning(f"Unknown engine: {engine}")
            return []

    def _generate_unreal_manifests(self, task_spec: Dict[str, Any], workflow_id: str) -> List[Dict]:
        """Generate Unreal Engine job manifests (AAA Pipeline)"""
        manifests = []
        desc = str(task_spec.get("description", "unnamed"))
        safe_desc = "".join([c for c in desc if c.isalnum() or c in (' ', '-', '_')]).strip()
        
        # Paths
        run_folder = self.output_dir / f"{safe_desc[:30]}_{workflow_id}" # For logs/renders
        project_name = f"VrindaProj_{workflow_id}"
        project_path = self.unreal_root / project_name # In Documents/Unreal Projects
        render_dir = run_folder / "renders"
        
        # 1. Create Project
        manifests.append({
            "id": "1_create_project",
            "engine": "unreal",
            "type": "project_create",
            "parameters": {
                "game_type": task_spec.get("type", "game"),
                "quality": "high"
            },
            "output": { "path": str(project_path) }
        })

        # 2. Ingest Assets (NEW STEP)
        assets = task_spec.get("assets", [])
        for asset in assets:
            manifests.append({
                "id": f"2_ingest_{asset}",
                "engine": "asset_manager", # Routed to AssetManager
                "type": "ingest_from_vault",
                "parameters": {
                    "asset_name": asset,
                    "target_project": str(project_path / "Content")
                }
            })

        # 3. Create Scene & Sequence (Director Mode)
        manifests.append({
            "id": "3_setup_scene",
            "engine": "unreal",
            "type": "scene_create",
            "parameters": {
                "description": desc,
                "assets": assets
            },
            "output": { "path": str(render_dir) }
        })

        # 4. Render Sequence (NEW STEP - renders to image sequence)
        manifests.append({
            "id": "4_render_sequence",
            "engine": "unreal",
            "type": "render_sequence",
            "parameters": {
                "sequence_path": "/Game/Cinematics/MainSequence",
                "resolution": task_spec.get("resolution", [1920, 1080]),
                "framerate": task_spec.get("framerate", 24)
            },
            "output": { "path": str(render_dir / "frames") }
        })

        # 5. Stitch to Video (NEW STEP - converts frames to MP4)
        manifests.append({
            "id": "5_stitch_video",
            "engine": "ffmpeg",
            "type": "stitch_sequence",
            "parameters": {
                "framerate": task_spec.get("framerate", 24),
                "quality": task_spec.get("quality", "high")
            },
            "input": { "pattern": str(render_dir / "frames" / "MainSequence_*.jpg") },
            "output": { "path": str(run_folder / f"{safe_desc}.mp4") }
        })

        return manifests

    def _generate_blender_manifests(self, task_spec: Dict[str, Any], workflow_id: str) -> List[Dict]:
        """Generate Blender job manifests"""
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

    def _execute_jobs(self, manifests: List[Dict], mode: ExecutionMode) -> Dict:
        """Execute job manifests"""
        results = {"overall_status": "completed", "job_results": {}}
        context = {} 

        for job in manifests:
            job_id = job["id"]
            self.logger.info(f"Executing: {job_id}")
            
            try:
                res = self._execute_single_job(job, context)
                results["job_results"][job_id] = res
                if res["status"] == "failed":
                    results["overall_status"] = "failed"
                    results["error"] = res.get("error")
                    if not self.config.get("continue_on_error"): break
            except Exception as e:
                self.logger.error(f"Execution exception: {e}")
                results["overall_status"] = "failed"
                results["error"] = str(e)
                break
                
        return results

    def _execute_single_job(self, job: Dict, context: Dict) -> Dict:
        """Execute a single job using the appropriate engine/manager"""
        engine = job["engine"]
        job_type = job["type"]
        params = job.get("parameters", {})
        
        # --- ASSET MANAGER ---
        if engine == "asset_manager":
            if job_type == "ingest_from_vault":
                success = self.asset_manager.ingest_asset_to_project(
                    params["asset_name"],
                    params["target_project"]
                )
                if not success: return {"status": "failed", "error": f"Failed to ingest {params['asset_name']}"}
                return {"status": "success", "message": "Asset Ingested"}

        # --- UNREAL ENGINE ---
        elif engine == "unreal":
            from engines.unreal_engine import create_unreal_engine
            unreal = create_unreal_engine()
            
            # Pass active project context if available
            if "active_project" in context:
                unreal.set_active_project(context["active_project"])

            if job_type == "project_create":
                # Extract target path
                target_path = job["output"]["path"]
                proj_name = Path(target_path).name
                
                # We pass the full parent directory to create_project
                parent_dir = str(Path(target_path).parent)
                
                res = unreal.create_project(proj_name, params.get("game_type"), target_dir=parent_dir)
                if res["status"] == "success":
                    context["active_project"] = res["project_file"] # Save .uproject path
                return res
                
            elif job_type == "scene_create":
                # This uses the AAA 'create_cinematic_sequence' logic inside Unreal
                return unreal.create_cinematic_sequence("MainSequence", [])
            
            elif job_type == "render_sequence":
                # Trigger MRQ
                return unreal.render_sequence(
                    params.get("sequence_path"),
                    job["output"]["path"],
                    tuple(params.get("resolution", [1920, 1080])),
                    context.get("active_project")
                )

        # --- FFMPEG ENGINE ---
        elif engine == "ffmpeg":
            from engines.ffmpeg_engine import create_ffmpeg_engine
            ffmpeg = create_ffmpeg_engine()
            
            if job_type == "stitch_sequence":
                input_pattern = job.get("input", {}).get("pattern", "")
                output_file = job["output"]["path"]
                
                if not input_pattern:
                    return {"status": "failed", "error": "No input pattern specified for FFmpeg stitch"}
                
                return ffmpeg.create_video_from_sequence(
                    input_pattern,
                    output_file,
                    framerate=params.get("framerate", 24),
                    quality=params.get("quality", "high")
                )
            
            elif job_type == "concat_clips":
                clip_paths = job.get("input", {}).get("clips", [])
                output_file = job["output"]["path"]
                
                if not clip_paths:
                    return {"status": "failed", "error": "No clips specified for concatenation"}
                
                return ffmpeg.concat_clips(clip_paths, output_file)

        # --- BLENDER ENGINE ---
        elif engine == "blender":
            from engines.blender_engine import create_blender_engine
            blender = create_blender_engine()
            
            if job_type == "render":
                # Pass output path explicitly
                out_path = Path(job["output"]["path"])
                # Blender engine expects the parent folder for frame sequences
                out_path.parent.mkdir(parents=True, exist_ok=True)
                return blender.render_from_spec(params, str(out_path))

        # --- FFMPEG ENGINE ---
        elif engine == "ffmpeg":
            from engines.ffmpeg_engine import create_ffmpeg_engine
            ffmpeg = create_ffmpeg_engine()
            
            if job_type == "stitch_sequence":
                return ffmpeg.create_video_from_sequence(
                    job["input"]["pattern"],
                    job["output"]["path"],
                    params.get("framerate", 24)
                )

        return {"status": "failed", "error": f"Unknown engine {engine}"}

    def _post_process_results(self, task_spec: Dict, execution_result: Dict) -> Dict[str, Any]:
        """Post-process execution results"""
        outputs = {}
        engine = task_spec.get("engine")
        
        if engine == "unreal":
            outputs["project"] = f"Created in {self.unreal_root}"
        elif engine == "blender":
            outputs["frames"] = "Check output/frames for rendered sequence"
            
        return {"outputs": outputs}

    def _generate_workflow_id(self):
        import uuid
        return str(uuid.uuid4())[:8]

    def _save_execution_report(self, result):
        import json
        log_dir = self.output_dir / "logs"
        log_dir.mkdir(parents=True, exist_ok=True)
        report_path = log_dir / f"workflow_{result['workflow_id']}.json"
        with open(report_path, "w") as f:
            json.dump(result, f, indent=2, default=str)

    def _find_engine_executable(self, engine: str) -> Optional[Path]:
        """Find engine executable path (Helper used in prep)"""
        # This is a fallback if needed by _prepare_environment, though config should have it
        return None 

    def _trigger_callback(self, event: str, data: Dict) -> None:
        if event in self.callbacks:
            for callback in self.callbacks[event]:
                try: callback(data)
                except Exception as e: self.logger.error(f"Callback error: {e}")

    def register_callback(self, event: str, callback: Callable) -> None:
        if event not in self.callbacks: self.callbacks[event] = []
        self.callbacks[event].append(callback)

    def get_execution_history(self) -> List[Dict]:
        return self.execution_history

def create_orchestrator(config: Optional[Dict] = None) -> Orchestrator:
    return Orchestrator(config)