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
        self.output_dir = Path(self.config.get("output_dir", "output"))
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
        """
        Execute a complete workflow based on task specification
        
        Args:
            task_spec: Normalized task specification from InputProcessor
            mode: Execution mode (sequential/parallel/interactive)
            dry_run: If True, only prepare but don't execute
            
        Returns:
            Execution result with status and output paths
        """
        self.logger.info(f"Starting workflow execution: {task_spec.get('description')[:50]}...")
        
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
            prep_result = self._prepare_environment(task_spec)
            if not prep_result["success"]:
                raise Exception(f"Environment preparation failed: {prep_result['error']}")
            
            result["stages"].append({
                "name": "prepare",
                "status": "completed",
                "details": prep_result
            })
            
            # Stage 2: Generate job manifests
            self.logger.info("Stage 2: Generating job manifests")
            job_manifests = self._generate_job_manifests(task_spec)
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
    
    def _prepare_environment(self, task_spec: Dict[str, Any]) -> Dict[str, Any]:
        """Prepare environment for execution"""
        try:
            engine = task_spec.get("engine")
            
            # Check if engine is available
            engine_path = self._find_engine_executable(engine)
            if not engine_path:
                return {
                    "success": False,
                    "error": f"Engine not found: {engine}"
                }
            
            # Create output directories
            output_dir = self.output_dir / task_spec.get("description", "unnamed")[:30]
            output_dir.mkdir(parents=True, exist_ok=True)
            
            # Create temp directory
            temp_dir = Path(".temp") / str(self._generate_workflow_id())
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
    
    def _generate_job_manifests(self, task_spec: Dict[str, Any]) -> List[Dict[str, Any]]:
        """Generate job manifests for execution"""
        engine = task_spec.get("engine")
        
        if engine == "blender":
            return self._generate_blender_manifests(task_spec)
        elif engine == "unreal":
            return self._generate_unreal_manifests(task_spec)
        elif engine == "davinci":
            return self._generate_davinci_manifests(task_spec)
        else:
            raise ValueError(f"Unknown engine: {engine}")
    
    def _generate_blender_manifests(self, task_spec: Dict[str, Any]) -> List[Dict]:
        """Generate Blender job manifests"""
        manifests = []
        
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
            },
            "assets": task_spec.get("assets", []),
            "output": {
                "format": "EXR",
                "path": "output/renders/{workflow_id}/frames"
            }
        }
        manifests.append(render_job)
        
        return manifests
    
    def _generate_unreal_manifests(self, task_spec: Dict[str, Any]) -> List[Dict]:
        """Generate Unreal Engine job manifests"""
        manifests = []
        
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
                "path": "output/games/{workflow_id}"
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
                "path": "output/games/{workflow_id}/Content"
            }
        }
        manifests.append(scene_job)
        
        return manifests
    
    def _generate_davinci_manifests(self, task_spec: Dict[str, Any]) -> List[Dict]:
        """Generate DaVinci Resolve job manifests"""
        manifests = []
        
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
                "path": "output/renders/{workflow_id}/frames"
            },
            "output": {
                "format": "mp4",
                "codec": "h264",
                "bitrate": "15000k" if task_spec.get("quality") == "ultra" else "10000k",
                "path": "output/videos/{workflow_id}.mp4"
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
        """Execute a single job"""
        engine = manifest.get("engine")
        job_type = manifest.get("type", "")
        
        self.logger.info(f"Executing {engine} job: {job_type}")
        
        # This would be implemented to call appropriate engine controllers
        # For now, return a mock success
        return {
            "status": "completed",
            "job_id": manifest.get("id"),
            "output": f"output/{engine}/{manifest.get('id')}",
            "duration": 0,
            "timestamp": datetime.now().isoformat()
        }
    
    def _post_process_results(
        self,
        task_spec: Dict,
        execution_result: Dict
    ) -> Dict[str, Any]:
        """Post-process execution results"""
        outputs = {}
        
        engine = task_spec.get("engine")
        
        if engine == "blender":
            outputs["video"] = "output/videos/final.mp4"
            outputs["frames"] = "output/renders/frames"
        elif engine == "unreal":
            outputs["game"] = "output/games/final.exe"
            outputs["assets"] = "output/games/assets"
        elif engine == "davinci":
            outputs["video"] = "output/videos/final.mp4"
            outputs["project"] = "output/davinci_projects"
        
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
