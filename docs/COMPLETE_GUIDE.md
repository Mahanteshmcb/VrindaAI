# VrindaAI 2.0 - Complete Documentation

## Overview

VrindaAI 2.0 is a unified autonomous AI system for creating professional 3D content across multiple engines:

- **Blender** - 3D rendering, animation, visual effects
- **Unreal Engine 5** - Game development, cinematic production  
- **DaVinci Resolve** - Video editing, color grading, post-production

Transform simple text prompts, JSON configs, or detailed scene descriptions into production-ready content.

---

## 🚀 Quick Start

### Installation

```bash
# Clone repository
git clone <repo-url>
cd VrindaAI

# Install Python dependencies
pip install -r requirements.txt

# Verify engines are installed
python vrindaai_cli.py --help
```

### 5-Minute Example

```bash
# Create a cinematic video from text
python vrindaai_cli.py --prompt "Create a sci-fi spaceship approaching a planet" --quality high

# Create a game from JSON
python vrindaai_cli.py --config game_config.json --engine unreal

# Create a 3D scene with detailed specifications
python vrindaai_cli.py --scene scene.json
```

---

## 📋 Input Formats

### 1. Text Prompt (Simplest)

```bash
python vrindaai_cli.py --prompt "Create a cinematic video of a spaceship"
```

**Features:**
- Natural language processing via LLM
- Auto-detects engine and output type
- Extracts style, duration, quality from text

**Example prompts:**
- "Make a game with action and exploration"
- "Create a 30-second sci-fi cinematic with dramatic lighting"
- "Build an architectural visualization of a modern house"

### 2. JSON Configuration

```json
{
  "type": "cinematic",
  "engine": "blender",
  "description": "Futuristic city flyover",
  "style": "cyberpunk",
  "duration": 60,
  "resolution": "1080p",
  "quality": "high",
  "assets": ["city_model", "neon_materials"],
  "templates": ["cinematic_master", "color_grade_cyberpunk"]
}
```

**Usage:**
```bash
python vrindaai_cli.py --config config.json
```

### 3. Scene Description (Most Detailed)

Complete scene specification with objects, cameras, lighting, effects:

```json
{
  "name": "Laboratory",
  "engine": "blender",
  "environment": "indoor",
  "objects": [
    {
      "name": "main_computer",
      "type": "model",
      "position": [0, 0, 0],
      "material": "metallic",
      "animation": {
        "type": "rotation",
        "speed": 1.0
      }
    }
  ],
  "camera": {
    "position": [10, 5, 8],
    "fov": 45,
    "path_animation": {
      "type": "orbit",
      "radius": 15
    }
  },
  "lighting": {
    "key_light": { ... },
    "fill_light": { ... },
    "rim_light": { ... }
  }
}
```

**Usage:**
```bash
python vrindaai_cli.py --scene scene.json
```

---

## 🎬 Output Types

| Input Type | Blender | Unreal | DaVinci |
|-----------|---------|--------|---------|
| Cinematic | Video (.mp4) | Movie Project | Edited Video |
| Game | N/A | Playable Game | N/A |
| Scene | 3D Files (.blend) | Scene Assets | N/A |
| Animation | Rendered Video | Animated Assets | Video |

---

## 🔧 System Architecture

### Core Components

```
src/
├── core/
│   ├── input_processor.py    → Unified input handling
│   ├── orchestrator.py       → Workflow coordination
│   └── config.py             → Configuration management
│
├── engines/
│   ├── blender_engine.py     → Blender automation
│   ├── unreal_engine.py      → Unreal Engine automation
│   └── davinci_engine.py     → DaVinci Resolve automation
│
├── models/
│   ├── prompt_analyzer.py    → LLM-based analysis
│   ├── scene_generator.py    → Scene from description
│   └── asset_optimizer.py    → Asset selection
│
└── utils/
    ├── logger.py
    ├── validators.py
    └── helpers.py
```

### Workflow Execution

```
1. Input Processing
   ↓
2. Task Specification Generation
   ↓
3. Environment Preparation
   ↓
4. Job Manifest Creation
   ↓
5. Sequential/Parallel Execution
   ↓
6. Post-Processing
   ↓
7. Output Generation
```

---

## 📚 Examples

### Example 1: Text to Cinematic Video

```python
from src.core.input_processor import InputProcessor
from src.core.orchestrator import Orchestrator

# Process text prompt
processor = InputProcessor()
task = processor.process_input("Create a cinematic space scene")

# Execute workflow
orchestrator = Orchestrator()
result = orchestrator.execute_workflow(task)

# Result contains video file path
print(f"Video: {result['output']['video']}")
```

### Example 2: JSON to Game

```python
import json

# Load game configuration
with open("game.json") as f:
    config = json.load(f)

# Create game
processor = InputProcessor()
task = processor.process_input(config)

orchestrator = Orchestrator()
result = orchestrator.execute_workflow(task)

print(f"Game executable: {result['output']['game']}")
```

### Example 3: Scene Description to 3D Render

```python
# Detailed scene with objects, lighting, camera
scene = {
    "name": "Lab",
    "engine": "blender",
    "objects": [...],
    "camera": {...},
    "lighting": {...}
}

processor = InputProcessor()
task = processor.process_input(scene)
result = orchestrator.execute_workflow(task)
```

Run these examples:
```bash
python examples/01_text_to_video.py
python examples/02_json_to_game.py
python examples/03_scene_description.py
```

