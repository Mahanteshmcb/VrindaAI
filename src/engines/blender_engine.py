"""
VrindaAI - Blender Engine Wrapper
Automates Blender 3D rendering with master templates and LLM integration
"""

import subprocess
import json
import logging
from typing import Dict, Any, Optional, List
from pathlib import Path
import tempfile
import sys

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
        desc = str(spec.get('description', ''))
        self.logger.info(f"Starting Blender render: {desc[:50]}...")
        
        try:
            # Generate Python script for Blender
            blender_script = self._generate_render_script(spec, output_dir)
            
            # Execute Blender
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
        """
        self.logger.info(f"Creating Blender scene: {description[:50]}...")
        
        script = self._generate_scene_creation_script(description, assets, style)
        
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
    
    def _generate_render_script(self, spec: Dict, output_dir: str) -> str:
        """Generate Blender Python script for rendering with Optimization logic"""
        
        # 1. Determine Quality Settings
        quality = spec.get("quality", "high")
        
        # Use EEVEE for speed if quality is low/medium (Real-time engine)
        # Use CYCLES for quality if high/ultra (Ray-tracing engine)
        if quality in ['low', 'medium']:
            engine_type = 'BLENDER_EEVEE_NEXT' # For Blender 4.2+
        else:
            engine_type = 'CYCLES'
            
        # Samples
        samples = 64
        if quality == "ultra": samples = 128
        elif quality == "low": samples = 16
        
        # Frame Range
        fps = spec.get('fps', 24)
        duration = spec.get('duration', 30)
        frame_end = duration * fps

        script = f"""
import bpy
import os

# Setup output
output_dir = r'{output_dir}'
os.makedirs(output_dir, exist_ok=True)

scene = bpy.context.scene

# --- 1. Engine Setup ---
try:
    scene.render.engine = '{engine_type}'
except:
    # Fallback if specific Eevee name fails
    scene.render.engine = 'BLENDER_EEVEE'

# Resolution
scene.render.resolution_x = 1920
scene.render.resolution_y = 1080
scene.render.resolution_percentage = 100

# Frame Range
scene.render.fps = {fps}
scene.frame_end = {frame_end}

# --- 2. GPU Optimization (Critical for Speed) ---
if 'CYCLES' in scene.render.engine:
    scene.cycles.device = 'GPU'
    scene.cycles.samples = {samples}
    scene.cycles.use_denoising = True
    
    # Enable GPU in Preferences
    prefs = bpy.context.preferences
    cprefs = prefs.addons['cycles'].preferences
    cprefs.refresh_devices()
    
    # Try OPTIX first (Nvidia RTX), then CUDA, then HIP (AMD)
    devices = cprefs.get_devices_for_type(compute_device_type='OPTIX')
    if not devices:
        devices = cprefs.get_devices_for_type(compute_device_type='CUDA')
    
    if devices:
        cprefs.compute_device_type = 'OPTIX' if 'OPTIX' in [d.type for d in devices] else 'CUDA'
        for device in devices:
            device.use = True
        print(f"Enabled GPU Rendering on: {{[d.name for d in devices]}}")
    else:
        print("WARNING: No GPU found! Rendering will be slow.")

# --- 3. Output Format ---
# Use PNG for low quality speed, EXR for professional pipeline
file_format = 'PNG' if '{quality}' == 'low' else 'OPEN_EXR'
scene.render.image_settings.file_format = file_format
scene.render.filepath = os.path.join(output_dir, 'frame_####')

print(f"Rendering {{scene.frame_end}} frames using {{scene.render.engine}}...")

# --- 4. Render ---
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
            
            # Use Popen to stream output line-by-line
            process = subprocess.Popen(
                cmd,
                stdout=subprocess.PIPE,
                stderr=subprocess.STDOUT,
                text=True,
                encoding='utf-8', # Force UTF-8 to handle progress bars
                errors='replace'
            )
            
            # FIX: Explicit check for None to satisfy Pylance
            if process.stdout is None:
                raise RuntimeError("Failed to create stdout pipe for Blender process")

            # Read stdout line by line
            while True:
                output = process.stdout.readline()
                if output == '' and process.poll() is not None:
                    break
                if output:
                    # Print to console immediately so user sees progress
                    print(f"Blender: {output.strip()}")
            
            if process.returncode == 0:
                return {
                    "status": "success",
                    "stdout": "Process finished (logs streamed)",
                    "stderr": None
                }
            else:
                return {
                    "status": "failed",
                    "error": "Blender execution failed (see console logs)"
                }
        
        except Exception as e:
            self.logger.error(f"Execution failed: {e}")
            return {"status": "failed", "error": str(e)}
            
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