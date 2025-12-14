# VrindaAI - Project Cleanup and Restructure Plan

## Current State Analysis

### ✅ What's Working
- **Autonomous Workflow Engine** - C++ orchestrator for multi-agent coordination
- **Job Manifest System** - JSON-based task definition system
- **Controllers** - Blender, Unreal Engine, DaVinci Resolve integration
- **Headless Execution** - Command-line interface for automation
- **LLM Integration** - Llama-based NLP for understanding descriptions

### ❌ What's Unnecessary
1. **Duplicate Python Video Scripts** (Redundant legacy implementations):
   - `autonomous_video_creator.py` - Duplicate functionality
   - `create_video.py` - Legacy implementation
   - `fast_video_creator.py` - Legacy implementation
   - `quick_video_creator.py` - Legacy implementation
   - `generate_demo_video.py` - Test script
   - `check_video_system.py` - Diagnostic script
   - `test_video_pipeline.py` - Test script

2. **Duplicate Test/Example Scripts**:
   - `example.py` - Old example
   - `autonomous_demo.py` - Legacy demo
   - Multiple test scripts in `/scripts` directory (7+ files)

3. **Deprecated Documentation Files** (40+ markdown files):
   - All "00_*.md" files - outdated status files
   - All "AUTONOMOUS_*" duplicates - multiple versions
   - All "VIDEO_*" files - legacy video system docs
   - All status/completion report files - redundant

4. **Outdated Batch Files**:
   - `*.bat` files - All of them (obsolete for automation)
   - `run_*.bat` files - Legacy shell scripts

5. **Test/Temp Data**:
   - `temp_renders/` - temporary directory
   - `test_*.json` files - old test configurations
   - `headless_execution.log` - log file
   - `__pycache__/` - Python cache

6. **Obsolete Configuration Files**:
   - `CMakeLists.txt.user` - Qt Creator user settings
   - `VrindaAI_sa_IN.ts` - Translation file (unused)
   - Individual CI pipeline examples

## New Structure Design

```
VrindaAI/
├── 📄 README.md (Updated)
├── 📄 ARCHITECTURE.md
├── 📄 GETTING_STARTED.md
├── 📄 API_REFERENCE.md
│
├── 📁 src/
│   ├── core/
│   │   ├── input_processor.py (NEW - LLM-based prompt/JSON parser)
│   │   ├── orchestrator.py (NEW - Unified workflow orchestrator)
│   │   └── config.py (Configuration management)
│   │
│   ├── engines/
│   │   ├── blender_engine.py (Blender automation)
│   │   ├── unreal_engine.py (Unreal automation)
│   │   └── davinci_engine.py (DaVinci Resolve automation)
│   │
│   ├── templates/
│   │   ├── blender_master.py (Master Blender template)
│   │   ├── unreal_master.py (Master Unreal template)
│   │   └── davinci_master.py (Master DaVinci template)
│   │
│   ├── models/ (LLM & ML models)
│   │   ├── prompt_analyzer.py
│   │   ├── scene_generator.py
│   │   └── asset_optimizer.py
│   │
│   └── utils/
│       ├── logger.py
│       ├── validators.py
│       └── helpers.py
│
├── 📁 cpp_services/ (C++ services - Qt/CMake)
│   ├── AutonomousWorkflowEngine/
│   ├── Controllers/
│   ├── Services/
│   └── CMakeLists.txt
│
├── 📁 master_templates/
│   ├── blender/
│   │   ├── cinematic_master.blend
│   │   ├── product_showcase_master.blend
│   │   └── architecture_master.blend
│   ├── unreal/
│   │   ├── game_starter.uproject
│   │   └── cinematic_starter.uproject
│   └── davinci/
│       ├── cinematic_color_profile.drp
│       └── youtube_color_grade.drp
│
├── 📁 assets/
│   ├── materials/
│   ├── models/
│   ├── textures/
│   ├── audio/
│   └── effects/
│
├── 📁 examples/
│   ├── text_prompt_to_video.py
│   ├── json_workflow_to_game.py
│   ├── scene_description_to_cinematic.py
│   └── complete_production_pipeline.py
│
├── 📁 tests/
│   ├── test_input_processor.py
│   ├── test_orchestrator.py
│   ├── test_engines.py
│   └── conftest.py
│
├── 📁 output/
│   ├── videos/
│   ├── games/
│   ├── scenes/
│   └── logs/
│
├── 📁 config/
│   ├── settings.yaml
│   ├── engine_paths.json
│   └── llm_config.json
│
├── 📁 docs/
│   ├── QUICK_START.md
│   ├── DETAILED_API.md
│   ├── EXAMPLES.md
│   └── TROUBLESHOOTING.md
│
├── requirements.txt (Updated)
├── setup.py
└── vrindaai_cli.py (Main entry point)
```

## Implementation Steps

### Phase 1: Cleanup (Remove Unnecessary Files)
- [ ] Delete all legacy Python video scripts
- [ ] Delete all test scripts from `/scripts`
- [ ] Delete all deprecated documentation (40+ files)
- [ ] Delete all `.bat` batch files
- [ ] Clean `temp_renders/` directory
- [ ] Remove test JSON files
- [ ] Delete unused config files

### Phase 2: Core Framework (NEW)
- [ ] Create `src/core/input_processor.py` - Unified input handling
- [ ] Create `src/core/orchestrator.py` - Workflow orchestration
- [ ] Create configuration system

### Phase 3: Engine Wrappers (NEW)
- [ ] Create `src/engines/blender_engine.py`
- [ ] Create `src/engines/unreal_engine.py`
- [ ] Create `src/engines/davinci_engine.py`

### Phase 4: Master Templates (NEW)
- [ ] Create Blender master templates with LLM integration
- [ ] Create Unreal master templates
- [ ] Create DaVinci master templates

### Phase 5: LLM Integration (NEW)
- [ ] Prompt parser for natural language input
- [ ] Scene generator from descriptions
- [ ] Asset optimizer and selector

### Phase 6: CLI & Documentation
- [ ] Create main CLI entry point
- [ ] Comprehensive examples
- [ ] Complete API documentation
- [ ] Getting started guide

## Input Formats Supported

```
1. Text Prompt:
   "Create a cinematic sci-fi movie showing a spaceship entering a wormhole"
   
2. JSON Configuration:
   {
     "type": "cinematic|game|scene",
     "engine": "blender|unreal|davinci",
     "description": "...",
     "style": "sci-fi|fantasy|realistic",
     "duration": 30,
     "assets": ["asset_id_1", "asset_id_2"],
     "templates": ["cinematic_master", "color_grade_cinematic"]
   }
   
3. Scene Description:
   {
     "objects": [
       {"type": "spaceship", "material": "metallic", "animation": "enter_wormhole"}
     ],
     "environment": "space",
     "camera": {"type": "cinematic_follow"},
     "lighting": "dramatic_3point"
   }
```

## Output Formats Delivered

```
1. Professional Video (.mp4, .mov, .exr sequence)
2. Playable Game (.exe, .apk, Web build)
3. 3D Scene (Blender .blend, Unreal .uasset)
4. Assets (Materials, Models, Textures)
5. Execution Report (JSON with metrics, timings, logs)
```

## Key Features After Restructure

✅ Single unified input system (text/JSON/scene)
✅ Multi-engine automation (Blender + Unreal + DaVinci)
✅ LLM-powered intelligent templating
✅ Master templates for common use cases
✅ Complete asset library management
✅ Professional CLI interface
✅ Comprehensive logging and reporting
✅ Scalable architecture for future engines
