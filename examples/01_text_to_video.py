#!/usr/bin/env python3
"""
VrindaAI Example 1: Text Prompt to Cinematic Video
Create professional 3D cinematic videos from simple text descriptions
"""

import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).parent.parent / "src"))

from core.input_processor import InputProcessor
from core.orchestrator import Orchestrator, ExecutionMode


def example_text_to_video():
    """
    Example: Convert text prompt to cinematic video
    """
    
    # Simple text prompt
    prompt = """
    Create a cinematic sci-fi movie showing a futuristic spaceship 
    approaching a distant planet. The camera should start far away 
    and smoothly zoom in as the ship moves closer. Include dramatic 
    lighting and a nebula background. Duration: 30 seconds, realistic style.
    """
    
    print("=" * 60)
    print("VrindaAI Example 1: Text to Cinematic Video")
    print("=" * 60)
    print(f"\nPrompt: {prompt.strip()}\n")
    
    # Step 1: Process the text prompt
    print("1️⃣  Processing text prompt...")
    input_processor = InputProcessor()
    task_spec = input_processor.process_input(prompt)
    
    print(f"   ✓ Engine detected: {task_spec.get('engine')}")
    print(f"   ✓ Output type: {task_spec.get('output_type')}")
    print(f"   ✓ Duration: {task_spec.get('duration')} seconds")
    print(f"   ✓ Quality: {task_spec.get('quality')}")
    
    # Step 2: Create orchestrator
    print("\n2️⃣  Setting up orchestration...")
    orchestrator = Orchestrator({
        "output_dir": "output/example1",
        "verbose": True
    })
    
    # Step 3: Execute workflow (dry run for example)
    print("\n3️⃣  Executing workflow (dry run)...")
    result = orchestrator.execute_workflow(
        task_spec,
        mode=ExecutionMode.SEQUENTIAL,
        dry_run=True  # Set to False to actually render
    )
    
    # Step 4: Display results
    print("\n4️⃣  Results:")
    print(f"   Status: {result['status']}")
    print(f"   Workflow ID: {result['workflow_id']}")
    
    if result.get('stages'):
        print("\n   Stages:")
        for stage in result['stages']:
            print(f"      • {stage['name']}: {stage['status']}")
    
    print("\n" + "=" * 60)
    print("✅ Example completed! To run actual rendering, set dry_run=False")
    print("=" * 60)


def example_cinematic_features():
    """
    Example with advanced cinematic features
    """
    
    advanced_prompt = """
    Create a cinematic short film about a futuristic city. 
    Include:
    - Camera path: crane shot starting from ground level rising to sky
    - Lighting: blue and orange color grading, dramatic shadows
    - Duration: 60 seconds at 4K resolution
    - Style: sci-fi cyberpunk
    - Effects: motion blur, depth of field
    - Music: cinematic orchestral (background)
    - Quality: ultra (maximum detail)
    """
    
    print("\nAdvanced Example: Cinematic Short Film")
    print("-" * 40)
    
    input_processor = InputProcessor()
    task_spec = input_processor.process_input(advanced_prompt)
    
    # Additional customization
    task_spec["resolution"] = "4k"
    task_spec["quality"] = "ultra"
    task_spec["templates"] = ["cinematic_master", "color_grade_cinematic"]
    
    print(f"Task spec generated:")
    print(f"  Duration: {task_spec['duration']}s")
    print(f"  Resolution: {task_spec.get('resolution')}")
    print(f"  Quality: {task_spec.get('quality')}")
    print(f"  Templates: {task_spec.get('templates')}")


if __name__ == "__main__":
    example_text_to_video()
    example_cinematic_features()
