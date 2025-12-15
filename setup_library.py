"""
VrindaAI - Asset Library Setup
Creates the standardized folder structure for the AAA Pipeline.
"""

import os
from pathlib import Path

# Configuration from settings.json
LIBRARY_ROOT = r"C:\Vrinda_Assets"

STRUCTURE = [
    "Raw_Downloads",        # Zip files from Sketchfab/Quixel
    "Processed_Meshes",     # FBX files ready for Unreal (Auto-rigged)
    "Animations",           # Universal .anim/.fbx files
    "Renders",              # Image sequences output by Unreal
    "Final_Edits",          # Final video files from FFmpeg
    "Temp"                  # Intermediate files
]

def setup_library():
    root = Path(LIBRARY_ROOT)
    
    print(f"📂 Setting up Asset Library at: {root}")
    
    try:
        root.mkdir(parents=True, exist_ok=True)
        print(f"✅ Root created: {root}")
        
        for folder in STRUCTURE:
            path = root / folder
            path.mkdir(exist_ok=True)
            print(f"  ├── ✅ {folder}/")
            
        print("\n🎉 Asset Library is ready!")
        print("Action Item: Drop your raw .obj/.fbx models into 'Raw_Downloads'")
        print("Action Item: Drop mixamo animations into 'Animations'")
        
    except PermissionError:
        print("❌ Error: Permission denied. Run this script as Administrator.")
    except Exception as e:
        print(f"❌ Error: {e}")

if __name__ == "__main__":
    setup_library()