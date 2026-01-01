"""
VrindaAI - Unreal Engine AAA Director Wrapper
Automates Asset Assembly, Sequencing, and Rendering.
"""

import subprocess
import json
import logging
import os
from typing import Dict, Any, Optional, List
from pathlib import Path
from datetime import datetime
import tempfile

logger = logging.getLogger(__name__)

class UnrealEngine:
    """
    Unreal Engine 5 automation engine.
    Handles AAA Director capabilities: Asset Spawning, Sequencing, and Rendering.
    Communicates through the shared Asset Manifest (project_assets.json).
    """
    
    def __init__(self, ue_path: Optional[str] = None):
        """Initialize Unreal Engine and verify paths."""
        self.ue_path = ue_path or self._find_unreal()
        if not self.ue_path:
            raise FileNotFoundError("Unreal Engine not found in system")
        
        self.logger = logging.getLogger(__name__)
        # Use Cmd.exe for headless/automation tasks
        self.editor_exe = Path(self.ue_path) / "Engine/Binaries/Win64/UnrealEditor-Cmd.exe"
        self.active_project_path: Optional[str] = None
    
    def _find_unreal(self) -> Optional[str]:
        """Find common Unreal Engine installation paths."""
        common_paths = [
            "C:/Program Files/Epic Games/UE_5.6",
            "C:/Program Files/Epic Games/UE_5.5",
            "C:/Program Files/Epic Games/UE_5.4",
            "C:/Program Files/Epic Games/UE_5.3",
            "C:/Program Files/Epic Games/UE_5.2",
        ]
        for path in common_paths:
            if Path(path).exists():
                return path
        return None

    def set_active_project(self, project_path: str):
        """Set the active project path for subsequent commands."""
        self.active_project_path = project_path

    # ==========================================
    # PHASE 1: ASSET MANIFEST RESOLUTION
    # ==========================================

    def _get_manifest_resolver_logic(self, project_path: str) -> str:
        """Returns Python logic to resolve asset IDs from project_assets.json."""
        manifest_path = Path(project_path).as_posix() + "/project_assets.json"
        return f"""
import unreal
import json
import os

def resolve_asset_id(asset_id):
    manifest_path = "{manifest_path}"
    if not os.path.exists(manifest_path):
        unreal.log_error(f"Manifest not found: {{manifest_path}}")
        return None

    with open(manifest_path, 'r') as f:
        manifest = json.load(f)
    
    assets = manifest.get("assets", [])
    for asset in assets:
        if asset["id"].upper() == asset_id.upper():
            # Return path or internal unreal path if available
            return asset.get("internal_path") or asset.get("path")
    return None
"""

    # ==========================================
    # PHASE 2: AUTOMATED CASTING (SPAWNING)
    # ==========================================

    def spawn_asset_from_manifest(
        self, 
        asset_id: str, 
        location: List[float], 
        rotation: List[float],
        project_path: Optional[str] = None
    ) -> Dict[str, Any]:
        """Spawns an actor resolved from the manifest ID at precise coordinates[cite: 11, 16]."""
        # 1. Resolve which path to use
        p_path = project_path or self.active_project_path
        
        # 2. Safety Check: Ensure we actually have a path before proceeding
        if not p_path:
            return {"status": "failed", "error": "No project path provided or active project set."}
        
        # 3. Now p_path is guaranteed to be a str, resolving the Pylance error
        resolver = self._get_manifest_resolver_logic(p_path)
        
        script = f"""
{resolver}

asset_path = resolve_asset_id("{asset_id}")
if asset_path:
    location = unreal.Vector({location[0]}, {location[1]}, {location[2]})
    rotation = unreal.Rotator({rotation[0]}, {rotation[1]}, {rotation[2]})
    
    # Load blueprint class and spawn [cite: 28]
    actor_class = unreal.EditorAssetLibrary.load_blueprint_class(asset_path)
    if actor_class:
        actor = unreal.EditorLevelLibrary.spawn_actor_from_class(actor_class, location, rotation)
        actor.set_actor_label("{asset_id}_Spawned")
        unreal.log(f"SUCCESS: Spawned {asset_id} at {{location}}")
    else:
        unreal.log_error(f"ERROR: Failed to load class for path: {{asset_path}}")
else:
    unreal.log_error(f"ERROR: Asset ID {asset_id} could not be resolved.")
"""
        return self._execute_automation(p_path, script)

    # ==========================================
    # PHASE 3: THE "SET" DIRECTOR (SEQUENCER)
    # ==========================================

    def create_cinematic_sequence(
        self, 
        sequence_name: str, 
        start_frame: int = 0,
        end_frame: int = 240,
        project_path: Optional[str] = None
    ) -> Dict[str, Any]:
        """Automates LevelSequence creation for camera cuts and dolly moves[cite: 19, 28]."""
        script = f"""
import unreal

def main():
    content_path = "/Game/Cinematics"
    if not unreal.EditorAssetLibrary.does_directory_exist(content_path):
        unreal.EditorAssetLibrary.make_directory(content_path)

    seq_path = f"{{content_path}}/{sequence_name}"
    asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
    
    # Create LevelSequence asset [cite: 28]
    sequence = asset_tools.create_asset(
        "{sequence_name}", content_path, unreal.LevelSequence, unreal.LevelSequenceFactoryNew()
    )

    # Set playback range (e.g. 10 seconds at 24fps) [cite: 19]
    sequence.set_playback_start(0)
    sequence.set_playback_end({end_frame})

    # Spawn and bind CineCamera
    camera_actor = unreal.EditorLevelLibrary.spawn_actor_from_class(unreal.CineCameraActor, unreal.Vector(0,0,100))
    camera_binding = sequence.add_possessable(camera_actor)
    
    # Add camera cut track
    sequence.add_master_track(unreal.MovieSceneCameraCutTrack)
    
    unreal.EditorAssetLibrary.save_asset(seq_path)
    print(f"SUCCESS: Cinematic Sequence {sequence_name} ready.")

main()
"""
        return self._execute_automation(project_path or self.active_project_path, script)

    # ==========================================
    # PHASE 4: MOVIE RENDER QUEUE (MRQ)
    # ==========================================

    def render_sequence(
        self, 
        sequence_asset_path: str, 
        output_dir: str, 
        project_path: Optional[str] = None
    ) -> Dict[str, Any]:
        """Triggers MRQ to output image sequences for FFmpeg assembly[cite: 6, 22]."""
        norm_output = Path(output_dir).as_posix()
        script = f"""
import unreal

# Configure Movie Render Queue [cite: 28]
subsystem = unreal.get_editor_subsystem(unreal.MoviePipelineQueueSubsystem)
queue = subsystem.get_queue()
job = queue.allocate_new_job()
job.sequence = unreal.SoftObjectPath("{sequence_asset_path}")

# Set Output Directory [cite: 22]
setting_output = job.get_configuration().find_or_add_setting(unreal.MoviePipelineOutputSetting)
setting_output.output_directory = unreal.DirectoryPath("{norm_output}")
setting_output.file_name_format = "shot_{sequence_asset_path}_{{frame_number}}"

unreal.log(f"SUCCESS: Render Job Queued for {{sequence_asset_path}} to {norm_output}")
"""
        return self._execute_automation(project_path or self.active_project_path, script)

    # ==========================================
    # HELPER & SYSTEM METHODS
    # ==========================================

    def _execute_automation(self, project_path: Optional[str], script: str) -> Dict[str, Any]:
        """Execute Python script headless via UnrealEditor-Cmd.exe."""
        with tempfile.NamedTemporaryFile(mode='w', suffix='.py', delete=False) as f:
            f.write(script)
            script_path = f.name
        
        try:
            cmd = [str(self.editor_exe)]
            if project_path:
                cmd.append(os.path.normpath(str(project_path)))
            
            # Headless Automation Flags [cite: 28]
            cmd.extend([
                "-run=PythonScript", f"-Script={script_path}",
                "-stdout", "-FullStdOutLogOutput", "-Unattended",
                "-NoSplash", "-NullRHI" 
            ])
            
            result = subprocess.run(cmd, capture_output=True, text=True, encoding='utf-8', timeout=600)
            
            if result.returncode == 0 and "ERROR:" not in result.stdout:
                return {"status": "success", "output": result.stdout}
            else:
                return {"status": "failed", "error": result.stdout or result.stderr}
        
        except Exception as e:
            return {"status": "failed", "error": str(e)}
        finally:
            if os.path.exists(script_path):
                os.unlink(script_path)

    def create_project(self, project_name: str, target_dir: str) -> Dict[str, Any]:
        """Create new UE project with standardized structure[cite: 12]."""
        project_dir = Path(target_dir) / project_name
        project_dir.mkdir(parents=True, exist_ok=True)
        
        # Standard folder standardization [cite: 12, 28]
        for folder in ["Raw_Downloads", "Processed_FBX", "Renders", "Config"]:
            (project_dir / folder).mkdir(exist_ok=True)

        uproject = {
            "FileVersion": 3,
            "Plugins": [
                {"Name": "PythonScriptPlugin", "Enabled": True},
                {"Name": "EditorScriptingUtilities", "Enabled": True},
                {"Name": "MovieRenderPipeline", "Enabled": True}
            ]
        }
        
        uproject_path = project_dir / f"{project_name}.uproject"
        with open(uproject_path, 'w') as f:
            json.dump(uproject, f, indent=2)
            
        return {"status": "success", "project_file": str(uproject_path)}

def create_unreal_engine(ue_path: Optional[str] = None) -> UnrealEngine:
    return UnrealEngine(ue_path)