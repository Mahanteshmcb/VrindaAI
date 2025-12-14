# VrindaAI - Job Manifest & Workflow Architecture Guide

## Overview

This document explains the new **Job Manifest** system - the core of VrindaAI's "Puppet Master" architecture. Instead of generating Python code on the fly, VrindaAI now creates structured JSON job tickets that are executed by specialized Master Scripts.

---

## The Architecture

```
User Request (GUI or API)
    ↓
JobManifestManager (C++)
    ↓
Job Manifest (JSON)
    ↓
Master Script (Python)
    ↓
3D Engine (Blender/Unreal/DaVinci)
    ↓
Final Output
```

### Key Components

1. **JobManifestManager** (C++): Creates, validates, and executes job tickets
2. **Master Scripts** (Python): Specialized automation scripts for each engine
3. **Job Manifest** (JSON): Universal contract between app and engines
4. **WorkflowExecutor** (C++): Orchestrates multi-stage workflows
5. **HeadlessExecutor** (C++): Runs jobs without GUI (for external integrations)

---

## Usage Guide

### 1. Creating a Simple Blender Render Job

```cpp
// In your C++ code (e.g., Controller)
#include "Services/jobmanifestmanager.h"
#include "Controllers/blendercontroller_refactored.h"

BlenderController blender;

// Create a basic render job
QJsonObject job = blender.createRenderJob(
    "MyProject",           // project name
    "Scene1",              // scene name
    "./output/render.mp4", // output path
    1, 250,                // frame range
    1920, 1080,            // resolution
    24                     // fps
);

// Add a character
blender.addCharacter(job, "char_0", "Hero", "./assets/hero.fbx");

// Add environment
blender.addEnvironment(job, "env_0", "Forest", "./assets/forest.glb");

// Setup camera
blender.setupCamera(job, 5.0f, 10.0f, 3.0f, 0.5f, 0.0f, 0.0f, 50.0f);

// Submit the job
blender.submitJob(job, "./jobs/job_001.json");
```

### 2. Creating a Complete Workflow

```cpp
#include "Services/workflowexecutor.h"

WorkflowExecutor executor;

// Create workflow
auto config = executor.createWorkflow("SciFi_Film", "./output");

// Scene 1: Blender background
WorkflowExecutor::Scene scene1;
scene1.id = "scene_001";
scene1.name = "Space Station";
scene1.targetEngine = WorkflowExecutor::WorkflowStage::Blender;
scene1.backgroundAsset = "./assets/space_station.glb";
scene1.frameStart = 1;
scene1.frameEnd = 120;
executor.addScene(config, scene1);

// Scene 2: Unreal cinematic
WorkflowExecutor::Scene scene2;
scene2.id = "scene_002";
scene2.name = "Interior Lab";
scene2.targetEngine = WorkflowExecutor::WorkflowStage::Unreal;
scene2.characterAsset = "./assets/character.fbx";
scene2.animation = "./assets/walk.fbx";
scene2.frameStart = 1;
scene2.frameEnd = 180;
executor.addScene(config, scene2);

// Execute complete workflow
if (executor.executeWorkflow(config)) {
    qDebug() << "Workflow completed successfully!";
    qDebug() << executor.getWorkflowStatus();
}
```

### 3. Headless Execution (from command line)

Execute a single job:
```bash
VrindaAI.exe --headless --job ./jobs/render_001.json
```

Execute a workflow:
```bash
VrindaAI.exe --headless --workflow ./workflows/scifi_film.json
```

### 4. Integration with External Apps (e.g., Vryndara)

```python
import subprocess
import json

# Create a job manifest (from Python or any language)
job = {
    "job_id": "ext_001",
    "header": {
        "project_name": "External Project",
        "scene_name": "Scene1",
        "resolution": {"width": 1920, "height": 1080},
        "fps": 24
    },
    "engine": "blender",
    "assets": {...},
    "actions": [...],
    "output": {"path": "./output.mp4"}
}

# Save the job
with open("job.json", "w") as f:
    json.dump(job, f)

# Execute via VrindaAI
result = subprocess.run([
    "VrindaAI.exe",
    "--headless",
    "--job", "job.json"
])

print(f"Job completed with exit code: {result.returncode}")
```

