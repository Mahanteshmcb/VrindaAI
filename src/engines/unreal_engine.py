"""
VrindaAI - Unreal Engine Wrapper
Automates game and cinematic creation with UE5
"""

import subprocess
import json
import logging
from typing import Dict, Any, Optional, List
from pathlib import Path
import tempfile

logger = logging.getLogger(__name__)


class UnrealEngine:
    """
    Unreal Engine 5 automation engine
    Handles game development, cinematic production, and scene creation
    """
    
    def __init__(self, ue_path: Optional[str] = None):
        """
        Initialize Unreal Engine
        
        Args:
            ue_path: Path to Unreal Engine installation
        """
        self.ue_path = ue_path or self._find_unreal()
        if not self.ue_path:
            raise FileNotFoundError("Unreal Engine not found in system")
        
        self.logger = logging.getLogger(__name__)
        self.editor_exe = Path(self.ue_path) / "Engine/Binaries/Win64/UnrealEditor.exe"
    
    def _find_unreal(self) -> Optional[str]:
        """Find Unreal Engine installation"""
        common_paths = [
            "C:/Program Files/Epic Games/UE_5.3",
            "C:/Program Files/Epic Games/UE_5.2",
            "C:/Program Files/Epic Games/UE_5.1",
            "C:/Program Files/Epic Games/UE_5.0",
        ]
        
        for path in common_paths:
            if Path(path).exists():
                return path
        
        return None
    
    def create_project(
        self,
        project_name: str,
        project_type: str = "game",
        quality: str = "high"
    ) -> Dict[str, Any]:
        """
        Create new Unreal Engine project
        
        Args:
            project_name: Name of the project
            project_type: Type - "game", "cinematic", "architecture"
            quality: Quality level - "low", "medium", "high", "ultra"
            
        Returns:
            Project creation result with path and metadata
        """
        self.logger.info(f"Creating Unreal {project_type} project: {project_name}")
        
        try:
            # Generate UE project file
            project_file = self._generate_project_file(
                project_name,
                project_type,
                quality
            )
            
            # Create project directory
            project_dir = Path("output/games") / project_name
            project_dir.mkdir(parents=True, exist_ok=True)
            
            # Write project file
            uproject_path = project_dir / f"{project_name}.uproject"
            with open(uproject_path, 'w') as f:
                json.dump(project_file, f, indent=2)
            
            self.logger.info(f"Project created at: {uproject_path}")
            
            return {
                "status": "success",
                "project_path": str(project_dir),
                "project_file": str(uproject_path),
                "project_name": project_name,
                "project_type": project_type
            }
        
        except Exception as e:
            self.logger.error(f"Project creation failed: {e}")
            return {"status": "failed", "error": str(e)}
    
    def create_scene(
        self,
        project_path: str,
        scene_name: str,
        description: str,
        assets: Optional[List[str]] = None
    ) -> Dict[str, Any]:
        """
        Create new level/scene in project
        
        Args:
            project_path: Path to UE project
            scene_name: Name of the scene
            description: Scene description
            assets: List of assets to include
            
        Returns:
            Scene creation result
        """
        self.logger.info(f"Creating scene: {scene_name}")
        
        try:
            # Generate Python script for UE automation
            script = self._generate_level_script(scene_name, description, assets or [])
            
            # Execute through UE automation
            result = self._execute_automation(project_path, script)
            
            return result
        
        except Exception as e:
            self.logger.error(f"Scene creation failed: {e}")
            return {"status": "failed", "error": str(e)}
    
    def add_actor(
        self,
        project_path: str,
        actor_name: str,
        actor_class: str,
        location: Optional[List[float]] = None,
        rotation: Optional[List[float]] = None
    ) -> Dict[str, Any]:
        """Add actor to scene"""
        location = location or [0, 0, 0]
        rotation = rotation or [0, 0, 0]
        
        script = f"""
import unreal

actor_class = unreal.load_object(class_=unreal.Actor, outer=None, name="{actor_class}")
actor = unreal.EditorLevelLibrary.spawn_actor_from_class(
    actor_class,
    location=unreal.Vector({location[0]}, {location[1]}, {location[2]})
)
actor.set_actor_label("{actor_name}")
print(f"Actor created: {{actor.get_actor_label()}}")
"""
        
        return self._execute_automation(project_path, script)
    
    def add_material(
        self,
        actor_name: str,
        material_path: str,
        material_params: Dict[str, Any]
    ) -> Dict[str, Any]:
        """Apply material to actor"""
        script = f"""
import unreal

# Find actor
world = unreal.get_editor_world()
actors = unreal.GameplayStatics.get_all_actors_of_class(
    world,
    unreal.Actor
)

target_actor = None
for actor in actors:
    if actor.get_actor_label() == "{actor_name}":
        target_actor = actor
        break

if target_actor and hasattr(target_actor, 'get_owned_components'):
    components = target_actor.get_owned_components()
    for component in components:
        if isinstance(component, unreal.PrimitiveComponent):
            material = unreal.load_object(name="{material_path}")
            if material:
                component.set_material(0, material)
                print(f"Material applied to {{target_actor.get_actor_label()}}")
"""
        
        return self._execute_automation("", script)
    
    def add_animation(
        self,
        actor_name: str,
        animation_path: str,
        loop: bool = False
    ) -> Dict[str, Any]:
        """Add animation to actor"""
        script = f"""
import unreal

# Find actor with skeletal mesh
world = unreal.get_editor_world()
actors = unreal.GameplayStatics.get_all_actors_of_class(
    world,
    unreal.SkeletalMeshActor
)

for actor in actors:
    if actor.get_actor_label() == "{actor_name}":
        component = actor.skeletal_mesh_component
        animation = unreal.load_object(name="{animation_path}")
        if animation:
            component.play_animation(animation, looping={loop})
            print(f"Animation applied: {{animation.get_name()}}")
"""
        
        return self._execute_automation("", script)
    
    def setup_camera(
        self,
        project_path: str,
        camera_params: Dict[str, Any]
    ) -> Dict[str, Any]:
        """Setup camera in scene"""
        position = camera_params.get("position", [0, 0, 100])
        rotation = camera_params.get("rotation", [0, 0, 0])
        fov = camera_params.get("fov", 50)
        
        script = f"""
import unreal

# Find or create camera actor
camera_actor = unreal.EditorLevelLibrary.spawn_actor_from_class(
    unreal.CineCameraActor,
    location=unreal.Vector({position[0]}, {position[1]}, {position[2]})
)

camera_actor.set_actor_rotation(
    unreal.Rotator({rotation[0]}, {rotation[1]}, {rotation[2]}),
    use_sweep=False
)

# Set FOV
camera_comp = camera_actor.get_cine_camera_component()
camera_comp.set_current_focal_length({fov})

print("Camera setup complete")
"""
        
        return self._execute_automation(project_path, script)
    
    def setup_lighting(
        self,
        project_path: str,
        lighting_config: Dict[str, Any]
    ) -> Dict[str, Any]:
        """Setup lighting in scene"""
        script = f"""
import unreal

# Setup directional light (sun)
sun = unreal.EditorLevelLibrary.spawn_actor_from_class(
    unreal.DirectionalLight,
    location=unreal.Vector(0, 0, 500)
)
sun.set_actor_rotation(unreal.Rotator(-45, 0, 0), use_sweep=False)

# Setup sky
sky = unreal.EditorLevelLibrary.spawn_actor_from_class(
    unreal.BP_Sky_Sphere_C,
    location=unreal.Vector(0, 0, 0)
)

print("Lighting setup complete")
"""
        
        return self._execute_automation(project_path, script)
    
    def setup_postprocess(
        self,
        project_path: str,
        postprocess_config: Dict[str, Any]
    ) -> Dict[str, Any]:
        """Setup post-processing effects"""
        script = f"""
import unreal

# Create post-process volume
pp_volume = unreal.EditorLevelLibrary.spawn_actor_from_class(
    unreal.PostProcessVolume,
    location=unreal.Vector(0, 0, 0)
)

# Set to unbounded
pp_volume.set_unbound(True)

# Configure post-process effects
settings = pp_volume.settings

# Color grading
settings.color_correction_shadows_whites_clamp = 0

# Bloom
settings.bloom_intensity = {postprocess_config.get('bloom', 1.0)}

# Motion blur
settings.motion_blur_amount = {postprocess_config.get('motion_blur', 0.5)}

print("Post-processing setup complete")
"""
        
        return self._execute_automation(project_path, script)
    
    def compile_project(
        self,
        project_path: str
    ) -> Dict[str, Any]:
        """Compile project"""
        self.logger.info(f"Compiling project: {project_path}")
        
        try:
            # Run build command
            cmd = [
                str(self.editor_exe),
                str(project_path),
                "-run=BuildLight",
                "-Unattended",
                "-NullRHI"
            ]
            
            result = subprocess.run(
                cmd,
                capture_output=True,
                text=True,
                timeout=3600
            )
            
            if result.returncode == 0:
                return {"status": "success", "message": "Project compiled successfully"}
            else:
                return {"status": "failed", "error": result.stderr}
        
        except Exception as e:
            return {"status": "failed", "error": str(e)}
    
    def package_project(
        self,
        project_path: str,
        platform: str = "windows"
    ) -> Dict[str, Any]:
        """Package project for distribution"""
        self.logger.info(f"Packaging project for {platform}")
        
        try:
            output_dir = Path(project_path) / "Packaged"
            output_dir.mkdir(exist_ok=True)
            
            # Build command for packaging
            cmd = [
                str(self.editor_exe),
                str(project_path),
                "-run=BuildGame",
                f"-Platform={platform.upper()}",
                f"-Installed=True",
                "-Unattended"
            ]
            
            result = subprocess.run(
                cmd,
                capture_output=True,
                text=True,
                timeout=7200  # 2 hours
            )
            
            if result.returncode == 0:
                return {
                    "status": "success",
                    "output_dir": str(output_dir),
                    "message": f"Project packaged for {platform}"
                }
            else:
                return {"status": "failed", "error": result.stderr}
        
        except Exception as e:
            return {"status": "failed", "error": str(e)}
    
    def _generate_project_file(
        self,
        project_name: str,
        project_type: str,
        quality: str
    ) -> Dict[str, Any]:
        """Generate .uproject file"""
        return {
            "FileVersion": 3,
            "EngineAssociation": "5.3",
            "Category": "Samples",
            "Description": f"VrindaAI {project_type.capitalize()} - {project_name}",
            "Modules": [
                {
                    "Name": project_name,
                    "Type": "Runtime",
                    "LoadingPhase": "Default"
                }
            ],
            "TargetPlatforms": [
                "Windows",
                "Linux",
                "Mac"
            ],
            "Plugins": [
                {
                    "Name": "Navmesh",
                    "Enabled": True
                },
                {
                    "Name": "Sequencer",
                    "Enabled": True
                },
                {
                    "Name": "CinematicCamera",
                    "Enabled": True
                }
            ]
        }
    
    def _generate_level_script(
        self,
        scene_name: str,
        description: str,
        assets: List[str]
    ) -> str:
        """Generate Unreal automation script for level creation"""
        script = f"""
import unreal

# Create new level
editor_level_lib = unreal.EditorLevelLibrary()
level = editor_level_lib.create_new_level(unreal.LoadObject(class_=unreal.World, name="/Game/Levels/{scene_name}"))

# Add default actors
sky = editor_level_lib.spawn_actor_from_class(
    unreal.BP_Sky_Sphere_C,
    location=unreal.Vector(0, 0, 0)
)

print(f"Level created: {scene_name}")

# Asset import would go here
assets = {json.dumps(assets)}
for asset in assets:
    print(f"Would import asset: {{asset}}")
"""
        return script
    
    def _execute_automation(self, project_path: str, script: str) -> Dict[str, Any]:
        """Execute automation script"""
        with tempfile.NamedTemporaryFile(mode='w', suffix='.py', delete=False) as f:
            f.write(script)
            script_path = f.name
        
        try:
            if project_path:
                cmd = [
                    str(self.editor_exe),
                    str(project_path),
                    "-run=PythonScript",
                    f"-Script={script_path}",
                    "-Unattended"
                ]
            else:
                cmd = [
                    str(self.editor_exe),
                    "-run=PythonScript",
                    f"-Script={script_path}",
                    "-Unattended"
                ]
            
            self.logger.info("Executing UE automation script")
            
            result = subprocess.run(
                cmd,
                capture_output=True,
                text=True,
                timeout=1800
            )
            
            if result.returncode == 0:
                return {
                    "status": "success",
                    "output": result.stdout
                }
            else:
                return {
                    "status": "failed",
                    "error": result.stderr or "Automation failed"
                }
        
        finally:
            Path(script_path).unlink(missing_ok=True)


def create_unreal_engine(ue_path: Optional[str] = None) -> UnrealEngine:
    """Factory function to create Unreal Engine wrapper"""
    return UnrealEngine(ue_path)
