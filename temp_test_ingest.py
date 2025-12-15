import json
from src.core.asset_manager import create_asset_manager

with open('config/settings.json','r') as f:
    cfg = json.load(f)

mgr = create_asset_manager(cfg)
asset = 'VPTemple99388731072fV2'
target = r'C:\Users\Mahantesh\Documents\Unreal Projects\VrindaProj_manual_test\Content'
res = mgr.ingest_asset_to_project(asset, target)
print('Ingest result:', res)
