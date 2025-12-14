# VrindaAI Headless Mode - Quick Reference

## Quick Start

```bash
# Navigate to VrindaAI directory
cd C:\Users\Mahantesh\DevelopmentProjects\VrindaAI\VrindaAI

# Run a workflow
VrindaAI.exe --headless --workflow test_blender_simple.json

# Check if it worked
echo %ERRORLEVEL%
# Output: 0 = Success, 1 = Failed
```

## Command Line Usage

### Execute a workflow
```bash
VrindaAI.exe --headless --workflow <workflow.json>
```

### Execute a single job
```bash
VrindaAI.exe --headless --job <job.json>
```

## Available Test Files

| File | Type | Description | Status |
|------|------|-------------|--------|
| `test_blender_simple.json` | Workflow | Simple Blender render test | ✅ Tested |
| `cinematic_pipeline.json` | Workflow | Full movie pipeline | Ready |
| `test_simple_workflow.json` | Workflow | Single-stage render | Ready |
| `jobs/blender_test_default_scene.json` | Job | Blender default scene render | ✅ Tested |
| `jobs/blender_cinematic_job.json` | Job | Cinematic 3D render | Ready |
| `jobs/davinci_composite_job.json` | Job | Video composition | Ready |

## Exit Codes

```
0 = Success - Workflow executed successfully
1 = Failure - Check headless_execution.log for details
```

## Checking Results

### View execution log
```bash
type headless_execution.log
```

### Check for output files
```bash
dir output\
```

### Monitor in real-time (PowerShell)
```powershell
Get-Content headless_execution.log -Wait
```

## Integration Examples

### PowerShell
```powershell
$result = & .\VrindaAI.exe --headless --workflow test_blender_simple.json

if ($LASTEXITCODE -eq 0) {
    Write-Host "✓ Success!"
    Get-ChildItem output\ -Recurse
} else {
    Write-Host "✗ Failed"
    Get-Content headless_execution.log -Tail 20
}
```

### Batch Script
```batch
@echo off
VrindaAI.exe --headless --workflow test_blender_simple.json
if %ERRORLEVEL% EQU 0 (
    echo ✓ Success!
    dir output
) else (
    echo ✗ Failed
    type headless_execution.log
)
```

### Python
```python
import subprocess
import json

# Execute workflow
result = subprocess.run([
    "VrindaAI.exe",
    "--headless",
    "--workflow",
    "test_blender_simple.json"
])

if result.returncode == 0:
    print("✓ Success!")
else:
    print("✗ Failed - check headless_execution.log")
```

## Creating Your Own Workflows

### Minimal Workflow Template
```json
{
  "workflow_id": "MY_WORKFLOW_001",
  "name": "MyWorkflow",
  "jobs": [
    "./jobs/my_job.json"
  ],
  "settings": {
    "parallel_execution": false,
    "continue_on_error": false
  }
}
```

### Minimal Job Template (Blender)
```json
{
  "job_id": "MY_JOB_001",
  "engine": "blender",
  "header": {
    "project_name": "MyProject"
  },
  "output": {
    "path": "./output/result.png"
  }
}
```

## Troubleshooting

### No output files generated
1. Check `headless_execution.log` for errors
2. Verify Python master scripts exist in `Resources/Scripts/`
3. Check that output paths are valid

### Exit code 1 (Failure)
1. Read `headless_execution.log`:
   ```bash
   type headless_execution.log
   ```
2. Common issues:
   - Missing asset files referenced in job
   - Invalid JSON syntax
   - Blender not installed
   - Script path errors

### Blender not found
- Don't worry! VrindaAI auto-detects Blender
- Supported: C:\Program Files\Blender Foundation\Blender 4.x\
- If installed elsewhere, manually add to system PATH

## Monitoring Execution

```powershell
# Show last 20 lines of log
Get-Content headless_execution.log -Tail 20

# Monitor in real-time
Get-Content headless_execution.log -Wait

# Search for errors
Select-String "ERROR" headless_execution.log

# Show last execution time
Get-Item headless_execution.log | Select-Object LastWriteTime
```

## Key Features

✅ **No GUI** - Runs completely in background  
✅ **Logging** - Complete execution log to file  
✅ **Exit Codes** - Proper status codes for scripts  
✅ **Auto-Detection** - Finds Blender automatically  
✅ **Scalable** - Multi-stage workflows supported  
✅ **CI/CD Ready** - Can be used in automation  
✅ **Error Handling** - Detailed error messages  

## Files Created/Modified

- ✅ `VrindaAI.exe` - Main executable (headless capable)
- ✅ `headless_execution.log` - Auto-generated log file
- ✅ `test_blender_simple.json` - Test workflow
- ✅ `cinematic_pipeline.json` - Full pipeline workflow
- ✅ `jobs/*.json` - Job definitions
- ✅ `HEADLESS_EXECUTION_STATUS.md` - Detailed documentation
- ✅ `HEADLESS_OPERATION_SUCCESS.md` - Success report

---

**Last Tested**: 2025-12-14 18:46:12  
**Blender Version**: 4.3  
**Status**: ✅ Operational
