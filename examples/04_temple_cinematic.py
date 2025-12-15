"""
VrindaAI AAA Example: Temple Cinematic
Demonstrates the full pipeline:
1. Create Unreal Project
2. Ingest 'VPTemple...' from Epic Vault
3. Create Cinematic Sequence
"""

import sys
import json
import logging
from pathlib import Path

# Add src to path so we can import modules
sys.path.insert(0, str(Path(__file__).parent.parent / "src"))

from core.orchestrator import Orchestrator, ExecutionMode

# Setup simple logging to console
logging.basicConfig(level=logging.INFO)

def run_temple_pipeline():
    # 1. Load Configuration
    with open("config/settings.json") as f:
        config = json.load(f)

    # 2. Define the Task
    # We explicitly ask for the asset we found in your Vault
    task_spec = {
        "description": "Ancient Temple Cinematic",
        "engine": "unreal",
        "type": "cinematic",
        "assets": [
            "VPTemple99388731072fV2"  # <--- EXACT FOLDER NAME FROM YOUR VAULT
        ]
    }

    # 3. Initialize Orchestrator
    orchestrator = Orchestrator(config)

    print("\n🚀 Starting AAA Temple Pipeline...")
    print(f"Target Asset: {task_spec['assets'][0]}")

    # 4. Execute
    result = orchestrator.execute_workflow(
        task_spec,
        mode=ExecutionMode.SEQUENTIAL,
        dry_run=False # Set to True to just see the plan
    )

    print("\n" + "="*40)
    print(f"Workflow ID: {result.get('workflow_id')}")
    print(f"Status: {result.get('status')}")
    if result.get('status') == 'failed':
        print(f"Error: {result.get('error')}")
    else:
        print("✅ Project Created & Assets Ingested!")
        print("Check 'output/' folder for the Unreal Project.")

if __name__ == "__main__":
    run_temple_pipeline()