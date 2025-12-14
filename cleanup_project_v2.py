#!/usr/bin/env python3
"""
VrindaAI Project Cleanup Script
Removes unnecessary files and prepares for restructuring
"""

import os
import shutil
from pathlib import Path

# Files and directories to remove
TO_REMOVE = [
    # Legacy Python video scripts
    "autonomous_video_creator.py",
    "create_video.py",
    "fast_video_creator.py",
    "quick_video_creator.py",
    "generate_demo_video.py",
    "check_video_system.py",
    "test_video_pipeline.py",
    "example.py",
    "autonomous_demo.py",
    "memory_service.py",  # Legacy service
    
    # Batch files (obsolete)
    "create_video.bat",
    "run_cinematic_pipeline.bat",
    "run_headless_workflow.bat",
    "run_vrindaai.bat",
    "run_workflow.bat",
    "test_headless.bat",
    
    # Deprecated documentation (40+ files)
    "00_COMPLETE_AND_READY.md",
    "00_START_HERE.md",
    "00_VIDEO_SYSTEM_GUIDE.md",
    "00_VIDEO_SYSTEM_READY.md",
    "ACTUAL_WORKING_EXAMPLES.md",
    "ARCHITECTURE_COMPLETE.md",
    "ARCHITECTURE_JOB_MANIFESTS.md",
    "AUTONOMOUS_VIDEO_COMPLETE.md",
    "AUTONOMOUS_VIDEO_GUIDE.md",
    "AUTONOMOUS_VIDEO_README.md",
    "AUTONOMOUS_VIDEO_SYSTEM_READY.md",
    "AUTONOMOUS_WORKFLOW_COMPLETE.md",
    "AUTONOMOUS_WORKFLOW_DOCUMENTATION_INDEX.md",
    "AUTONOMOUS_WORKFLOW_INTEGRATION.md",
    "AUTONOMOUS_WORKFLOW_STATUS.md",
    "CINEMATIC_PIPELINE_COMPLETE.md",
    "CINEMATIC_QUICK_START.md",
    "CREATE_VIDEO_GUIDE.md",
    "ERRORS_FIXED.md",
    "ERROR_RESOLUTION.md",
    "EXECUTIVE_SUMMARY.md",
    "FINAL_RESOLUTION_REPORT.md",
    "FINAL_STATUS_REPORT.md",
    "HEADLESS_EXECUTION_STATUS.md",
    "HEADLESS_OPERATION_SUCCESS.md",
    "HEADLESS_QUICK_REFERENCE.md",
    "IMPLEMENTATION_COMPLETE.md",
    "IMPLEMENTATION_SUMMARY.md",
    "PROBLEMS_RESOLVED.md",
    "PROFESSIONAL_CINEMA_PIPELINE_GUIDE.md",
    "PROJECT_COMPLETION_REPORT.md",
    "QUICK_START_AUTONOMOUS_WORKFLOWS.md",
    "QUICKSTART_JOBS.md",
    "README_CINEMATIC_SYSTEM.md",
    "README_DEPENDENCIES.md",
    "START_HERE_AUTONOMOUS_WORKFLOWS.md",
    "START_HERE_VIDEOS.md",
    "TEST_RESULTS_AND_STATUS.md",
    "VIDEOS_COMPLETE_GUIDE.md",
    "VIDEO_CREATED_SUCCESS.md",
    "VIDEO_CREATION_INDEX.md",
    "VIDEO_QUICK_REFERENCE.md",
    "VIDEO_SYSTEM_READY.md",
    "VIDEO_SYSTEM_STATUS.md",
    "DOCUMENTATION_INDEX.md",
    
    # Test/Config files
    "test_blender_simple.json",
    "test_simple_job.json",
    "test_simple_workflow.json",
    "test_workflow.json",
    "headless_execution.log",
    "CMakeLists.txt.user",
    "VrindaAI_sa_IN.ts",
    "VIDEO_SYSTEM_READY.txt",
    
    # Directories
    "temp_renders",
    "__pycache__",
    "build",  # Will be recreated
]

# Scripts directory cleanup - keep only important ones
SCRIPTS_TO_REMOVE = [
    "animate_a_cube_to_rise_and_spin_in_a_3d_scene_using_unreal_engine_technical_specs__create_a_3d_scene_in_unreal_engine_add_a_cube_object_to_the_scene_animate_the_cube_to_rise_and_spin_using_the_unreal_engine_animation_tool.py",
    "create_a_3d_animation_of_a_cube_rising_and_spinning_in_unreal_engine_technical_specs__use_unreal_engine_for_animation_create_a_rising_motion_for_the_cube_spin_the_cube_continuously.py",
    "create_character.py",
    "develop_a_simple_calculator_application_using_c.py",
    "develop_a_simple_weather_checking_application.py",
    "generate_a_3d_scene_with_a_cube_that_rises_and_spins_once_in_unreal_engine_technical_specs__create_a_3d_cube_model_write_a_script_for_the_cube_to_rise_and_spin_once_import_the_3d_cube_into_unreal_engine_set_up_the_scene_with_the_risen_and_spinning_cube.py",
    "process_goal1_ill_help_you_create_a_house_project_in_unreal_engine_using_your_blender_designs_lets_begin_by_setting_up_a_new_unreal_projectcreate_a_new_unreal_engine_project_and_import_blender_designs_for_a_house_project.py",
    "test_script.py",
    "cleanup_project.py",
]

def cleanup():
    """Remove unnecessary files and directories"""
    root = Path(".")
    removed_count = 0
    
    print("🧹 Starting VrindaAI Project Cleanup...\n")
    
    # Remove individual files
    for file_path in TO_REMOVE:
        full_path = root / file_path
        if full_path.exists():
            if full_path.is_dir():
                try:
                    shutil.rmtree(full_path)
                    print(f"✓ Removed directory: {file_path}")
                    removed_count += 1
                except Exception as e:
                    print(f"✗ Failed to remove directory {file_path}: {e}")
            else:
                try:
                    full_path.unlink()
                    print(f"✓ Removed file: {file_path}")
                    removed_count += 1
                except Exception as e:
                    print(f"✗ Failed to remove file {file_path}: {e}")
    
    # Remove scripts
    scripts_dir = root / "scripts"
    if scripts_dir.exists():
        for script in SCRIPTS_TO_REMOVE:
            script_path = scripts_dir / script
            if script_path.exists():
                try:
                    script_path.unlink()
                    print(f"✓ Removed script: scripts/{script}")
                    removed_count += 1
                except Exception as e:
                    print(f"✗ Failed to remove script {script}: {e}")
    
    # Remove old job history files
    jobs_dir = root / "jobs"
    if jobs_dir.exists():
        for job_file in jobs_dir.glob("history_*.json"):
            try:
                job_file.unlink()
                print(f"✓ Removed history file: {job_file.name}")
                removed_count += 1
            except Exception as e:
                print(f"✗ Failed to remove {job_file}: {e}")
    
    print(f"\n✅ Cleanup complete! Removed {removed_count} files/directories")
    print("\n📁 Next steps:")
    print("1. Create new directory structure in src/")
    print("2. Migrate core functionality to new modules")
    print("3. Update requirements.txt")
    print("4. Create new documentation")

if __name__ == "__main__":
    cleanup()
