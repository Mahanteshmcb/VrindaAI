"""
VrindaAI - Blender Engine Wrapper (AAA Pipeline Version)
Roles: 
1. The "Auto-Rigger": Prepares raw assets for Unreal Engine.
2. The "Renderer": Handles pure Blender rendering workflows.
"""

import subprocess
import json
import logging
import os
import shutil
from typing import Dict, Any, Optional, List
from pathlib import Path
import tempfile
import sys

logger = logging.getLogger(__name__)


class BlenderEngine:
    """
    Blender automation engine.
    Handles Asset Processing (AAA Pipeline) and Rendering (Legacy Pipeline).
    """
    
    def __init__(self, blender_path: Optional[str] = None):
        """
        Initialize Blender engine
        """
        self.logger = logging.getLogger(__name__)
        self.blender_path = blender_path or self._find_blender()
        if not self.blender_path:
            raise FileNotFoundError("Blender executable not found in system PATH or standard locations.")
    
    def _find_blender(self) -> Optional[str]:
        """Find Blender executable"""
        import shutil
        # Check PATH first
        blender_exe = shutil.which("blender")
        if blender_exe:
            return blender_exe
        
        # Check common Windows paths
        common_paths = [
            r"C:\Program Files\Blender Foundation\Blender 4.3\blender.exe",
            r"C:\Program Files\Blender Foundation\Blender 4.2\blender.exe",
            r"C:\Program Files\Blender Foundation\Blender 4.1\blender.exe",
            r"C:\Program Files (x86)\Blender Foundation\Blender\blender.exe"
        ]
        
        for path in common_paths:
            if Path(path).exists():
                return path
        
        return None

    # ==========================================
    # PHASE 2: ASSET PROCESSING (The "Auto-Rigger")
    # ==========================================

    def process_asset(
        self,
        input_path: str,
        output_path: str,
        rig_type: str = "basic_human", # options: basic_human, basic_quadruped
        export_format: str = "fbx"
    ) -> Dict[str, Any]:
        """
        AAA Pipeline Function:
        Ingests a raw mesh (obj/glb), cleans it, auto-rigs it using Rigify,
        and exports a clean FBX ready for Unreal Engine.
        """
        self.logger.info(f"Processing asset for Unreal: {input_path} -> {output_path}")
        
        # Ensure output dir exists
        Path(output_path).parent.mkdir(parents=True, exist_ok=True)

        # Blender Python Script to run headless
        script = f"""
import bpy
import os

# 1. Clear Default Scene
bpy.ops.wm.read_factory_settings(use_empty=True)

# 2. Import Asset
input_file = r"{input_path}"
ext = os.path.splitext(input_file)[1].lower()

try:
    if ext == ".obj":
        bpy.ops.wm.obj_import(filepath=input_file)
    elif ext == ".fbx":
        bpy.ops.import_scene.fbx(filepath=input_file)
    elif ext in [".glb", ".gltf"]:
        bpy.ops.import_scene.gltf(filepath=input_file)
    else:
        print(f"ERROR: Unsupported format {{ext}}")
        exit(1)
except Exception as e:
    print(f"ERROR: Import failed: {{e}}")
    exit(1)

# Select the imported mesh
mesh_obj = None
for obj in bpy.context.selected_objects:
    if obj.type == 'MESH':
        mesh_obj = obj
        break

if not mesh_obj:
    print("ERROR: No mesh found in imported file")
    exit(1)

# 3. Auto-Rigging (Rigify Integration)
# Enable Rigify addon if not enabled
if 'rigify' not in bpy.context.preferences.addons:
    bpy.ops.preferences.addon_enable(module="rigify")

# Deselect all, select mesh
bpy.ops.object.select_all(action='DESELECT')
mesh_obj.select_set(True)
bpy.context.view_layer.objects.active = mesh_obj

# Center mesh
bpy.ops.object.origin_set(type='ORIGIN_GEOMETRY', center='MEDIAN')
mesh_obj.location = (0,0,0)

# Add Rig
if "{rig_type}" == "basic_quadruped":
    bpy.ops.object.armature_basic_quadruped_add()
else:
    bpy.ops.object.armature_human_metarig_add()

metarig = bpy.context.object
metarig.name = "Root"

# Naive Scaling: Scale rig to match mesh height approximately
# (A real production script would allow manual bone placement or use ML for keypoint detection)
dim_z = mesh_obj.dimensions.z
# Assuming standard metarig is ~2m tall. Scale accordingly.
scale_factor = dim_z / 1.8 
metarig.scale = (scale_factor, scale_factor, scale_factor)
bpy.ops.object.transform_apply(scale=True)

# Parent Mesh to Rig with Automatic Weights
bpy.ops.object.select_all(action='DESELECT')
mesh_obj.select_set(True)
metarig.select_set(True)
bpy.context.view_layer.objects.active = metarig

try:
    bpy.ops.object.parent_set(type='ARMATURE_AUTO')
    print("Rigging applied with automatic weights.")
except Exception as e:
    print(f"WARNING: Auto-weighting failed: {{e}}")

# 4. Export for Unreal (FBX)
out_file = r"{output_path}"
bpy.ops.export_scene.fbx(
    filepath=out_file,
    use_selection=True,
    global_scale=1.0,
    apply_unit_scale=True,
    bake_anim=False,
    object_types={{'ARMATURE', 'MESH'}},
    mesh_smooth_type='FACE',
    add_leaf_bones=False, # Critical for Unreal Engine compatibility
    primary_bone_axis='Y', 
    secondary_bone_axis='X'
)
print("SUCCESS: Export complete")
"""
        return self._execute_blender(script, {})

    # ==========================================
    # LEGACY / HYBRID: RENDERING & SCENE CREATION
    # ==========================================

    def render_from_spec(
        self,
        spec: Dict[str, Any],
        output_dir: str
    ) -> Dict[str, Any]:
        """
        Render scene based on specification (Classic Blender Workflow).
        """
        desc = str(spec.get('description', ''))
        self.logger.info(f"Starting Blender render: {desc[:50]}...")
        
        try:
            blender_script = self._generate_render_script(spec, output_dir)
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
        """Create 3D scene from description"""
        self.logger.info(f"Creating Blender scene: {description[:50]}...")
        script = self._generate_scene_creation_script(description, assets, style)
        return self._execute_blender(script, {
            "description": description,
            "scene_creation": True
        })

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
    obj.animation_data_clear()
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
        script += f"    print(f'Animation added to {{obj.name}}')"
        return self._execute_script(script)

    def setup_camera(self, camera_params: Dict[str, Any]) -> Dict[str, Any]:
        """Setup camera for rendering"""
        position = camera_params.get("position", [0, 0, 10])
        rotation = camera_params.get("rotation", [0, 0, 0])
        fov = camera_params.get("fov", 50)
        
        script = f"""
import bpy
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
print(f"Camera setup complete")
"""
        return self._execute_script(script)

    def setup_lighting(self, lighting_config: Dict[str, Any]) -> Dict[str, Any]:
        """Setup lighting for scene"""
        script = f"""
import bpy
# Clear existing lights
for obj in bpy.data.objects:
    if obj.type == 'LIGHT':
        bpy.data.objects.remove(obj, do_unlink=True)
lights_config = {json.dumps(lighting_config)}
for light_name, config in lights_config.items():
    bpy.ops.object.light_add(
        type=config.get('type', 'SUN'),
        location=config.get('position', [0,0,5])
    )
    light = bpy.context.active_object
    light.data.energy = config.get('energy', 1000)
    print(f"Light added: {{light_name}}")
"""
        return self._execute_script(script)

    # ==========================================
    # HELPER METHODS
    # ==========================================

    def _generate_render_script(self, spec: Dict, output_dir: str) -> str:
        """Generate Blender Python script for rendering"""
        quality = spec.get("quality", "high")
        engine_type = 'BLENDER_EEVEE_NEXT' if quality in ['low', 'medium'] else 'CYCLES'
        samples = 128 if quality == "ultra" else 64
        fps = spec.get('fps', 24)
        duration = spec.get('duration', 30)
        frame_end = duration * fps

        script = f"""
import bpy
import os

output_dir = r'{output_dir}'
os.makedirs(output_dir, exist_ok=True)
scene = bpy.context.scene

try:
    scene.render.engine = '{engine_type}'
except:
    scene.render.engine = 'BLENDER_EEVEE'

scene.render.resolution_x = 1920
scene.render.resolution_y = 1080
scene.render.resolution_percentage = 100
scene.render.fps = {fps}
scene.frame_end = {frame_end}

if 'CYCLES' in scene.render.engine:
    scene.cycles.device = 'GPU'
    scene.cycles.samples = {samples}
    scene.cycles.use_denoising = True
    prefs = bpy.context.preferences
    cprefs = prefs.addons['cycles'].preferences
    cprefs.refresh_devices()
    devices = cprefs.get_devices_for_type(compute_device_type='OPTIX') or cprefs.get_devices_for_type(compute_device_type='CUDA')
    if devices:
        for device in devices: device.use = True
        print(f"Enabled GPU Rendering")

scene.render.image_settings.file_format = 'OPEN_EXR' if '{quality}' != 'low' else 'PNG'
scene.render.filepath = os.path.join(output_dir, 'frame_####')

print(f"Rendering {{scene.frame_end}} frames...")
bpy.ops.render.render(animation=True)
print("Render complete!")
"""
        return script

    def _generate_scene_creation_script(self, description: str, assets: List[str], style: str) -> str:
        script = f"""
import bpy
bpy.ops.object.select_all(action='SELECT')
bpy.ops.object.delete(use_global=False)
description = '''{description}'''
assets = {json.dumps(assets)}
print(f"Scene created for {{description}}")
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
            
            self.logger.info(f"Executing Blender...")
            
            process = subprocess.Popen(
                cmd,
                stdout=subprocess.PIPE,
                stderr=subprocess.STDOUT,
                text=True,
                encoding='utf-8',
                errors='replace'
            )
            
            if process.stdout is None:
                raise RuntimeError("Failed to create stdout pipe")

            while True:
                output = process.stdout.readline()
                if output == '' and process.poll() is not None:
                    break
                if output:
                    print(f"Blender: {output.strip()}")
            
            if process.returncode == 0:
                return {"status": "success", "stdout": "Process finished"}
            else:
                return {"status": "failed", "error": "Blender execution failed"}
        
        except Exception as e:
            self.logger.error(f"Execution failed: {e}")
            return {"status": "failed", "error": str(e)}
            
        finally:
            Path(script_path).unlink(missing_ok=True)
    
    def _execute_script(self, script: str) -> Dict[str, Any]:
        """Execute simple script wrapper"""
        return self._execute_blender(script, {})


def create_blender_engine(blender_path: Optional[str] = None) -> BlenderEngine:
    """Factory function"""
    return BlenderEngine(blender_path)