# Create a test script (test_asset_ingest.py) locally to verify:
from src.core.asset_manager import create_asset_manager
import json

with open("config/settings.json") as f:
    conf = json.load(f)

manager = create_asset_manager(conf)
available = manager.scan_vault()
print("Available Assets:", available)

# Uncomment to test actual copy (WARNING: Copies data!)
# manager.ingest_asset_to_project("VPTemple99388731072fV2", "output/games/TestProject/Content")