# Contributing to VrindaAI

Thank you for your interest in contributing to VrindaAI! This guide will help you set up the development environment and get started.

## Prerequisites

Before you begin, ensure you have the following installed:
- **Qt 6.5+** (with MinGW 64-bit compiler)
- **CMake 3.19+**
- **Python 3.11+** (for Python dependencies)
- **Conda** or **venv** (for Python environment management)
- **Git**

## Development Setup

### 1. Clone the Repository
```bash
git clone <repository-url>
cd VrindaAI
```

### 2. Set Up Qt Environment (Windows)

First, locate your Qt installation:
```powershell
where windeployqt
# Or search for Qt6 installation:
Get-ChildItem -Path "C:\" -Filter "*Qt*" -Directory -ErrorAction SilentlyContinue | Select-Object -First 10
```

Typical Qt6 location: `C:\Qt\6.9.1\mingw_64`

### 3. Set Up Python Environment

#### Option A: Using Conda
```bash
conda create -n vrindaai python=3.11
conda activate vrindaai
pip install -r requirements.txt
```

#### Option B: Using venv
```bash
python -m venv venv
.\venv\Scripts\activate  # On Windows
source venv/bin/activate  # On macOS/Linux
pip install -r requirements.txt
```

### 4. Build the C++ Project

```bash
# Create build directory
mkdir build
cd build

# Configure with CMake
cmake -G "MinGW Makefiles" ..

# Build the project
cmake --build . --config Release
```

### 5. Run the Application

**Important:** The app requires the llama server to be running first.

#### Terminal 1: Start the Llama Server
```bash
cd llama.cpp/build/bin/Release
.\llama-server.exe --model mistral.gguf -ngl 99 --parallel 4 --threads 8 --host 127.0.0.1 --port 8080 --ctx-size 4096
```

Check server health:
```bash
curl http://127.0.0.1:8080/health
```

#### Terminal 2: Start the Python Memory Service
```bash
cd VrindaAI
conda activate vrindaai
python memory_service.py
```

#### Terminal 3: Run the Application
```bash
cd build/Desktop_Qt_6_9_1_MinGW_64_bit-Debug
.\VrindaAI.exe
```

Or from the project root:
```bash
.\VrindaAI.exe
```

### 6. Deploy Dependencies (Optional)

To create a standalone executable with all Qt dependencies:
```powershell
& "C:\Qt\6.9.1\mingw_64\bin\windeployqt.exe" "path\to\VrindaAI.exe" --no-compiler-runtime
```

## Project Structure

```
VrindaAI/
├── Controllers/          # Agent controllers (Coder, Designer, Manager, etc.)
├── Services/            # Database, Model, and Report Management
├── Utils/               # Utility functions
├── db/                  # Database files
├── llama.cpp/           # LLM inference engine
├── projects/            # User project directories
├── prompts/             # Agent system prompts
├── CMakeLists.txt       # Build configuration
├── main.cpp             # Application entry point
├── mainwindow.cpp/h     # Main UI window
├── memory_service.py    # Vector database service
└── requirements.txt     # Python dependencies
```

## Code Style Guidelines

### C++
- Use C++17 standard
- Follow Qt naming conventions (camelCase for functions, PascalCase for classes)
- Include proper error handling and logging
- Use `nullptr` instead of `NULL`

### Python
- Follow PEP 8 style guidelines
- Use type hints where possible
- Include docstrings for functions and classes

## Commit Guidelines

- Use clear, descriptive commit messages
- Reference issue numbers when applicable (e.g., `Fixes #123`)
- Commit format: `[Type] Brief description`
  - Types: `feat`, `fix`, `docs`, `refactor`, `test`, `chore`
  - Example: `[feat] Add new agent controller for code analysis`

## Testing

Before submitting a PR:
1. Build the project successfully
2. Run the application and test core functionality
3. Check for any compilation warnings or errors
4. Verify the llama server connection works

## Submitting a Pull Request

1. Fork the repository
2. Create a feature branch: `git checkout -b feature/my-feature`
3. Make your changes and commit
4. Push to your fork: `git push origin feature/my-feature`
5. Open a pull request with a clear description of changes

## Troubleshooting

### Qt not found
- Ensure Qt6 is installed and in PATH
- Run `where windeployqt` to verify installation
- Check CMakeLists.txt for correct Qt6 component configuration

### Llama server connection fails
- Verify server is running on `127.0.0.1:8080`
- Check firewall settings
- Ensure mistral.gguf model exists in `llama.cpp/build/bin/Release/`

### Python dependencies fail to install
- Update pip: `pip install --upgrade pip`
- Clear pip cache: `pip cache purge`
- Try installing packages individually for better error messages

## Getting Help

- Check existing issues on GitHub
- Review the main README.md for architecture overview
- Open an issue with detailed reproduction steps if you find a bug

---

**Happy contributing!** 🚀