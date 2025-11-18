# Dependency install instructions (Windows PowerShell)

This project uses Qt6 (Core, Widgets, Gui, Sql, PrintSupport, Network, LinguistTools) and a small Python service. When you have internet access you can install everything with the commands below.

1) Install Python dependencies (run from the `VrindaAI` project folder):

```powershell
# Install Python packages listed in requirements.txt
python -m pip install --upgrade pip; python -m pip install -r requirements.txt
```

2) Install C++ dependencies using vcpkg (one-line):

```powershell
# Clone vcpkg, bootstrap it, then install packages declared in vcpkg.json (run from the folder containing vcpkg.json)
git clone https://github.com/microsoft/vcpkg.git; Set-Location vcpkg; .\bootstrap-vcpkg.bat; .\vcpkg.exe install --triplet x64-windows
```

Notes and tips
- The `vcpkg.json` file (manifest mode) is located in the project root and declares Qt6 base/tools, `nlohmann-json` and `sqlite3`.
- After vcpkg installs packages, configure CMake in VS Code to use vcpkg's toolchain. Example CMake configure arguments:

```powershell
-DCMAKE_TOOLCHAIN_FILE="<path-to-vcpkg>\scripts\buildsystems\vcpkg.cmake" -DVCPKG_TARGET_TRIPLET=x64-windows
```

- If you prefer the official Qt installer from Qt (qt.io), you can install Qt 6.5+ instead of using vcpkg. When using the official installer, ensure CMake can find Qt (set Qt6_DIR or use the Qt VS Tools).
- `requirements.txt` contains Python packages used by `memory_service.py`. The first run of `chromadb`/`sentence-transformers` may download model artifacts requiring internet. If you need fully offline usage, download required models before going offline.

If you'd like, I can: (a) add a VS Code `settings.json` and `cmake-kits.json` snippet to wire vcpkg into CMake Tools, or (b) expand the vcpkg manifest with more Qt features (linguist, svg, etc.). Which would you prefer?
