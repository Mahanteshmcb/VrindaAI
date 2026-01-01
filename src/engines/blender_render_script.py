import bpy
import sys
import os

# Get arguments (The path to the STL)
argv = sys.argv
argv = argv[argv.index("--") + 1:] 
stl_path = argv[0]
output_path = argv[1]

# 1. Clean Scene
bpy.ops.wm.read_factory_settings(use_empty=True)

# 2. Import STL
bpy.ops.import_mesh.stl(filepath=stl_path)
obj = bpy.context.selected_objects[0]
bpy.ops.object.shade_smooth()

# 3. Setup Camera
cam_data = bpy.data.cameras.new(name='Camera')
cam_obj = bpy.data.objects.new(name='Camera', object_data=cam_data)
bpy.context.collection.objects.link(cam_obj)
bpy.context.scene.camera = cam_obj
cam_obj.location = (100, -100, 80)
cam_obj.rotation_euler = (0.9, 0, 0.78) # Iso Angle

# 4. Setup Lights (Studio Setup)
def create_light(name, loc, energy):
    light_data = bpy.data.lights.new(name=name, type='AREA')
    light_data.energy = energy
    light_obj = bpy.data.objects.new(name=name, object_data=light_data)
    bpy.context.collection.objects.link(light_obj)
    light_obj.location = loc
    light_obj.scale = (50, 50, 50)
    # Look at center
    import mathutils
    direction = mathutils.Vector((0,0,0)) - mathutils.Vector(loc)
    rot_quat = direction.to_track_quat('-Z', 'Y')
    light_obj.rotation_euler = rot_quat.to_euler()

create_light('Key', (80, -80, 100), 2000)
create_light('Fill', (-80, -50, 50), 500)
create_light('Rim', (0, 100, 80), 1000)

# 5. Create Material (Procedural Metal)
mat = bpy.data.materials.new(name="Inconel")
mat.use_nodes = True
nodes = mat.node_tree.nodes
links = mat.node_tree.links
nodes.clear()

# Nodes
output = nodes.new(type='ShaderNodeOutputMaterial')
principled = nodes.new(type='ShaderNodeBsdfPrincipled')
principled.inputs['Metallic'].default_value = 1.0
principled.inputs['Roughness'].default_value = 0.2
principled.inputs['Base Color'].default_value = (0.8, 0.8, 0.85, 1)

links.new(principled.outputs['BSDF'], output.inputs['Surface'])
obj.data.materials.append(mat)

# 6. Render
bpy.context.scene.render.filepath = output_path
bpy.context.scene.render.resolution_x = 1080
bpy.context.scene.render.resolution_y = 1080
bpy.ops.render.render(write_still=True)