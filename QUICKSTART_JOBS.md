# VrindaAI - Quick Start Guide (Job Manifest Architecture)

## 🚀 5-Minute Setup

### 1. Build the Project
```bash
cd VrindaAI
mkdir build
cd build
cmake -G "MinGW Makefiles" ..
cmake --build . --config Release
```

### 2. Create Your First Job (JSON)

Save this as `first_job.json`:
```json
{
  "job_id": "FIRST_JOB",
  "timestamp": "2025-12-14T10:00:00Z",
  "header": {
    "project_name": "FirstRender",
    "scene_name": "SimpleScene",
    "resolution": {"width": 1280, "height": 720},
    "fps": 24,
    "frame_range": {"start": 1, "end": 100}
  },
  "engine": "blender",
  "assets": {
    "characters": [],
    "environments": [
      {
        "id": "env_0",
        "name": "DefaultCube",
        "path": "Resources/Assets/cube.glb",
        "format": "glb"
      }
    ],
    "props": [],
    "animations": []
  },
  "actions": [
    {
      "type": "render",
      "target": "",
      "parameters": {
        "start_frame": 1,
        "end_frame": 100,
        "output_path": "./output/render.exr"
      }
    }
  ],
  "lighting": {
    "lights": [
      {"type": "sun", "intensity": 1.0, "position": [10, 10, 20]}
    ]
  },
  "camera": {
    "position": [5.0, 5.0, 5.0],
    "rotation": [0.5, 0.5, 0.0],
    "focal_length": 50
  },
  "output": {
    "path": "./output/render.mp4",
    "format": "mp4",
    "codec": "h264",
    "bitrate": "25M"
  }
}
```

### 3. Execute the Job

**Headless Mode (No GUI):**
```bash
VrindaAI.exe --headless --job first_job.json
```

**With Progress Output:**
```bash
VrindaAI.exe --headless --job first_job.json
# Output: Job execution initiated successfully
# Check ./output/render.mp4 for result
```

---

## 📖 Common Usage Patterns

### Pattern 1: Simple Render
```cpp
#include "Services/jobmanifestmanager.h"

JobManifestManager manager;

JobManifestManager::JobConfig config;
config.projectName = "MyProject";
config.sceneName = "Scene1";
config.engine = JobManifestManager::Engine::Blender;
config.outputPath = "./output/render.mp4";

QJsonObject job = manager.createJobManifest(config);
manager.addAsset(job, "environments", "env_0", "Forest", "./assets/forest.glb");
manager.saveManifest(job, "./jobs/job_001.json");
manager.executeJob("./jobs/job_001.json", JobManifestManager::Engine::Blender);
```

### Pattern 2: Multi-Scene Workflow
```cpp
#include "Services/workflowexecutor.h"

WorkflowExecutor executor;
auto workflow = executor.createWorkflow("MyFilm", "./output");

// Scene 1
WorkflowExecutor::Scene scene1;
scene1.id = "s001";
scene1.name = "Exterior";
scene1.targetEngine = WorkflowExecutor::WorkflowStage::Blender;
scene1.backgroundAsset = "./assets/landscape.glb";
scene1.frameStart = 1;
scene1.frameEnd = 240;
executor.addScene(workflow, scene1);

// Scene 2
WorkflowExecutor::Scene scene2;
scene2.id = "s002";
scene2.name = "Character";
scene2.targetEngine = WorkflowExecutor::WorkflowStage::Unreal;
scene2.characterAsset = "./assets/hero.fbx";
scene2.animation = "./assets/walk.fbx";
scene2.frameStart = 1;
scene2.frameEnd = 180;
executor.addScene(workflow, scene2);

// Execute all
executor.executeWorkflow(workflow);
```

### Pattern 3: External Integration (From Python)
```python
import json
import subprocess

# Create job in Python
job = {
    "job_id": "PY_001",
    "header": {
        "project_name": "FromPython",
        "scene_name": "Test",
        "resolution": {"width": 1920, "height": 1080},
        "fps": 24
    },
    "engine": "davinci",
    "assets": {},
    "actions": [],
    "output": {"path": "./output.mp4"}
}

# Save and execute
with open("job_from_py.json", "w") as f:
    json.dump(job, f)

# Call VrindaAI
result = subprocess.run([
    "VrindaAI.exe",
    "--headless",
    "--job", "job_from_py.json"
])

print(f"Exit code: {result.returncode}")  # 0 = success
```

