import sys
import os
import time

# Ensure imports work
sys.path.append(os.path.join(os.getcwd(), "src"))

from src.core.orchestrator import create_orchestrator

def test_engineering_offload():
    print("--- 🧪 Testing Neural Link (Phase 1) ---")
    
    # 1. Initialize Orchestrator (it connects to localhost:50051)
    orchestrator = create_orchestrator({
        "kernel_address": "localhost:50051",
        "output_dir": "./output"
    })
    
    # 2. Define an Engineering Task (Triggers 'Offload')
    task = {
        "description": "Design a 50mm Warp Ring with gyroid internal lattice",
        "engine": "blender" # Engine doesn't matter, 'design' keyword triggers offload
    }
    
    # 3. Execute
    result = orchestrator.execute_workflow(task)
    
    # 4. Verify
    print("\n--- Result ---")
    print(f"Status: {result['status']}")
    print(f"Message: {result.get('message')}")
    
    if result['status'] == "offloaded":
        print("\n✅ SUCCESS: Task was successfully offloaded to Vryndara!")
    else:
        print("\n❌ FAILURE: Task ran locally instead of offloading.")

if __name__ == "__main__":
    test_engineering_offload()