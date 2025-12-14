# VrindaAI - Multi-Agent AI Desktop Application

A powerful multi-agent AI system that automates creative and technical workflows. VrindaAI coordinates specialized agents to handle coding, 3D design, video editing, project management, and more—enabling you to describe a goal and watch it come to life.

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

## 🔄 Workflow Pipeline

### 1. You → Assistant
- **Input:** Describe your goal (e.g., "Make a game trailer", "Create cinematic short", "Build a JARVIS bot")
- **Assistant:** 
  - Asks clarifying questions if needed
  - Breaks task into sub-tasks
  - Routes to Manager for execution

### 2. Manager → Specialized Agents
Based on task type, Manager assigns work:

| Task Type | Assigned To |
|-----------|------------|
| New code/scripts | Coder |
| 3D/design work | Designer |
| Knowledge gaps | Researcher |
| Game/cinematic setup | Engine |
| Video post-production | Editor |
| Final assembly | Integrator |

**Manager also:**
- Uses Researcher to find solutions when stuck
- Escalates complex decisions to Assistant
- Tracks project progress

### 3. Researcher Agent
Handles:
- Technical solutions & best practices
- Scientific/academic research
- Innovative approaches
- Feature optimization

**Triggered by:**
- Assistant requesting detailed planning
- Manager encountering blockers
- Complex problem solving

### 4. Engine Agent (Unreal)
Controls:
- Game/movie project setup
- Environmental configuration
- Asset management
- Scene exports for editing

### 5. Editor Agent (DaVinci/Resolve)
Produces:
- Frame sequences & cuts
- Color grading & effects
- Trailers & cinematic edits
- Final video outputs

### 6. Integrator Agent
Combines:
- Code files
- 3D designs
- Animations
- Videos
- Exports in specified formats

---

## 📁 Project Structure

```
VrindaAI/
├── Controllers/              # Agent controllers
│   ├── animationcontroller.*
│   ├── blendercontroller.*
│   ├── compilercontroller.*
│   ├── llamaservercontroller.*
│   ├── modelingcontroller.*
│   ├── projectstatecontroller.*
│   ├── projectworkflow.*
│   ├── texturingcontroller.*
│   ├── unrealcontroller.*
│   └── validatorcontroller.*
├── Services/                 # Core services
│   ├── databasemanager.*
│   ├── modelmanager.*
│   ├── projectmanager.*
│   ├── reportgenerator.*
│   └── vectordatabasemanager.*
├── Utils/                    # Utility functions
├── db/                       # Database files
├── llama.cpp/               # LLM inference engine
├── projects/                # User project directories
├── prompts/                 # Agent system prompts
├── styles/                  # UI styling
├── CMakeLists.txt           # Build configuration
├── main.cpp                 # Application entry point
├── mainwindow.cpp/h         # Main UI window
├── memory_service.py        # Vector database service
└── requirements.txt         # Python dependencies
```

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
