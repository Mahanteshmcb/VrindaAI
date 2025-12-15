from src.engines.unreal_engine import create_unreal_engine
import json

# load config path
with open('config/settings.json','r') as f:
    cfg = json.load(f)

ue_path = cfg.get('paths',{}).get('unreal')
if ue_path and ue_path.endswith('UnrealEditor-Cmd.exe'):
    # pass the Engine root
    ue_root = ue_path.replace('Engine/Binaries/Win64/UnrealEditor-Cmd.exe','').replace('Engine\\Binaries\\Win64\\UnrealEditor-Cmd.exe','')
else:
    ue_root = None

ue = create_unreal_engine(ue_root)
res = ue.create_project('VrindaProj_manual_test', project_type='cinematic', target_dir=r'C:\Users\Mahantesh\Documents\Unreal Projects')
print(json.dumps(res, indent=2))
