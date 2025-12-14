#!/usr/bin/env python3
"""
VrindaAI Example 2: JSON Workflow to Game
Create complete games from structured JSON configurations
"""

import sys
import json
from pathlib import Path

sys.path.insert(0, str(Path(__file__).parent.parent / "src"))

from core.input_processor import InputProcessor
from core.orchestrator import Orchestrator, ExecutionMode


def example_json_to_game():
    """
    Example: Create a game from JSON configuration
    """
    
    # JSON configuration for a game
    game_config = {
        "type": "game",
        "engine": "unreal",
        "description": "Action-adventure game with exploration and combat",
        "style": "realistic",
        "quality": "high",
        "assets": [
            "player_character",
            "enemies_pack",
            "environment_forest",
            "weapons_pack"
        ],
        "templates": ["game_starter", "action_mechanics"],
        "parameters": {
            "game_mode": "action_adventure",
            "target_platform": "windows",
            "multiplayer": False,
            "graphics_quality": "high"
        },
        "game_mechanics": {
            "movement": "wasd_keys",
            "combat": "mouse_click",
            "inventory": "tab_key",
            "jumping": "spacebar"
        }
    }
    
    print("=" * 60)
    print("VrindaAI Example 2: JSON to Game")
    print("=" * 60)
    print(f"\nGame Configuration:")
    print(json.dumps(game_config, indent=2))
    
    # Process JSON configuration
    print("\n1️⃣  Processing JSON configuration...")
    input_processor = InputProcessor()
    task_spec = input_processor.process_input(game_config)
    
    print(f"   ✓ Engine: {task_spec.get('engine')}")
    print(f"   ✓ Type: {task_spec.get('type')}")
    print(f"   ✓ Assets: {len(task_spec.get('assets', []))} items")
    print(f"   ✓ Quality: {task_spec.get('quality')}")
    
    # Setup orchestration
    print("\n2️⃣  Setting up Unreal Engine project...")
    orchestrator = Orchestrator({
        "output_dir": "output/example2_game"
    })
    
    # Execute (dry run)
    print("\n3️⃣  Executing game creation workflow...")
    result = orchestrator.execute_workflow(
        task_spec,
        mode=ExecutionMode.SEQUENTIAL,
        dry_run=True
    )
    
    print(f"\n4️⃣  Workflow Status: {result['status']}")
    print(f"   Workflow ID: {result['workflow_id']}")
    
    if result.get('job_count'):
        print(f"   Jobs to execute: {result['job_count']}")
    
    print("\n" + "=" * 60)


def example_multiplayer_game():
    """
    Example: Create a multiplayer game
    """
    
    multiplayer_config = {
        "type": "game",
        "engine": "unreal",
        "description": "Multiplayer battle royale game",
        "style": "stylized",
        "quality": "high",
        "parameters": {
            "game_mode": "multiplayer_battle_royale",
            "max_players": 100,
            "map_size": "large",
            "respawn_enabled": False,
            "cross_platform": True
        },
        "assets": [
            "player_models_pack",
            "weapons_full_suite",
            "large_map_island",
            "ui_system_battle_royale",
            "networking_system"
        ]
    }
    
    print("\nMultiplayer Game Example")
    print("-" * 40)
    print(json.dumps(multiplayer_config, indent=2))


if __name__ == "__main__":
    example_json_to_game()
    example_multiplayer_game()
