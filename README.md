# VrindaAI 2.0 - Autonomous AI Content Creator

**Transform text prompts, JSON configs, or scene descriptions into professional 3D content for Blender, Unreal Engine, and DaVinci Resolve.**

## 🚀 Quick Start

### Prerequisites
- **Windows 10/11** with NVIDIA GPU (or CPU fallback)
- **Qt 6.5+** 
- **Python 3.11+**
- **CMake 3.19+**

### Installation & Running

1. **Clone the repository:**
   ```bash
   git clone <repository-url>
   cd VrindaAI
   ```

2. **Set up Python environment:**
   ```bash
   conda create -n vrindaai python=3.11
   conda activate vrindaai
   pip install -r requirements.txt
   ```

3. **Start the Llama Server** (Terminal 1):
   ```bash
   cd llama.cpp/build/bin/Release
   .\llama-server.exe --model mistral.gguf -ngl 99 --parallel 4 --threads 8 --host 127.0.0.1 --port 8080 --ctx-size 4096
   ```
   
   Verify server health:
   ```bash
   curl http://127.0.0.1:8080/health
   ```

4. **Start the Memory Service** (Terminal 2):
   ```bash
   conda activate vrindaai
   python memory_service.py
   ```

5. **Launch the Application** (Terminal 3):
   ```bash
   .\VrindaAI.exe
   ```
   
   Or if running from source:
   ```bash
   cd build/Desktop_Qt_6_9_1_MinGW_64_bit-Debug
   .\VrindaAI.exe
   ```

---

## 🧠 Core Agents Overview

| Agent | Role | Responsibilities |
|-------|------|------------------|
| **Assistant (Vrinda)** | Central Coordinator | Receives your goals, breaks them down, routes tasks, and requests your feedback/permission |
| **Manager** | Project Overseer | Coordinates all agents, tracks progress, assigns tasks, and handles mid-level decisions |
| **Coder** | Development | Writes, debugs, and optimizes code for applications and scripts |
| **Designer** | 3D/Visual Creation | Handles Blender, 3D modeling, character design, and scene layouts |
| **Researcher** | Knowledge & Exploration | Discovers new methods, solutions, and best practices for complex problems |
| **Engine** | Unreal Integration | Sets up game/movie projects, manages assets, and exports scenes |
| **Editor** | Post-Production | Video editing, color grading, cinematic effects, and trailer creation |
| **Integrator** | Final Assembly | Combines outputs from all agents into production-ready deliverables |
| **Architect** | System Design | Designs system architecture and structure |
| **Validator** | Quality Assurance | Tests and validates outputs against requirements |
| **Planner** | Task Planning | Breaks down complex goals into manageable tasks |
| **Modeler** | 3D Content | Creates 3D models and assets |
| **Texturer** | Material Design | Creates textures and materials for 3D models |
| **Animator** | Motion & Animation | Creates animations and motion sequences |
| **Compiler** | Build & Deploy | Handles compilation, building, and deployment tasks |

---


## 🎯 Features

✅ **Unified Input System** - Text prompts, JSON configs, or detailed scene descriptions
✅ **Multi-Engine Support** - Blender (3D), Unreal Engine (Games), DaVinci Resolve (Video)
✅ **LLM-Powered** - Intelligent prompt analysis and scene generation
✅ **Master Templates** - Professional pre-built templates for common use cases
✅ **Autonomous Workflow** - Fully automated from input to output
✅ **Professional Quality** - Cinema-grade rendering and post-production
✅ **Scalable** - Parallel execution, custom extensions, API integration

## 🔄 Workflow Pipeline

### 1. Install Dependencies


pip install -r requirements.txt
|-----------|------------|

### 2. Verify Engine Installation

| Knowledge gaps | Researcher |
python vrindaai_cli.py --help


### 3. Create Your First Project

**Text Prompt to Video:**
- Escalates complex decisions to Assistant
python vrindaai_cli.py --prompt "Create a sci-fi cinematic showing a spaceship" --quality high
### 3. Researcher Agent

**Game from JSON:**
- Scientific/academic research
python vrindaai_cli.py --config examples/game_config.json --engine unreal
- Feature optimization

**Scene from Description:**
- Assistant requesting detailed planning
python vrindaai_cli.py --scene examples/scene.json


## 📋 Input Formats

### Text Prompt (Simplest)
- Game/movie project setup
python vrindaai_cli.py --prompt "Create a 30-second sci-fi video"
- Asset management

