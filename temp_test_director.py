import json
from src.engines.unreal_engine import create_unreal_engine
from pathlib import Path

with open('config/settings.json','r') as f:
    cfg = json.load(f)

ue_path = cfg.get('paths',{}).get('unreal')
ue_root = None
if ue_path and ue_path.endswith('UnrealEditor-Cmd.exe'):
    ue_root = ue_path.replace('Engine/Binaries/Win64/UnrealEditor-Cmd.exe','')

ue = create_unreal_engine(ue_root)
proj_dir = Path(r'C:/Users/Mahantesh/Documents/Unreal Projects/VrindaProj_manual_test')
uproject = proj_dir / (proj_dir.name + '.uproject')
print('Using uproject:', uproject)
res = ue.create_cinematic_sequence('MainSequence', [], project_path=str(uproject))
print(res)
