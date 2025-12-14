#!/usr/bin/env python3
"""
VrindaAI - Autonomous AI Content Creator
Main entry point - unified CLI for creating 3D content across all engines
"""

import argparse
import json
import logging
import sys
from pathlib import Path
from typing import Optional, Dict, Any, Union

# FIX: Force UTF-8 encoding for Windows Consoles to support emojis (🚀, ✅, etc.)
if sys.platform == "win32":
    try:
        # Pylance doesn't see reconfigure on abstract TextIO, so we ignore the type check
        sys.stdout.reconfigure(encoding='utf-8') # type: ignore
        sys.stderr.reconfigure(encoding='utf-8') # type: ignore
    except AttributeError:
        # Fallback for older Python versions
        import codecs
        sys.stdout = codecs.getwriter("utf-8")(sys.stdout.detach()) # type: ignore
        sys.stderr = codecs.getwriter("utf-8")(sys.stderr.detach()) # type: ignore

# Add src to path
sys.path.insert(0, str(Path(__file__).parent / "src"))

from core.input_processor import InputProcessor, InputType
from core.orchestrator import Orchestrator, ExecutionMode


def setup_logging(verbose: bool = False) -> logging.Logger:
    """Setup logging configuration"""
    level = logging.DEBUG if verbose else logging.INFO
    
    # Ensure log directory exists
    log_dir = Path("output/logs")
    log_dir.mkdir(parents=True, exist_ok=True)
    
    # Configure logging with UTF-8 encoding for file handler
    logging.basicConfig(
        level=level,
        format='%(asctime)s - %(name)s - %(levelname)s - %(message)s',
        handlers=[
            # Explicitly use utf-8 for the log file
            logging.FileHandler(log_dir / "vrindaai.log", encoding='utf-8'),
            # Logs go to stderr so stdout is kept clean for JSON output
            logging.StreamHandler(sys.stderr)
        ]
    )
    
    return logging.getLogger("vrindaai_cli")


