"""
VrindaAI - Blender Engine Wrapper
Automates Blender 3D rendering with master templates and LLM integration
"""

import subprocess
import json
import logging
from typing import Dict, Any, Optional, List, Tuple, cast
from pathlib import Path
import tempfile

logger = logging.getLogger(__name__)


class BlenderEngine:
    """
    Blender automation engine
    Handles 3D rendering, modeling, animation, and scene creation
    """
    
    def __init__(self, blender_path: Optional[str] = None):
        """
        Initialize Blender engine
        
        Args:
            blender_path: Path to Blender executable
        """
        self.blender_path = blender_path or self._find_blender()
        if not self.blender_path:
            raise FileNotFoundError("Blender not found in system PATH")
        
        self.logger = logging.getLogger(__name__)
    
    def _find_blender(self) -> Optional[str]:
        """Find Blender executable"""
        import shutil
        blender_exe = shutil.which("blender")
        if blender_exe:
            return blender_exe
        
        # Check common Windows paths
        common_paths = [
            "C:/Program Files/Blender Foundation/Blender 4.3/blender.exe",
            "C:/Program Files/Blender Foundation/Blender 4.2/blender.exe",
            "C:/Program Files/Blender Foundation/Blender 4.1/blender.exe",
        ]
        
        for path in common_paths:
            if Path(path).exists():
                return path
        
        return None
    
    def render_from_spec(
        self,
        spec: Dict[str, Any],
        output_dir: str
    ) -> Dict[str, Any]:
        """
        Render scene based on specification
        
        Args:
            spec: Task specification with render parameters
            output_dir: Output directory for renders
            
        Returns:
            Render result with output paths and metadata
        """
        # FIX: Ensure description is a string and not None before slicing
        description = str(spec.get('description', ''))
        self.logger.info(f"Starting Blender render: {description[:50]}...")
        
        try:
            # Generate Python script for Blender
            blender_script = self._generate_render_script(spec, output_dir)
            
            # Execute Blender in headless mode
            result = self._execute_blender(blender_script, spec)
            
            return result
        
        except Exception as e:
            self.logger.error(f"Blender render failed: {e}")
            return {"status": "failed", "error": str(e)}
    
    def create_scene(
        self,
        description: str,
        assets: List[str],
        style: str = "cinematic"
    ) -> Dict[str, Any]:
        """
        Create 3D scene from description
        
        Args:
            description: Natural language scene description
            assets: List of assets to include
            style: Visual style (cinematic, realistic, stylized, etc.)
            
        Returns:
            Created scene metadata and path
        """
        self.logger.info(f"Creating Blender scene: {description[:50]}...")
        
        # Generate scene creation script
        script = self._generate_scene_creation_script(description, assets, style)
        
        # Execute
        result = self._execute_blender(script, {
            "description": description,
            "scene_creation": True
        })
        
        return result
    
    def apply_material(
        self,
        object_name: str,
        material_params: Dict[str, Any]
    ) -> Dict[str, Any]:
        """Apply material to object"""
        script = f"""
import bpy

obj = bpy.data.objects.get('{object_name}')
if obj:
    mat = bpy.data.materials.new(name="{object_name}_material")
    mat.use_nodes = True
    bsdf = mat.node_tree.nodes["Principled BSDF"]
    
    # Set material properties
    bsdf.inputs['Base Color'].default_value = {tuple(material_params.get('color', [1,1,1,1]))}
    bsdf.inputs['Metallic'].default_value = {material_params.get('metallic', 0.0)}
    bsdf.inputs['Roughness'].default_value = {material_params.get('roughness', 0.5)}
    
    obj.data.materials.append(mat)
    print(f"Material applied to {{obj.name}}")
"""
        return self._execute_script(script)
    
    def add_animation(
        self,
        object_name: str,
        animation: Dict[str, Any]
    ) -> Dict[str, Any]:
        """Add animation to object"""
        anim_type = animation.get("type")
        duration = animation.get("duration", 120)
        
        script = f"""
import bpy

obj = bpy.data.objects.get('{object_name}')
if obj:
    scene = bpy.context.scene
    
    # Clear existing keyframes
    obj.animation_data_clear()
    
    # Create animation based on type
"""
        
        if anim_type == "rotation":
            script += f"""
    obj.rotation_euler = {tuple(animation.get('start_rotation', [0,0,0]))}
    obj.keyframe_insert(data_path='rotation_euler', frame={animation.get('start_frame', 1)})
    
    obj.rotation_euler = {tuple(animation.get('end_rotation', [0,0,6.28]))}
    obj.keyframe_insert(data_path='rotation_euler', frame={animation.get('end_frame', duration)})
"""
        elif anim_type == "location":
            script += f"""
    obj.location = {tuple(animation.get('start_pos', [0,0,0]))}
    obj.keyframe_insert(data_path='location', frame={animation.get('start_frame', 1)})
    
    obj.location = {tuple(animation.get('end_pos', [0,0,10]))}
    obj.keyframe_insert(data_path='location', frame={animation.get('end_frame', duration)})
"""
        
        script += """
    print(f"Animation added to {obj.name}")
"""
        
        return self._execute_script(script)
    
    def setup_camera(
        self,
        camera_params: Dict[str, Any]
    ) -> Dict[str, Any]:
        """Setup camera for rendering"""
        position = camera_params.get("position", [0, 0, 10])
        rotation = camera_params.get("rotation", [0, 0, 0])
        fov = camera_params.get("fov", 50)
        
        script = f"""
import bpy

# Find or create camera
camera = None
for obj in bpy.data.objects:
    if obj.type == 'CAMERA':
        camera = obj
        break

if not camera:
    bpy.ops.object.camera_add()
    camera = bpy.context.active_object

camera.location = {tuple(position)}
camera.rotation_euler = {tuple(rotation)}
camera.data.lens = {fov}

bpy.context.scene.camera = camera
print(f"Camera setup: position={{camera.location}}, fov={{{fov}}}")
"""
        return self._execute_script(script)
    
    def setup_lighting(
        self,
        lighting_config: Dict[str, Any]
    ) -> Dict[str, Any]:
        """Setup lighting for scene"""
        script = f"""
import bpy

# Clear existing lights
for obj in bpy.data.objects:
    if obj.type == 'LIGHT':
        bpy.data.objects.remove(obj, do_unlink=True)

# Setup 3-point lighting
lights_config = {json.dumps(lighting_config)}

for light_name, config in lights_config.items():
    bpy.ops.object.light_add(
        type=config.get('type', 'SUN'),
        location=config.get('position', [0,0,5])
    )
    light = bpy.context.active_object
    light.data.energy = config.get('energy', 1000)
    light.data.angle = config.get('angle', 0.1)
    print(f"Light added: {{light_name}}")
"""
        return self._execute_script(script)
    
    def configure_renderer(
        self,
        render_config: Dict[str, Any]
    ) -> Dict[str, Any]:
        """Configure rendering settings"""
        script = f"""
import bpy

scene = bpy.context.scene

# Set render engine
scene.render.engine = '{render_config.get('engine', 'CYCLES')}'

# Set resolution
scene.render.resolution_x = {render_config.get('width', 1920)}
scene.render.resolution_y = {render_config.get('height', 1080)}
scene.render.resolution_percentage = {render_config.get('scale', 100)}

# Set frame range
scene.frame_start = {render_config.get('frame_start', 1)}
scene.frame_end = {render_config.get('frame_end', 240)}
scene.render.fps = {render_config.get('fps', 24)}

# Cycles settings
if scene.render.engine == 'CYCLES':
    cycles = scene.cycles
    cycles.samples = {render_config.get('samples', 128)}
    cycles.use_denoising = {render_config.get('denoise', True)}
    cycles.denoiser = 'OPTIX'
    cycles.device = '{render_config.get('device', 'GPU')}'

print("Renderer configured")
"""
        return self._execute_script(script)
    
    def render_sequence(
        self,
        output_path: str,
        render_config: Dict[str, Any]
    ) -> Dict[str, Any]:
        """Render frame sequence"""
        self.logger.info(f"Starting render sequence to {output_path}")
        
        script = f"""
import bpy
import os

output_dir = r'{output_path}'
os.makedirs(output_dir, exist_ok=True)

scene = bpy.context.scene
scene.render.filepath = os.path.join(output_dir, 'frame_####.exr')
scene.render.image_settings.file_format = 'OPEN_EXR'

# Render animation
bpy.ops.render.render(animation=True)
print(f"Render complete: {{scene.frame_end - scene.frame_start + 1}} frames")
"""
        
        result = self._execute_script(script)
        
        if result.get("status") == "success":
            # Verify output files
            output_dir = Path(output_path)
            frame_files = list(output_dir.glob("frame_*.exr"))
            result["frame_count"] = len(frame_files)
            result["output_path"] = output_path
        
        return result
    
    def _generate_render_script(self, spec: Dict, output_dir: str) -> str:
        """Generate Blender Python script for rendering"""
        template_name = spec.get("template", "cinematic_master")
        
        script = f"""
import bpy
import os

# Setup output
output_dir = r'{output_dir}'
os.makedirs(output_dir, exist_ok=True)

scene = bpy.context.scene

# Configure renderer
scene.render.engine = 'CYCLES'
scene.render.resolution_x = 1920
scene.render.resolution_y = 1080
scene.render.fps = 24

# Set frame range based on duration
duration_seconds = {spec.get('duration', 30)}
fps = 24
frame_end = duration_seconds * fps
scene.frame_end = frame_end

# Configure Cycles
scene.cycles.samples = {spec.get('samples', 64)}
scene.cycles.device = 'GPU'
scene.cycles.use_denoising = True

# Setup output format
scene.render.filepath = os.path.join(output_dir, 'frame_####.exr')
scene.render.image_settings.file_format = 'OPEN_EXR'
scene.render.image_settings.color_depth = '16'

print(f"Rendering {{frame_end}} frames...")

# Render animation
bpy.ops.render.render(animation=True)

print("Render complete!")
"""
        return script
    
    def _generate_scene_creation_script(
        self,
        description: str,
        assets: List[str],
        style: str
    ) -> str:
        """Generate Blender script for scene creation"""
        script = f"""
import bpy

# Clear default scene
bpy.ops.object.select_all(action='SELECT')
bpy.ops.object.delete(use_global=False)

# Import description and create scene
description = '''{description}'''
assets = {json.dumps(assets)}
style = '{style}'

# Add basic lighting
bpy.ops.object.light_add(type='SUN', location=(0, 0, 10))
light = bpy.context.active_object
light.data.energy = 2

# Add camera
bpy.ops.object.camera_add(location=(0, 0, 15))
scene = bpy.context.scene
scene.camera = bpy.context.active_object

print(f"Scene created with {{len(assets)}} assets")
"""
        return script
    
    def _execute_blender(self, script: str, spec: Dict) -> Dict[str, Any]:
        """Execute Blender with Python script"""
        with tempfile.NamedTemporaryFile(mode='w', suffix='.py', delete=False) as f:
            f.write(script)
            script_path = f.name
        
        try:
            cmd = [
                str(self.blender_path),
                "-b",  # Headless
                "-P", script_path
            ]
            
            self.logger.info(f"Executing: {' '.join(cmd)}")
            
            result = subprocess.run(
                cmd,
                capture_output=True,
                text=True,
                timeout=3600
            )
            
            if result.returncode == 0:
                return {
                    "status": "success",
                    "stdout": result.stdout,
                    "stderr": result.stderr
                }
            else:
                return {
                    "status": "failed",
                    "error": result.stderr or "Blender execution failed"
                }
        
        finally:
            Path(script_path).unlink(missing_ok=True)
    
    def _execute_script(self, script: str) -> Dict[str, Any]:
        """Execute Python script in Blender context"""
        blender_file = Path("temp_blender.blend")
        
        # Create temp blend file if needed
        if not blender_file.exists():
            cmd = [str(self.blender_path), "-b", "-o", str(blender_file), "-x", "1"]
            subprocess.run(cmd, capture_output=True)
        
        return self._execute_blender(script, {})


def create_blender_engine(blender_path: Optional[str] = None) -> BlenderEngine:
    """Factory function to create Blender engine"""
    return BlenderEngine(blender_path)