---

## Job Manifest Structure

### Minimal Example

```json
{
  "job_id": "JOB001",
  "timestamp": "2025-12-14T10:30:00Z",
  "header": {
    "project_name": "MyProject",
    "scene_name": "Scene1",
    "resolution": {"width": 1920, "height": 1080},
    "fps": 24
  },
  "engine": "blender",
  "assets": {
    "characters": [
      {
        "id": "char_0",
        "name": "Hero",
        "path": "./assets/hero.fbx",
        "format": "fbx"
      }
    ],
    "environments": [
      {
        "id": "env_0",
        "name": "Forest",
        "path": "./assets/forest.glb",
        "format": "glb"
      }
    ],
    "props": [],
    "animations": []
  },
  "actions": [
    {
      "type": "import_asset",
      "target": "char_0",
      "parameters": {}
    },
    {
      "type": "set_camera",
      "target": "",
      "parameters": {
        "position": [5.0, 10.0, 3.0],
        "rotation": [0.5, 0.0, 0.0],
        "focal_length": 50
      }
    },
    {
      "type": "render",
      "target": "",
      "parameters": {
        "start_frame": 1,
        "end_frame": 250,
        "output_path": "./output.exr"
      }
    }
  ],
  "lighting": {
    "lights": [
      {
        "type": "sun",
        "intensity": 1.0,
        "position": [10, 10, 20]
      }
    ]
  },
  "camera": {
    "position": [5.0, 10.0, 3.0],
    "rotation": [0.5, 0.0, 0.0],
    "focal_length": 50,
    "keyframes": []
  },
  "output": {
    "path": "./output.mp4",
    "format": "mp4",
    "codec": "h264",
    "bitrate": "25M"
  }
}
```

---

## Master Scripts

### blender_master.py
- Imports assets, applies animations, sets up lighting/camera
- Renders scenes to disk
- Handles batch processing

### unreal_master.py
- Creates/loads levels, places actors
- Builds cinematic sequences
- Renders via Movie Render Queue

### davinci_master.py
- Imports clips, creates timelines
- Color grades, adds effects
- Exports final video

All scripts:
- Read job manifests and execute actions
- Support parallel execution
- Provide detailed logging
- Handle error recovery

---

## Advanced Features

### 1. Camera Keyframing

```json
"camera": {
  "keyframes": [
    {"frame": 0, "position": [0, 0, 10], "rotation": [0, 0, 0]},
    {"frame": 120, "position": [5, 5, 8], "rotation": [0.1, 0.1, 0]},
    {"frame": 240, "position": [10, 0, 5], "rotation": [0, 0.2, 0]}
  ]
}
```

### 2. Complex Lighting

```json
"lighting": {
  "ambient_strength": 0.5,
  "lights": [
    {"type": "sun", "intensity": 1.0, "position": [10, 10, 20]},
    {"type": "point", "intensity": 0.8, "position": [0, 0, 5]},
    {"type": "spot", "intensity": 2.0, "position": [-5, 5, 10]}
  ]
}
```

### 3. Parallel Job Execution

Set `"parallel_execution": true` in workflow config to run multiple jobs simultaneously.

---

## Error Handling

Each Master Script logs to:
```
./project/renders/blender_render_<timestamp>.log
./project/renders/unreal_render_<timestamp>.log
./project/renders/davinci_edit_<timestamp>.log
```

Inspect these logs for detailed error information.

---

## Future Enhancements

1. **Smart Asset Selection**: LLM chooses appropriate assets based on scene description
2. **Automatic Lighting**: AI-generated lighting configurations
3. **GPU Orchestration**: Distribute jobs across GPU cluster
4. **Streaming Renders**: Real-time preview of renders
5. **Version Control**: Git integration for project assets

---

This architecture ensures:
✅ Stability - No on-the-fly code generation
✅ Scalability - Jobs can be executed anywhere
✅ Integration - Other apps can use VrindaAI as a backend
✅ Debuggability - All operations logged and traceable
✅ Future-proof - Easy to add new engines or actions
