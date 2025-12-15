"""
VrindaAI - Connection Tester (AAA Pipeline Version)
Verifies that Python can find and control Blender, Unreal, and FFmpeg.
"""

import os
import sys
import subprocess
import platform
import shutil
from pathlib import Path

# --- CONFIGURATION ---
PATHS = {
    "output_dir": r"C:\Users\Mahantesh\DevelopmentProjects\VrindaAI\VrindaAI\output",
    
    # FFmpeg (The New Editor)
    "ffmpeg": r"C:\ffmpeg\bin\ffmpeg.exe",
    
    # Blender (The Auto-Rigger)
    "blender": [
        r"C:\Program Files\Blender Foundation\Blender 4.3\blender.exe",
        r"C:\Program Files\Blender Foundation\Blender 4.2\blender.exe",
    ],
    
    # Unreal Engine (The Director)
    "unreal": [
        r"C:\Program Files\Epic Games\UE_5.3\Engine\Binaries\Win64\UnrealEditor-Cmd.exe",
        r"C:\Program Files\Epic Games\UE_5.2\Engine\Binaries\Win64\UnrealEditor-Cmd.exe",
        r"C:\Program Files\Epic Games\UE_5.6\Engine\Binaries\Win64\UnrealEditor-Cmd.exe",
    ],
}

def print_status(component, status, message=""):
    icon = "✅" if status else "❌"
    print(f"{icon} {component}: {message}")

def test_directory_access():
    """Test 1: Can we write to your specific output folder?"""
    path = Path(PATHS["output_dir"])
    try:
        path.mkdir(parents=True, exist_ok=True)
        test_file = path / "handshake_test.txt"
        test_file.write_text("VrindaAI can write here.")
        if test_file.exists():
            test_file.unlink() # Delete it
            print_status("Output Directory", True, f"Writable at {path}")
            return True
    except Exception as e:
        print_status("Output Directory", False, f"Failed: {e}")
        return False

def test_blender_connection():
    """Test 2: Can we call Blender headlessly?"""
    found_path = None
    for p in PATHS["blender"]:
        if os.path.exists(p):
            found_path = p
            break
            
    if not found_path:
        # Check PATH as fallback
        if shutil.which("blender"):
            found_path = "blender"
        else:
            print_status("Blender", False, "Executable not found in standard paths.")
            return False

    try:
        # Ask Blender for its version
        result = subprocess.run([found_path, "-b", "--version"], capture_output=True, text=True)
        if result.returncode == 0:
            version = result.stdout.splitlines()[0]
            print_status("Blender", True, f"Connected ({version})")
            return True
    except Exception as e:
        print_status("Blender", False, f"Execution error: {e}")
        return False

def test_unreal_connection():
    """Test 3: Is Unreal Engine installed and command-line ready?"""
    found_path = None
    for p in PATHS["unreal"]:
        if os.path.exists(p):
            found_path = p
            break
            
    if not found_path:
        print_status("Unreal Engine", False, "UnrealEditor-Cmd.exe not found.")
        return False
    
    print_status("Unreal Engine", True, f"Found at {found_path}")
    return True

def test_ffmpeg_connection():
    """Test 4: Is FFmpeg installed and accessible?"""
    found_path = None
    
    # 1. Check explicit path
    if os.path.exists(PATHS["ffmpeg"]):
        found_path = PATHS["ffmpeg"]
    
    # 2. Check System PATH
    if not found_path:
        found_path = shutil.which("ffmpeg")

    if not found_path:
        print_status("FFmpeg", False, "Executable not found. Please install FFmpeg and add to PATH.")
        return False

    try:
        result = subprocess.run([found_path, "-version"], capture_output=True, text=True)
        if result.returncode == 0:
            # Extract version (first line usually contains version info)
            version_info = result.stdout.splitlines()[0].split("version")[1].strip().split(" ")[0]
            print_status("FFmpeg", True, f"Connected (Version: {version_info})")
            return True
    except Exception as e:
        print_status("FFmpeg", False, f"Execution error: {e}")
        return False

if __name__ == "__main__":
    print(f"--- VrindaAI Handshake Protocol (AAA Pipeline) ---")
    print(f"System: {platform.system()} {platform.release()}")
    print("-" * 35)
    
    test_directory_access()
    test_blender_connection()
    test_unreal_connection()
    test_ffmpeg_connection()
    
    print("-" * 35)