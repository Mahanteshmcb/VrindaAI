import os
import sys
import subprocess
from pathlib import Path

def find_proto_file(start_dir):
    """Searches for vryndara.proto in sibling directories going up 4 levels."""
    current = start_dir
    # Check up to 4 levels up
    for _ in range(5):
        # Look for 'vryndara' or 'Vryndara' folder in current directory
        if current.exists():
            for child in current.iterdir():
                if child.is_dir() and child.name.lower().startswith("vryndara"):
                    # Check common paths inside
                    candidate = child / "protos" / "vryndara.proto"
                    if candidate.exists():
                        return candidate
                    candidate = child / "Vryndara_Core" / "protos" / "vryndara.proto"
                    if candidate.exists():
                        return candidate

        # Move up one level
        if current.parent == current:
            break # Hit root
        current = current.parent
    return None

def main():
    # Get the directory where this script is running
    script_dir = Path(__file__).parent.resolve()
    print(f"🔧 Setting up VrindaAI Workspace from: {script_dir}")

    # 1. Find the Source Proto
    print("   🔍 Searching for 'vryndara.proto'...")
    proto_src = find_proto_file(script_dir)
    
    if not proto_src:
        print("❌ CRITICAL ERROR: Could not find 'vryndara.proto'.")
        print("   Please ensure the 'vryndara' repository is downloaded and sits near 'VrindaAI'.")
        return

    print(f"   ✅ Found proto file at: {proto_src}")

    # 2. Ensure Output Directory Exists
    proto_out_dir = script_dir / "src" / "core" / "proto"
    if not proto_out_dir.exists():
        print(f"   Creating directory: {proto_out_dir}")
        proto_out_dir.mkdir(parents=True, exist_ok=True)
    
    # Create __init__.py to make it a package
    (proto_out_dir / "__init__.py").touch()

    # 3. Compile Protos
    print("   ⚙️  Compiling gRPC definitions...")
    cmd = [
        sys.executable, "-m", "grpc_tools.protoc",
        f"-I{proto_src.parent}",
        f"--python_out={proto_out_dir}",
        f"--grpc_python_out={proto_out_dir}",
        str(proto_src)
    ]
    
    result = subprocess.run(cmd, capture_output=True, text=True)
    if result.returncode != 0:
        print(f"❌ Protoc Compilation Failed:\n{result.stderr}")
        return
    else:
        print("   ✅ Compilation successful.")

    # 4. Patch the Imports (The Pylance Fix)
    grpc_file = proto_out_dir / "vryndara_pb2_grpc.py"
    if grpc_file.exists():
        print("   🩹 Patching vryndara_pb2_grpc.py imports...")
        with open(grpc_file, "r") as f:
            content = f.read()
        
        # The Fix: Switch absolute import to relative
        new_content = content.replace(
            "import vryndara_pb2 as vryndara__pb2", 
            "from . import vryndara_pb2 as vryndara__pb2"
        )
        
        with open(grpc_file, "w") as f:
            f.write(new_content)
        print("   ✅ Patch applied.")

    print("\n🎉 Workspace Setup Complete! You can now run the tests.")

if __name__ == "__main__":
    main()