### JSON Configuration
### 5. Editor Agent (DaVinci/Resolve)
python vrindaai_cli.py --config config.json
- Color grading & effects

### Scene Description
```bash
python vrindaai_cli.py --scene examples/scene.json
```

## 🏗️ Project Structure

```
VrindaAI/
├── src/                    # Core Python modules
│   ├── core/              # Input processor & orchestrator
│   ├── engines/           # Blender, Unreal, DaVinci wrappers
│   ├── models/            # LLM integration
│   └── utils/             # Utilities
├── examples/              # Usage examples
├── master_templates/      # Professional templates
├── config/                # Configuration files
├── docs/                  # Complete documentation
├── output/                # Generated content
└── vrindaai_cli.py       # Main entry point
```

## 💻 System Requirements

### Minimum
- Windows 10/11 or Linux
- 16GB RAM
- Nvidia GPU (RTX 2060 or better) or CPU fallback
- Python 3.10+

### Recommended
- Windows 11
- 32GB+ RAM  
- Nvidia RTX 3080 or better
- SSD for temp storage

## 🔧 Engine Requirements

Install these separately:

1. **Blender 4.0+**
   ```bash
   # Download from https://www.blender.org/download/
   ```

2. **Unreal Engine 5.2+**
   ```bash
   # Download from https://www.epicgames.com/
   ```

3. **DaVinci Resolve 18+**
   ```bash
   # Download from https://www.blackmagicdesign.com/
   ```

4. **Llama Server** (for LLM)
   ```bash
   pip install llama-cpp-python
   python -m llama_cpp_server --model mistral.gguf --port 8080
   ```

## 📚 Documentation

- **[Complete Guide](docs/COMPLETE_GUIDE.md)** - Full documentation
- **[Examples](examples/)** - Runnable examples
- **[Templates](master_templates/)** - Available templates

## 🎬 Examples

### Example 1: Text to Video
```python
python examples/01_text_to_video.py
```

### Example 2: JSON to Game
```python
python examples/02_json_to_game.py
```

### Example 3: Scene Description
```python
python examples/03_scene_description.py
```

## 🎨 Use Cases

| Use Case | Engine | Input | Output |
|----------|--------|-------|--------|
| Cinematic Video | Blender | Prompt/JSON | .mp4 video |
| Product Visualization | Blender | Scene desc | EXR + Video |
| Game Development | Unreal | Config | .exe game |
| Architectural Render | Blender | Scene desc | 4K image |
| Video Editing | DaVinci | Media files | Edited video |

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
    "port": 8080
  },
  "rendering": {
    "gpu_enabled": true,
    "max_samples": 128
  }
}
```

## 🔌 API Usage

```python
from src.core.input_processor import InputProcessor
from src.core.orchestrator import Orchestrator

# Process input
processor = InputProcessor()
task = processor.process_input("Create a cinematic video")

# Execute workflow
orchestrator = Orchestrator()
result = orchestrator.execute_workflow(task)

print(f"Output: {result['output']}")
```

## 🚨 Troubleshooting

### Engine not found
```bash
python vrindaai_cli.py --verbose  # Check engine paths
```

### Slow rendering
```bash
# Use lower quality
python vrindaai_cli.py --prompt "..." --quality medium --resolution 720p
```

### GPU out of memory
```bash
# Reduce samples and use CPU
python vrindaai_cli.py --prompt "..." --quality low
```

See [Complete Guide](docs/COMPLETE_GUIDE.md) for more troubleshooting.

## 🤝 Contributing

Contributions welcome! Areas for contribution:

- New master templates
- Additional engine support
- Performance optimizations
- Documentation improvements
- Bug fixes

## 📜 License

MIT License - See LICENSE file

## 📞 Support

- **Documentation**: See `docs/` folder
- **Issues**: Check GitHub issues
- **Examples**: Run examples in `examples/` folder

---

## What's New in 2.0

✨ **Major Restructuring**
- Removed 80+ unnecessary files
- Unified input system for all engines
- Modular architecture for easy extension
- Professional documentation

🎯 **New Capabilities**
- LLM-powered prompt analysis
- Multi-engine orchestration
- Master template system
- Unified CLI interface
- Complete API for integration

🚀 **Performance Improvements**
- Parallel execution support
- Optimized rendering
- Better resource management
- Faster job scheduling


**VrindaAI 2.0** - From Imagination to Production

---

## 🔧 Technology Stack

### Backend
- **LLM Framework:** llama.cpp with Mistral 7B model
- **GPU Support:** NVIDIA CUDA (optimized for RTX 4050 and above)
- **Vector Database:** ChromaDB for semantic memory
- **ML Models:** Sentence-Transformers for embeddings

### Frontend
- **GUI Framework:** Qt 6.9.1
- **UI Language:** C++17
- **Database:** SQLite

### Python Services
- **Web Framework:** Flask
- **NLP:** Transformers, Sentence-Transformers
- **Deep Learning:** PyTorch
- **Vector DB:** ChromaDB

---

## ⚙️ Configuration

### Key Settings
- **LLM Server:** `http://127.0.0.1:8080`
- **Memory Service:** Local Flask server
- **Default Model:** Mistral 7B (4.07 GiB)
- **Context Size:** 4096 tokens
- **GPU Optimization:** 99 layers offloaded to CUDA

