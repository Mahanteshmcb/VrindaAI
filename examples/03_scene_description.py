#!/usr/bin/env python3
"""
VrindaAI Example 3: Scene Description to 3D Scene
Create detailed 3D scenes from structured scene descriptions
"""

import sys
import json
from pathlib import Path

sys.path.insert(0, str(Path(__file__).parent.parent / "src"))

from core.input_processor import InputProcessor
from core.orchestrator import Orchestrator, ExecutionMode


def example_scene_description():
    """
    Example: Create a 3D scene from scene description
    """
    
    # Detailed scene description
    scene_desc = {
        "name": "Futuristic Laboratory",
        "engine": "blender",
        "environment": "indoor_high_tech",
        "description": "A futuristic research laboratory with advanced equipment",
        
        "objects": [
            {
                "name": "central_computer",
                "type": "model",
                "material": "metallic",
                "position": [0, 0, 0],
                "scale": [2, 2, 2],
                "animation": {
                    "type": "rotation",
                    "axis": "z",
                    "speed": 1.0
                }
            },
            {
                "name": "research_table",
                "type": "model",
                "material": "wood_polished",
                "position": [-5, 0, 0],
                "scale": [1, 2, 1]
            },
            {
                "name": "glass_tube_01",
                "type": "model",
                "material": "glass_translucent",
                "position": [3, 1, 0],
                "scale": [0.5, 3, 0.5],
                "animation": {
                    "type": "emissive_glow",
                    "color": [0, 1, 1],
                    "intensity": 2.0
                }
            },
            {
                "name": "robot_arm",
                "type": "model",
                "material": "metallic_chrome",
                "position": [0, 3, 2],
                "rotation": [45, 0, 0],
                "animation": {
                    "type": "armature",
                    "animation_name": "working_motion"
                }
            }
        ],
        
        "camera": {
            "type": "cinematic",
            "position": [10, 5, 8],
            "target": [0, 0, 0],
            "fov": 45,
            "path_animation": {
                "type": "orbit",
                "radius": 15,
                "duration": 240,
                "height_variation": 3
            }
        },
        
        "lighting": {
            "environment": {
                "type": "sun",
                "angle": 45,
                "intensity": 2.0,
                "color": [1, 0.95, 0.9]
            },
            "key_light": {
                "type": "area",
                "position": [5, 4, 3],
                "intensity": 1500,
                "color": [1, 1, 1]
            },
            "fill_light": {
                "type": "area",
                "position": [-5, 2, 2],
                "intensity": 800,
                "color": [0.8, 0.9, 1]
            },
            "rim_light": {
                "type": "area",
                "position": [0, -4, 4],
                "intensity": 1000,
                "color": [1, 0.5, 0.2]
            }
        },
        
        "rendering": {
            "engine": "cycles",
            "device": "gpu",
            "samples": 128,
            "denoiser": "optix",
            "resolution": [1920, 1080],
            "fps": 24,
            "frame_start": 1,
            "frame_end": 240
        },
        
        "effects": [
            {
                "type": "volume_scatter",
                "density": 0.1,
                "color": [0.9, 0.95, 1.0]
            },
            {
                "type": "bloom",
                "intensity": 0.5,
                "threshold": 0.8
            }
        ]
    }
    
    print("=" * 60)
    print("VrindaAI Example 3: Scene Description")
    print("=" * 60)
    print(f"\nScene: {scene_desc['name']}")
    print(f"Engine: {scene_desc['engine']}")
    print(f"Objects: {len(scene_desc['objects'])}")
    
    # Process scene description
    print("\n1️⃣  Processing scene description...")
    input_processor = InputProcessor()
    task_spec = input_processor.process_input(scene_desc)
    
    print(f"   ✓ Engine: {task_spec.get('engine')}")
    print(f"   ✓ Objects: {len(task_spec.get('objects', []))}")
    print(f"   ✓ Camera animation: {task_spec.get('camera', {}).get('path_animation', {}).get('type')}")
    
    # Create orchestrator
    print("\n2️⃣  Setting up Blender rendering...")
    orchestrator = Orchestrator({
        "output_dir": "output/example3_scene"
    })
    
    # Execute workflow
    print("\n3️⃣  Executing scene creation workflow...")
    result = orchestrator.execute_workflow(
        task_spec,
        mode=ExecutionMode.SEQUENTIAL,
        dry_run=True
    )
    
    print(f"\n4️⃣  Result:")
    print(f"   Status: {result['status']}")
    print(f"   Output frames: output/renders/frames/")
    print(f"   Duration: 10 seconds (240 frames @ 24fps)")
    
    print("\n" + "=" * 60)


def example_architectural_scene():
    """
    Example: Architectural visualization scene
    """
    
    arch_scene = {
        "name": "Modern House Exterior",
        "engine": "blender",
        "environment": "outdoor_daytime",
        "objects": [
            {
                "name": "house_structure",
                "type": "model",
                "material": "concrete_light",
                "position": [0, 0, 0]
            },
            {
                "name": "glass_windows",
                "type": "model",
                "material": "glass_reflective",
                "position": [0, 0, 2]
            },
            {
                "name": "landscaping_trees",
                "type": "model",
                "material": "foliage_natural",
                "position": [-10, 5, 0]
            }
        ],
        "camera": {
            "type": "static",
            "position": [20, 15, 8],
            "target": [0, 0, 3]
        },
        "rendering": {
            "samples": 256,
            "device": "gpu"
        }
    }
    
    print("\nArchitectural Visualization Scene")
    print("-" * 40)
    print(json.dumps(arch_scene, indent=2)[:300] + "...")


if __name__ == "__main__":
    example_scene_description()
    example_architectural_scene()
