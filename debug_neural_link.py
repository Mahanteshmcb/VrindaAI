import sys
import os
import grpc
import logging
from pathlib import Path

# Configure logging to show up in the terminal
logging.basicConfig(level=logging.INFO, format='%(levelname)s: %(message)s')
logger = logging.getLogger("Debugger")

def debug():
    print("\n--- 🕵️‍♂️ VrindaAI Neural Link Debugger ---\n")

    # 1. FIX SYSTEM PATH (Simulate Orchestrator)
    root_path = Path(__file__).parent.resolve()
    sys.path.append(str(root_path))
    print(f"📂 Project Root: {root_path}")

    # 2. TEST IMPORT
    print("\n[1] Testing Imports...")
    try:
        from src.core.proto import vryndara_pb2, vryndara_pb2_grpc
        print("   ✅ Protos imported successfully.")
        print(f"   ℹ️  Proto Module: {vryndara_pb2}")
    except ImportError as e:
        print(f"   ❌ IMPORT FAILED: {e}")
        print("   👉 Run 'python setup_workspace.py' again.")
        return

    # 3. TEST PROTO ATTRIBUTES
    print("\n[2] Checking Proto Definitions...")
    try:
        # Check if PublishRequest exists
        req_class = getattr(vryndara_pb2, 'PublishRequest', None)
        if req_class:
            print("   ✅ PublishRequest found.")
        else:
            print("   ❌ PublishRequest NOT found in vryndara_pb2!")
            print(f"   ℹ️  Available contents: {dir(vryndara_pb2)}")
            return
    except Exception as e:
        print(f"   ❌ Error checking protos: {e}")
        return

    # 4. TEST CONNECTION TO KERNEL
    print("\n[3] Testing Connection to Kernel...")
    target = 'localhost:50051'
    print(f"   📡 Connecting to {target}...")
    
    try:
        channel = grpc.insecure_channel(target)
        stub = vryndara_pb2_grpc.KernelStub(channel)
        
        # Try a simple Register call to ping the server
        # We use a dummy ID just to see if the server responds
        import time
        ping_request = vryndara_pb2.AgentInfo(
            id="debug_probe",
            name="Debugger",
            type="PROBE",
            capabilities=["ping"],
            status="ACTIVE",
            endpoint="none"
        )
        
        # Set a short timeout so we don't hang forever
        response = stub.Register(ping_request, timeout=5)
        
        if response.success:
            print("   ✅ CONNECTION SUCCESSFUL!")
            print("   🎉 The Kernel is online and responding.")
        else:
            print("   ⚠️  Server reachable, but returned failure.")
            print(f"   Response: {response}")

    except grpc.RpcError as e:
        print(f"   ❌ CONNECTION FAILED: {e.code()}")
        print(f"   Details: {e.details()}")
        print("\n   👉 SOLUTION: Is the Vryndara Kernel running?")
        print("   1. Open a new terminal.")
        print("   2. cd into 'vryndara' folder.")
        print("   3. Run: python kernel/main.py")

if __name__ == "__main__":
    debug()