def main():
    """Main entry point"""
    parser = argparse.ArgumentParser(
        description="VrindaAI - Autonomous AI Content Creator",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:

  # Create cinematic video from text prompt
  python vrindaai_cli.py --prompt "Create a sci-fi cinematic showing a spaceship"
  
  # Create from JSON configuration
  python vrindaai_cli.py --config config.json
  
  # Create from scene description
  python vrindaai_cli.py --scene scene.json
  
  # Create game from prompt
  python vrindaai_cli.py --prompt "Create an action game" --engine unreal
  
  # Dry run - prepare without executing
  python vrindaai_cli.py --prompt "..." --dry-run
        """
    )
    
    # Input options (mutually exclusive)
    input_group = parser.add_mutually_exclusive_group(required=True)
    input_group.add_argument(
        "--prompt", "-p",
        help="Natural language prompt for content creation"
    )
    input_group.add_argument(
        "--config", "-c",
        help="Path to JSON configuration file"
    )
    input_group.add_argument(
        "--scene", "-s",
        help="Path to scene description JSON"
    )
    input_group.add_argument(
        "--file", "-f",
        help="Path to input file (auto-detect format)"
    )
    # JSON argument for C++ Integration
    input_group.add_argument(
        "--json", 
        type=str, 
        help="Raw JSON input string (for C++ Integration)"
    )
    
    # Engine selection
    parser.add_argument(
        "--engine", "-e",
        choices=["blender", "unreal", "davinci"],
        default="blender",
        help="Target engine (default: blender)"
    )
    
    # Output options
    parser.add_argument(
        "--output", "-o",
        default="output",
        help="Output directory for results"
    )
    
    # Execution options
    parser.add_argument(
        "--mode",
        choices=["sequential", "parallel", "interactive"],
        default="sequential",
        help="Execution mode (default: sequential)"
    )
    
    parser.add_argument(
        "--dry-run",
        action="store_true",
        help="Prepare workflow without executing"
    )
    
    # Logging and debugging
    parser.add_argument(
        "--verbose", "-v",
        action="store_true",
        help="Enable verbose logging"
    )
    
    parser.add_argument(
        "--show-manifest",
        action="store_true",
        help="Display job manifests before execution"
    )
    
    # Advanced options
    parser.add_argument(
        "--quality",
        choices=["low", "medium", "high", "ultra"],
        default="high",
        help="Output quality level"
    )
    
    parser.add_argument(
        "--duration",
        type=int,
        default=30,
        help="Duration in seconds (for video/animation)"
    )
    
    parser.add_argument(
        "--resolution",
        choices=["720p", "1080p", "4k"],
        default="1080p",
        help="Output resolution"
    )
    
    args = parser.parse_args()
    
    # Setup logging
    logger = setup_logging(args.verbose)
    logger.info("=" * 60)
    logger.info("VrindaAI - Autonomous Content Creator")
    logger.info("=" * 60)
    
    try:
        # Step 1: Process input
        logger.info("\n📋 Step 1: Processing input...")
        
        input_processor = InputProcessor()
        
        # Determine input
        input_data: Union[str, Path, Dict] = ""
        input_type = None
        
        if args.prompt:
            input_data = args.prompt
            input_type = InputType.TEXT_PROMPT
        elif args.config:
            input_data = args.config
            input_type = InputType.FILE_PATH
        elif args.scene:
            input_data = args.scene
            input_type = InputType.FILE_PATH
        elif args.file:
            input_data = Path(args.file)
        elif args.json:
            try:
                input_data = json.loads(args.json)
                input_type = InputType.JSON_CONFIG
                if isinstance(input_data, dict) and input_data.get("input_type") == "scene_description":
                     input_type = InputType.SCENE_DESCRIPTION
            except json.JSONDecodeError as e:
                logger.error(f"Invalid JSON provided: {e}")
                print(json.dumps({"status": "error", "error": "Invalid JSON input"}))
                return 1
        
        # Process input
        task_spec = input_processor.process_input(input_data, input_type)
        
        # Override parameters if specified
        if args.engine != "blender":
            task_spec["engine"] = args.engine
        
        task_spec["quality"] = args.quality
        task_spec["duration"] = args.duration
        task_spec["resolution"] = args.resolution
        task_spec["output_dir"] = args.output
        
        # Validate
        if not input_processor.validate_task_spec(task_spec):
            logger.error("❌ Invalid task specification")
            print(json.dumps({"status": "failed", "error": "Invalid task specification"}))
            return 1
        
        logger.info(f"✅ Input processed successfully")
        logger.info(f"   Engine: {task_spec.get('engine')}")
        logger.info(f"   Type: {task_spec.get('input_type')}")
        logger.info(f"   Quality: {task_spec.get('quality')}")
        
        # Step 2: Create orchestrator and execute
        logger.info("\n🚀 Step 2: Preparing workflow...")
        
        orchestrator = Orchestrator({
            "output_dir": args.output,
            "verbose": args.verbose
        })
        
        # Execute workflow
        execution_mode = ExecutionMode(args.mode)
        
        logger.info(f"   Mode: {execution_mode.value}")
        logger.info(f"   Dry run: {args.dry_run}")
        
        result = orchestrator.execute_workflow(
            task_spec,
            mode=execution_mode,
            dry_run=args.dry_run
        )
        
        # Step 3: Display results
        logger.info("\n📊 Step 3: Workflow Results")
        logger.info("=" * 60)
        
        logger.info(f"Status: {result['status']}")
        logger.info(f"Workflow ID: {result['workflow_id']}")
        logger.info(f"Engine: {result['engine']}")
        
        if result.get('stages'):
            logger.info("\nStages:")
            for stage in result['stages']:
                logger.info(f"  • {stage['name']}: {stage['status']}")
        
        if result.get('output'):
            logger.info("\nOutputs:")
            for key, value in result['output'].items():
                logger.info(f"  • {key}: {value}")
        
        # Print clean JSON to stdout for C++ integration
        print(json.dumps(result, indent=2, default=str))

        if result['status'] == "failed":
            logger.error(f"\n❌ Error: {result.get('error')}")
            return 1
        else:
            logger.info("\n✅ Workflow completed successfully!")
            
            # Save report
            report_path = Path(args.output) / "logs" / f"workflow_{result['workflow_id']}.json"
            logger.info(f"\nExecution report saved to: {report_path}")
        
        return 0
    
    except KeyboardInterrupt:
        logger.warning("\n⚠️  Workflow cancelled by user")
        return 130
    
    except Exception as e:
        logger.exception(f"❌ Fatal error: {e}")
        print(json.dumps({"status": "failed", "error": str(e)}))
        return 1


if __name__ == "__main__":
    sys.exit(main())