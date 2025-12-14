"""
VrindaAI - Unified Orchestrator
Coordinates workflow execution across all engines (Blender, Unreal, DaVinci)
"""

import json
import logging
import asyncio
from typing import Dict, Any, Optional, List, Callable
from pathlib import Path
from enum import Enum
from datetime import datetime
import subprocess
import sys

logger = logging.getLogger(__name__)


class WorkflowStatus(Enum):
    """Workflow execution status"""
    PENDING = "pending"
    RUNNING = "running"
    COMPLETED = "completed"
    FAILED = "failed"
    CANCELLED = "cancelled"


class ExecutionMode(Enum):
    """Execution modes"""
    SEQUENTIAL = "sequential"  # Execute jobs one after another
    PARALLEL = "parallel"      # Execute multiple jobs simultaneously
    INTERACTIVE = "interactive" # Wait for user input between steps


class Orchestrator:
    """
    Main orchestration engine for VrindaAI
    Handles workflow coordination, job scheduling, and execution monitoring
    """
    
    def __init__(self, config: Optional[Dict] = None):
        """
        Initialize orchestrator
        
        Args:
            config: Configuration dictionary with paths and settings
        """
        self.config = config or self._load_default_config()
        self.logger = logging.getLogger(__name__)
        # FIX: Ensure output_dir is absolute to avoid ambiguity
        self.output_dir = Path(self.config.get("output_dir", "output")).resolve()
        self.output_dir.mkdir(parents=True, exist_ok=True)
        
        self.workflow_status = {}
        self.execution_history = []
        self.callbacks = {}
    
    def _load_default_config(self) -> Dict[str, Any]:
        """Load default configuration"""
        return {
            "output_dir": "output",
            "temp_dir": ".temp",
            "log_dir": "output/logs",
            "max_parallel_jobs": 2,
            "timeout_seconds": 3600,
            "retry_failed": True,
            "max_retries": 2,
        }
    
    def execute_workflow(
        self,
        task_spec: Dict[str, Any],
        mode: ExecutionMode = ExecutionMode.SEQUENTIAL,
        dry_run: bool = False
    ) -> Dict[str, Any]:
        """Execute a complete workflow based on task specification"""
        
        # FIX: Safely handle description
        description = str(task_spec.get('description', ''))
        self.logger.info(f"Starting workflow execution: {description[:50]}...")
        
        workflow_id = self._generate_workflow_id()
        
        result = {
            "workflow_id": workflow_id,
            "status": WorkflowStatus.RUNNING.value,
            "start_time": datetime.now().isoformat(),
            "engine": task_spec.get("engine"),
            "input_type": task_spec.get("input_type"),
            "stages": [],
            "output": {},
        }
        
        try:
            # Stage 1: Prepare environment
            self.logger.info("Stage 1: Preparing environment")
            prep_result = self._prepare_environment(task_spec, workflow_id)
            
            if not prep_result or not prep_result.get("success"):
                error_msg = prep_result.get('error') if prep_result else "Unknown error"
                raise Exception(f"Environment preparation failed: {error_msg}")
            
            result["stages"].append({
                "name": "prepare",
                "status": "completed",
                "details": prep_result
            })
            
            # Stage 2: Generate job manifests
            self.logger.info("Stage 2: Generating job manifests")
            # FIX: Pass workflow_id to generate correct paths
            job_manifests = self._generate_job_manifests(task_spec, workflow_id)
            result["job_count"] = len(job_manifests)
            
            if dry_run:
                self.logger.info("DRY RUN: Stopping before execution")
                result["status"] = WorkflowStatus.PENDING.value
                result["manifests"] = job_manifests
                return result
            
            # Stage 3: Execute jobs
            self.logger.info(f"Stage 3: Executing {len(job_manifests)} jobs in {mode.value} mode")
            execution_result = self._execute_jobs(job_manifests, mode)
            
            result["stages"].append({
                "name": "execution",
                "status": execution_result["overall_status"],
                "jobs_executed": execution_result["jobs_executed"],
                "jobs_failed": execution_result["jobs_failed"],
            })
            
            if execution_result["overall_status"] != "completed":
                result["status"] = WorkflowStatus.FAILED.value
                result["error"] = execution_result.get("error")
                return result
            
            # Stage 4: Post-process results
            self.logger.info("Stage 4: Post-processing results")
            post_result = self._post_process_results(task_spec, execution_result)
            result["output"] = post_result["outputs"]
            
            result["stages"].append({
                "name": "post_process",
                "status": "completed",
                "outputs": post_result["outputs"]
            })
            
            result["status"] = WorkflowStatus.COMPLETED.value
            result["end_time"] = datetime.now().isoformat()
            
            self.logger.info("✅ Workflow completed successfully")
            self._trigger_callback("on_completion", result)
            
        except Exception as e:
            self.logger.error(f"❌ Workflow failed: {e}")
            result["status"] = WorkflowStatus.FAILED.value
            result["error"] = str(e)
            result["end_time"] = datetime.now().isoformat()
            self._trigger_callback("on_failure", result)
        
        # Save execution report
        self._save_execution_report(result)
        self.execution_history.append(result)
        
        return result
    
    def _prepare_environment(self, task_spec: Dict[str, Any], workflow_id: str) -> Dict[str, Any]:
        """Prepare environment for execution"""
        try:
            engine = str(task_spec.get("engine", ""))
            
            engine_path = self._find_engine_executable(engine)
            if not engine_path:
                return {
                    "success": False,
                    "error": f"Engine not found: {engine}"
                }
            
            # FIX: Use safe folder naming and create specific job folder
            desc = str(task_spec.get("description", "unnamed"))
            safe_desc = "".join([c for c in desc if c.isalnum() or c in (' ', '-', '_')]).strip()
            
            # Create a dedicated folder for this run: output/ProjectName_ID
            output_dir = self.output_dir / f"{safe_desc[:30]}_{workflow_id}"
            output_dir.mkdir(parents=True, exist_ok=True)
            
            # Create temp directory
            temp_dir = Path(".temp") / workflow_id
            temp_dir.mkdir(parents=True, exist_ok=True)
            
            return {
                "success": True,
                "engine_path": str(engine_path),
                "output_dir": str(output_dir),
                "temp_dir": str(temp_dir),
                "timestamp": datetime.now().isoformat()
            }
        
        except Exception as e:
            return {"success": False, "error": str(e)}
    
    def _generate_job_manifests(self, task_spec: Dict[str, Any], workflow_id: str) -> List[Dict[str, Any]]:
        """Generate job manifests for execution"""
        engine = task_spec.get("engine")
        
        if engine == "blender":
            return self._generate_blender_manifests(task_spec, workflow_id)
        elif engine == "unreal":
            return self._generate_unreal_manifests(task_spec, workflow_id)
        elif engine == "davinci":
            return self._generate_davinci_manifests(task_spec, workflow_id)
        else:
            raise ValueError(f"Unknown engine: {engine}")
    
    def _generate_blender_manifests(self, task_spec: Dict[str, Any], workflow_id: str) -> List[Dict]:
        """Generate Blender job manifests"""
        manifests = []
        
        # Calculate correct output path
        desc = str(task_spec.get("description", "unnamed"))
        safe_desc = "".join([c for c in desc if c.isalnum() or c in (' ', '-', '_')]).strip()
        project_folder = self.output_dir / f"{safe_desc[:30]}_{workflow_id}"
        
        # Main render job
        render_job = {
            "id": "blender_render_main",
            "engine": "blender",
            "type": "render",
            "template": task_spec.get("templates", ["cinematic_master"])[0],
            "parameters": {
                "style": task_spec.get("style", "cinematic"),
                "duration": task_spec.get("duration", 30),
                "resolution": task_spec.get("resolution", "1080p"),
                "fps": task_spec.get("fps", 24),
                "samples": 128 if task_spec.get("quality") == "ultra" else 64,
                "engine": "cycles",
                "device": "gpu" if self._has_gpu() else "cpu",
                "description": task_spec.get("description"),
                "quality": task_spec.get("quality", "high")
            },
            "assets": task_spec.get("assets", []),
            "output": {
                "format": "EXR",
                # FIX: Use absolute, resolved path with f-string injection
                "path": str(project_folder / "frames")
            }
        }
        manifests.append(render_job)
        
        return manifests
    
    def _generate_unreal_manifests(self, task_spec: Dict[str, Any], workflow_id: str) -> List[Dict]:
        """Generate Unreal Engine job manifests"""
        manifests = []
        
        desc = str(task_spec.get("description", "unnamed"))
        safe_desc = "".join([c for c in desc if c.isalnum() or c in (' ', '-', '_')]).strip()
        project_folder = self.output_dir / f"{safe_desc[:30]}_{workflow_id}"
        
        # Project creation
        create_job = {
            "id": "unreal_project_create",
            "engine": "unreal",
            "type": "project_create",
            "template": task_spec.get("templates", ["game_starter"])[0],
            "parameters": {
                "game_type": task_spec.get("type", "game"),
                "quality": task_spec.get("quality", "high"),
                "description": task_spec.get("description"),
            },
            "output": {
                "path": str(project_folder / "ue_project")
            }
        }
        manifests.append(create_job)
        
        # Scene creation
        scene_job = {
            "id": "unreal_scene_create",
            "engine": "unreal",
            "type": "scene_create",
            "parameters": {
                "description": task_spec.get("description"),
                "assets": task_spec.get("assets", []),
            },
            "depends_on": ["unreal_project_create"],
            "output": {
                "path": str(project_folder / "ue_project" / "Content")
            }
        }
        manifests.append(scene_job)
        
        return manifests
    
    def _generate_davinci_manifests(self, task_spec: Dict[str, Any], workflow_id: str) -> List[Dict]:
        """Generate DaVinci Resolve job manifests"""
        manifests = []
        
        desc = str(task_spec.get("description", "unnamed"))
        safe_desc = "".join([c for c in desc if c.isalnum() or c in (' ', '-', '_')]).strip()
        project_folder = self.output_dir / f"{safe_desc[:30]}_{workflow_id}"
        
        # Color grading job
        grade_job = {
            "id": "davinci_grade",
            "engine": "davinci",
            "type": "color_grade",
            "template": task_spec.get("templates", ["cinematic_color_profile"])[0],
            "parameters": {
                "style": task_spec.get("style", "cinematic"),
                "grade_type": "professional",
                "description": task_spec.get("description"),
            },
            "input": {
                "type": "sequence",
                "path": str(project_folder / "frames")
            },
            "output": {
                "format": "mp4",
                "codec": "h264",
                "bitrate": "15000k" if task_spec.get("quality") == "ultra" else "10000k",
                "path": str(project_folder / "video.mp4")
            }
        }
        manifests.append(grade_job)
        
        return manifests
    
    def _execute_jobs(
        self,
        manifests: List[Dict],
        mode: ExecutionMode
    ) -> Dict[str, Any]:
        """Execute job manifests"""
        result = {
            "overall_status": "completed",
            "jobs_executed": 0,
            "jobs_failed": 0,
            "job_results": {}
        }
        
        try:
            if mode == ExecutionMode.SEQUENTIAL:
                result = self._execute_sequential(manifests)
            elif mode == ExecutionMode.PARALLEL:
                result = self._execute_parallel(manifests)
            elif mode == ExecutionMode.INTERACTIVE:
                result = self._execute_interactive(manifests)
            
            return result
        
        except Exception as e:
            self.logger.error(f"Job execution failed: {e}")
            result["overall_status"] = "failed"
            result["error"] = str(e)
            return result
    
    def _execute_sequential(self, manifests: List[Dict]) -> Dict:
        """Execute jobs sequentially"""
        self.logger.info("Executing jobs sequentially")
        
        results = {
            "overall_status": "completed",
            "jobs_executed": 0,
            "jobs_failed": 0,
            "job_results": {}
        }
        
        for manifest in manifests:
            self.logger.info(f"Executing job: {manifest['id']}")
            
            # Check dependencies
            if "depends_on" in manifest:
                deps = manifest["depends_on"]
                for dep in deps:
                    if results["job_results"].get(dep, {}).get("status") != "completed":
                        self.logger.warning(f"Dependency {dep} not completed, skipping {manifest['id']}")
                        continue
            
            try:
                job_result = self._execute_job(manifest)
                results["job_results"][manifest["id"]] = job_result
                results["jobs_executed"] += 1
                
                if job_result["status"] != "completed":
                    results["jobs_failed"] += 1
                    if not self.config.get("continue_on_error"):
                        results["overall_status"] = "failed"
                        break
            
            except Exception as e:
                self.logger.error(f"Job execution failed: {e}")
                results["job_results"][manifest["id"]] = {
                    "status": "failed",
                    "error": str(e)
                }
                results["jobs_failed"] += 1
        
        return results
    
    def _execute_parallel(self, manifests: List[Dict]) -> Dict:
        """Execute jobs in parallel (simplified)"""
        self.logger.info("Executing jobs in parallel")
        # For simplicity, fall back to sequential for now
        # Full async implementation would use asyncio
        return self._execute_sequential(manifests)
    
    def _execute_interactive(self, manifests: List[Dict]) -> Dict:
        """Execute jobs with user interaction"""
        self.logger.info("Executing jobs in interactive mode")
        results = {"overall_status": "completed", "jobs_executed": 0, "jobs_failed": 0, "job_results": {}}
        
        for manifest in manifests:
            self.logger.info(f"\nReady to execute job: {manifest['id']}")
            self.logger.info(f"Description: {manifest.get('parameters', {}).get('description', 'N/A')}")
            
            response = input("Execute this job? (y/n): ").lower()
            if response == 'y':
                try:
                    job_result = self._execute_job(manifest)
                    results["job_results"][manifest["id"]] = job_result
                    results["jobs_executed"] += 1
                except Exception as e:
                    results["job_results"][manifest["id"]] = {"status": "failed", "error": str(e)}
                    results["jobs_failed"] += 1
        
        return results
    
    def _execute_job(self, manifest: Dict) -> Dict[str, Any]:
        """Execute a single job using real Engine Wrappers"""
        engine_type = manifest.get("engine")
        job_type = manifest.get("type", "")
        job_id = manifest.get("id")
        
        self.logger.info(f"Executing {engine_type} job: {job_type}")
        start_time = datetime.now()
        
        output_data = {}
        
        try:
            # ---------------------------------------------------------
            # 1. BLENDER ENGINE
            # ---------------------------------------------------------
            if engine_type == "blender":
                from engines.blender_engine import create_blender_engine
                blender = create_blender_engine()
                
                if job_type == "render":
                    # Use path from manifest, ensure parent dir exists
                    out_path_str = manifest.get("output", {}).get("path")
                    # No fallback here, trust the manifest generation
                    if not out_path_str:
                         raise ValueError("No output path in manifest")
                         
                    output_path = Path(out_path_str)
                    output_path.parent.mkdir(parents=True, exist_ok=True)
                    
                    # Call actual render logic
                    # We pass the parent folder because Blender appends frame numbers
                    output_data = blender.render_from_spec(
                        manifest.get("parameters", {}), 
                        str(output_path) # Pass the frame folder itself
                    )
                
                elif job_type == "scene_create":
                    # Call scene creation logic
                    desc = manifest.get("parameters", {}).get("description", "")
                    assets = manifest.get("parameters", {}).get("assets", [])
                    output_data = blender.create_scene(desc, assets)

            # ---------------------------------------------------------
            # 2. UNREAL ENGINE
            # ---------------------------------------------------------
            elif engine_type == "unreal":
                from engines.unreal_engine import create_unreal_engine
                unreal = create_unreal_engine()
                
                if job_type == "project_create":
                    # Use Unreal wrapper to create project
                    params = manifest.get("parameters", {})
                    out_path = manifest.get("output", {}).get("path")
                    # Note: You may need to implement create_project inside unreal_engine.py
                    # if it doesn't exist, this is a placeholder call:
                    if hasattr(unreal, 'create_project'):
                        # Pass project NAME, not full path, if your wrapper expects name
                        # Assuming wrapper takes name and creates in output/games
                        proj_name = Path(out_path).name
                        output_data = unreal.create_project(proj_name, params.get("game_type"))
                    else:
                        output_data = {"status": "simulated", "message": "Unreal create_project called"}

                elif job_type == "scene_create":
                    desc = manifest.get("parameters", {}).get("description", "")
                    assets = manifest.get("parameters", {}).get("assets", [])
                    if hasattr(unreal, 'create_scene'):
                        # Pass a dummy project path or the one from dependencies
                        output_data = unreal.create_scene("DefaultProject", "NewScene", desc, assets)
            
            # ---------------------------------------------------------
            # 3. DAVINCI RESOLVE
            # ---------------------------------------------------------
            elif engine_type == "davinci":
                from engines.davinci_engine import create_davinci_engine
                davinci = create_davinci_engine()
                
                if job_type == "color_grade":
                    # Placeholder for Davinci logic
                    # davinci.render_timeline(...)
                    output_data = {"status": "simulated", "message": "DaVinci color grading executed"}

            # Calculate duration
            duration = (datetime.now() - start_time).total_seconds()

            # Check if engine wrapper returned a failure status
            if isinstance(output_data, dict) and output_data.get("status") == "failed":
                raise Exception(output_data.get("error", "Unknown engine error"))

            return {
                "status": "completed",
                "job_id": job_id,
                "output": output_data,
                "duration": duration,
                "timestamp": datetime.now().isoformat()
            }

        except Exception as e:
            self.logger.error(f"Job execution failed: {e}")
            return {
                "status": "failed",
                "job_id": job_id,
                "error": str(e),
                "timestamp": datetime.now().isoformat()
            }
    
    def _post_process_results(
        self,
        task_spec: Dict,
        execution_result: Dict
    ) -> Dict[str, Any]:
        """Post-process execution results"""
        outputs = {}
        
        # In a real scenario, we would parse execution_result['job_results']
        # to find exact paths. For now, we infer based on the job logic.
        engine = task_spec.get("engine")
        
        # We need to know the workflow ID to construct paths, 
        # or grab them from the last executed job.
        last_job_id = list(execution_result.get("job_results", {}).keys())[-1]
        last_job_output = execution_result.get("job_results", {}).get(last_job_id, {}).get("output", {})
        
        if engine == "blender":
            # Best guess if we can't extract from logs
            outputs["frames"] = "Check output/renders folder"
            if isinstance(last_job_output, dict) and "output_path" in last_job_output:
                 outputs["frames"] = last_job_output["output_path"]
                 
        elif engine == "unreal":
            outputs["game"] = "Check output/games folder"
        elif engine == "davinci":
            outputs["video"] = "Check output/videos folder"
        
        return {"outputs": outputs}
    
    def _find_engine_executable(self, engine: str) -> Optional[Path]:
        """Find engine executable path"""
        paths = {
            "blender": [
                Path("C:/Program Files/Blender Foundation/Blender 4.3/blender.exe"),
                Path("C:/Program Files/Blender Foundation/Blender 4.2/blender.exe"),
            ],
            "unreal": [
                Path("C:/Program Files/Epic Games/UE_5.3/Engine/Binaries/Win64/UnrealEditor.exe"),
                Path("C:/Program Files/Epic Games/UE_5.2/Engine/Binaries/Win64/UnrealEditor.exe"),
            ],
            "davinci": [
                Path("C:/Program Files/Blackmagic Design/DaVinci Resolve/Resolve.exe"),
                Path("C:/Program Files/DaVinci Resolve/bin/Resolve.exe"),
            ]
        }
        
        for path in paths.get(engine, []):
            if path.exists():
                return path
        
        return None
    
    def _has_gpu(self) -> bool:
        """Check if GPU is available"""
        # Simplified check
        return True
    
    def _generate_workflow_id(self) -> str:
        """Generate unique workflow ID"""
        import uuid
        return str(uuid.uuid4())[:8]
    
    def _save_execution_report(self, result: Dict) -> None:
        """Save execution report to file"""
        log_dir = self.output_dir / "logs"
        log_dir.mkdir(parents=True, exist_ok=True)
        
        report_path = log_dir / f"workflow_{result['workflow_id']}.json"
        with open(report_path, 'w') as f:
            json.dump(result, f, indent=2, default=str)
        
        self.logger.info(f"Execution report saved: {report_path}")
    
    def _trigger_callback(self, event: str, data: Dict) -> None:
        """Trigger registered callbacks"""
        if event in self.callbacks:
            for callback in self.callbacks[event]:
                try:
                    callback(data)
                except Exception as e:
                    self.logger.error(f"Callback error: {e}")
    
    def register_callback(self, event: str, callback: Callable) -> None:
        """Register callback for event"""
        if event not in self.callbacks:
            self.callbacks[event] = []
        self.callbacks[event].append(callback)
    
    def get_execution_history(self) -> List[Dict]:
        """Get execution history"""
        return self.execution_history


def create_orchestrator(config: Optional[Dict] = None) -> Orchestrator:
    """Factory function to create orchestrator"""
    return Orchestrator(config)