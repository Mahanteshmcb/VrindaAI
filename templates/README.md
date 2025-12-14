# Master Templates

This folder should contain master scene templates and script templates used by the orchestrator to generate engine-specific scripts.

Suggested files:
- `blender_master_scene.blend` (a minimal base scene containing camera, light, HDRI)
- `blender_template.py` (python template for Blender scripts)
- `unreal_master_project/` (UE project with base assets)
- `davinci_master_timeline.xml` (resolve timeline template)

Place shared assets in `../assets/` and reference by name in generated job manifests.