### Environment Variables
Create a `.env` file if needed:
```
LLAMA_SERVER_URL=http://127.0.0.1:8080
MEMORY_SERVICE_URL=http://127.0.0.1:5000
MODEL_PATH=./llama.cpp/build/bin/Release/mistral.gguf
```

---

## 🛠️ Building from Source

### Prerequisites
- Visual Studio Build Tools or MinGW 64-bit
- CMake 3.19+
- Qt 6.5+ (MinGW 64-bit)

### Build Steps
```bash
# Create build directory
mkdir build
cd build

# Configure
cmake -G "MinGW Makefiles" ..

# Build
cmake --build . --config Release

# Deploy Qt libraries
& "C:\Qt\6.9.1\mingw_64\bin\windeployqt.exe" "path\to\VrindaAI.exe" --no-compiler-runtime
```

---

## 📖 Documentation

- **[CONTRIBUTING.md](CONTRIBUTING.md)** - Development setup and contribution guidelines
- **[README_DEPENDENCIES.md](README_DEPENDENCIES.md)** - Detailed dependency information
- **[prompts/](prompts/)** - Agent system prompts and behavior configuration

---

## 🐛 Troubleshooting

### Llama Server Won't Start
- Verify NVIDIA GPU drivers are installed
- Check that `mistral.gguf` exists in the correct path
- Ensure port 8080 is not in use

### Memory Service Connection Error
- Start Python service in Terminal 2
- Verify Flask is running on `http://127.0.0.1:5000`

### Qt Library Missing
- Run `windeployqt` on the executable
- Ensure Qt6 path is correct

### Agent Not Responding
- Check llama server health: `curl http://127.0.0.1:8080/health`
- Review log files in the project directory
- Restart the application

---

## 📋 System Requirements

| Component | Requirement |
|-----------|------------|
| OS | Windows 10/11 (64-bit) |
| GPU | NVIDIA (RTX 4050 minimum, 6GB+ VRAM) |
| CPU | Intel/AMD 8-core minimum |
| RAM | 16GB minimum, 32GB recommended |
| Storage | 20GB free (for models and projects) |
| Network | Internet for initial setup, local only for operation |

---

## 📜 License

This project is proprietary. See LICENSE file for details.

---

## 🤝 Contributing

Interested in contributing? Please read [CONTRIBUTING.md](CONTRIBUTING.md) for guidelines on:
- Setting up development environment
- Code style standards
- Commit conventions
- Pull request process

---

## 💬 Support & Feedback

For issues, questions, or feature requests:
1. Check existing [issues](../../issues)
2. Review [CONTRIBUTING.md](CONTRIBUTING.md) troubleshooting section
3. Open a new issue with detailed reproduction steps

---

## 🎯 Roadmap

### Phase 1: Core Reliability (In Progress)
- ✅ Multi-agent coordination
- ✅ Agent specialization
- 🔄 JSON-based task planning
- 🔄 State management

### Phase 2: Deep Integration (Planned)
- Asset management system
- Advanced workflow automation
- Error correction loop
- Manual intervention support

### Phase 3: Enterprise Polish (Planned)
- User-facing error UI
- Configuration management
- GUI settings panel
- Professional installer

### Phase 4: Advanced Features (Future)
- Specialist model support
- Long-term memory system
- Deep API integration
- GUI automation

---

## 🚀 Getting Started

**First time?** Start with the [Quick Start](#-quick-start) section above.

**Contributing?** Check out [CONTRIBUTING.md](CONTRIBUTING.md).

**Want to understand the agents?** Read the [Core Agents Overview](#-core-agents-overview).

---

**VrindaAI** - Where AI meets creativity. Transform ideas into reality.