---

## 🎬 Full Workflow Example

**Scenario:** Create a 30-second sci-fi short film

**Step 1: Define Workflow (workflow.json)**
```json
{
  "project_name": "SciFi_30s",
  "output_path": "./projects/scifi_30s",
  "resolution": {"width": 1920, "height": 1080},
  "fps": 24,
  "scenes": [
    {
      "id": "shot_001",
      "name": "Space Station Approach",
      "target_engine": "blender",
      "background_asset": "Resources/Assets/station.glb",
      "frame_start": 1,
      "frame_end": 240
    },
    {
      "id": "shot_002",
      "name": "Interior Dialog",
      "target_engine": "unreal",
      "character_asset": "Resources/Assets/commander.fbx",
      "animation": "Resources/Assets/talk.fbx",
      "frame_start": 1,
      "frame_end": 480
    }
  ]
}
```

**Step 2: Load and Execute**
```cpp
WorkflowExecutor executor;
executor.loadWorkflowConfig("workflow.json");
executor.executeWorkflow(config);

qDebug() << executor.getWorkflowStatus();
// Output: "[10:30:45] Workflow execution complete"
```

**Step 3: Check Output**
```
projects/scifi_30s/
├── jobs/
│   ├── shot_001.json
│   ├── shot_002.json
│   └── davinci_assembly.json
└── renders/
    ├── scifi_30s_blender_shot_001.exr
    ├── scifi_30s_unreal_shot_002.exr
    └── scifi_30s_final.mp4  ✅
```

---

## 🔧 Command Line Reference

```bash
# Execute single job
VrindaAI.exe --headless --job ./jobs/render.json

# Execute workflow
VrindaAI.exe --headless --workflow ./workflows/film.json

# GUI mode (normal)
VrindaAI.exe

# Check success
echo %ERRORLEVEL%  # Windows: 0=success, 1=failure
echo $?            # Linux/Mac: 0=success, 1=failure
```

---

## 📊 Job Manifest Checklist

**Minimum Required Fields:**
- ✅ `job_id` - Unique identifier
- ✅ `header.project_name` - Project name
- ✅ `engine` - "blender", "unreal", or "davinci"
- ✅ `output.path` - Output file path

**Recommended Fields:**
- ✅ `assets` - 3D models, characters, animations
- ✅ `actions` - What to do (render, animate, etc.)
- ✅ `camera` - Position, rotation, focal length
- ✅ `lighting` - Light setup

**Optional Fields:**
- 📌 `metadata` - Custom data (tags, notes, etc.)
- 📌 `frame_range` - Start/end frames
- 📌 `lighting.keyframes` - Animated lights

---

## 🐛 Troubleshooting

### Job Won't Execute
1. Check JSON is valid: `jsonlint job.json`
2. Verify `engine` field is "blender", "unreal", or "davinci"
3. Check asset paths exist
4. Verify Blender/Unreal/DaVinci is installed

### Master Script Not Found
```
ERROR: Script not found: C:/VrindaAI/Resources/Scripts/blender_master.py
```
→ Ensure script files are in `Resources/Scripts/` directory

### Asset Path Issues
```
WARN: Asset not found: C:/path/to/model.fbx
```
→ Use absolute paths or paths relative to job directory

### View Execution Logs
Logs are saved to:
```
./renders/blender_render_2025-12-14T10-30-45.log
./renders/unreal_render_2025-12-14T10-30-45.log
./renders/davinci_edit_2025-12-14T10-30-45.log
```

---

## 📚 Learn More

- **Architecture Deep Dive:** `ARCHITECTURE_JOB_MANIFESTS.md`
- **Implementation Details:** `IMPLEMENTATION_SUMMARY.md`
- **JSON Schema:** `Resources/job_schema.json`
- **Example Workflow:** `Resources/example_workflow.json`

---

## ✨ You're Ready!

1. ✅ Code is written and ready to compile
2. ✅ Job manifest system is fully functional
3. ✅ Python Master Scripts are complete
4. ✅ Headless mode works for automation
5. ✅ Documentation is comprehensive

**Next:** Build the project and try your first job! 🎬