---

## 🎨 Master Templates

Pre-built templates for common scenarios:

### Blender Templates
- `cinematic_master` - Professional cinematic setup
- `product_showcase_master` - Product visualization
- `architecture_master` - Architectural rendering

### Unreal Templates
- `game_starter` - Game project foundation
- `action_mechanics` - Action game mechanics
- `cinematic_starter` - Cinematic production

### DaVinci Templates
- `cinematic_color_profile` - Cinematic color grading
- `youtube_color_grade` - YouTube-optimized grading
- `scifi_color_grade` - Sci-fi look

**Usage in JSON:**
```json
{
  "templates": ["cinematic_master", "color_grade_scifi"]
}
```

---

## 🎛️ Command Line Options

```bash
python vrindaai_cli.py [OPTIONS]

Input (choose one):
  --prompt TEXT              Natural language prompt
  --config FILE              JSON configuration file
  --scene FILE               Scene description JSON
  --file FILE                Auto-detect input format

Options:
  --engine {blender,unreal,davinci}  Target engine
  --output DIR               Output directory
  --quality {low,medium,high,ultra}  Output quality
  --duration SECONDS         Duration (for video)
  --resolution {720p,1080p,4k}       Output resolution
  --mode {sequential,parallel,interactive}  Execution mode
  --dry-run                  Prepare without executing
  --verbose                  Verbose logging
  --show-manifest            Display job manifests
```

---

## 🔌 API Reference

### InputProcessor

```python
from src.core.input_processor import InputProcessor

processor = InputProcessor()

# Process any input type
task_spec = processor.process_input(input_data, input_type)

# Validate specification
is_valid = processor.validate_task_spec(task_spec)

# Generate job manifest
manifest = processor.generate_job_manifest(task_spec)
```

### Orchestrator

```python
from src.core.orchestrator import Orchestrator, ExecutionMode

orchestrator = Orchestrator(config)

# Execute workflow
result = orchestrator.execute_workflow(
    task_spec,
    mode=ExecutionMode.SEQUENTIAL,
    dry_run=False
)

# Register callbacks
orchestrator.register_callback("on_completion", callback_func)

# Get execution history
history = orchestrator.get_execution_history()
```

### Engine APIs

```python
# Blender
from src.engines.blender_engine import create_blender_engine
engine = create_blender_engine()
result = engine.render_from_spec(spec, output_dir)

# Unreal
from src.engines.unreal_engine import create_unreal_engine
engine = create_unreal_engine()
result = engine.create_project("MyGame", "game")

# DaVinci
from src.engines.davinci_engine import create_davinci_engine
engine = create_davinci_engine()
result = engine.create_project("MyProject")
```

---

## ⚙️ Configuration

Edit `config/settings.json`:

```json
{
  "paths": {
    "blender": "C:/Program Files/Blender Foundation/Blender 4.3/blender.exe",
    "unreal": "C:/Program Files/Epic Games/UE_5.3",
    "davinci": "C:/Program Files/Blackmagic Design/DaVinci Resolve"
  },
  "llm": {
    "host": "127.0.0.1",
    "port": 8080,
    "model": "mistral.gguf"
  },
  "rendering": {
    "gpu_enabled": true,
    "max_samples": 256
  }
}
```

---

## 🚨 Troubleshooting

### Engine not found
- Verify installation paths in `config/settings.json`
- Use `--verbose` flag for detailed error messages

### Rendering too slow
- Reduce `max_samples` in config
- Lower quality setting: `--quality low`
- Enable GPU: Ensure NVIDIA GPU drivers are installed

### GPU out of memory
- Reduce resolution: `--resolution 720p`
- Lower sample count
- Use CPU rendering as fallback

### LLM not connecting
- Ensure Llama server is running: `python -m llama_cpp_server ...`
- Check host/port in config

---

## 📊 Output Structure

```
output/
├── videos/
│   ├── workflow_abc123/
│   │   ├── frames/          (EXR sequence)
│   │   └── final.mp4        (H.264 video)
├── games/
│   ├── MyGame/
│   │   ├── MyGame.exe
│   │   └── Content/
├── scenes/
│   ├── scene_name/
│   │   └── scene_name.blend
├── logs/
│   └── workflow_*.json      (Execution reports)
└── projects/
    └── project_name/
```

---

## 🔐 Best Practices

1. **Start with dry-run** - Use `--dry-run` to verify configuration
2. **Use high quality** - Set `--quality high` for final renders
3. **Monitor execution** - Check logs for performance metrics
4. **Backup assets** - Keep master templates and asset library safe
5. **Version control** - Track JSON configs in git

---

## 🤝 Integration

### With Existing Workflows

VrindaAI is designed to integrate with:
- CI/CD pipelines
- Asset management systems
- Project management tools
- Content delivery platforms

### Custom Extensions

```python
# Create custom engine wrapper
from src.core.orchestrator import Orchestrator

class CustomEngine:
    def __init__(self):
        pass
    
    def execute(self, task_spec):
        # Your implementation
        pass

# Register in orchestrator
```

---

## 📞 Support & Resources

- **Documentation**: `docs/` folder
- **Examples**: `examples/` folder
- **Configuration**: `config/` folder
- **Templates**: `master_templates/` folder

---

## 📜 License

See LICENSE file for details.

---

**VrindaAI 2.0 - Autonomous AI Content Creation**
Transform ideas into production-ready 3D content.
