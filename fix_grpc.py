import os
from pathlib import Path

def fix_imports():
    # Path to the generated gRPC file inside your project structure
    file_path = Path("src/core/proto/vryndara_pb2_grpc.py")
    
    if not file_path.exists():
        print(f"❌ File not found: {file_path}")
        print("Make sure you are in the VrindaAI root folder.")
        return

    with open(file_path, "r") as f:
        content = f.read()

    # The fix: Change 'import vryndara_pb2' to 'from . import vryndara_pb2'
    # This tells Python to look in the current package folder
    old_import = "import vryndara_pb2 as vryndara__pb2"
    new_import = "from . import vryndara_pb2 as vryndara__pb2"

    if old_import in content:
        new_content = content.replace(old_import, new_import)
        with open(file_path, "w") as f:
            f.write(new_content)
        print("✅ Successfully patched vryndara_pb2_grpc.py")
    elif new_import in content:
        print("ℹ️ File is already patched.")
    else:
        print("⚠️ Could not find the specific import line. Check file manually.")

if __name__ == "__main__":
    fix_imports()