"""
VrindaAI - Asset Manager
Handles the 'Local-First' asset pipeline and Unified Asset Manifest integration.
C: Drive Optimized Version for Phase 2 Verification.
"""

import os
import shutil
import logging
import json
from pathlib import Path
from datetime import datetime
from typing import List, Optional, Dict

logger = logging.getLogger(__name__)

class AssetManager:
    def __init__(self, config: Dict):
        """
        Initialize the Asset Manager with studio library paths.
        Updated to use C:/Vrinda_Assets/Temp/ as the primary local library.
        """
        self.config = config
        
        # SYSTEM FIX: Default to C: drive path provided by user
        default_library = "C:/Vrinda_Assets/Temp/"
        self.library_path = Path(self.config.get("paths", {}).get("library_path", default_library))
        
        # Vault cache path (typically Epic Games default)
        self.vault_path = Path(self.config.get("paths", {}).get("vault_cache", "C:/Program Files/Epic Games/Launcher/VaultCache"))
        self.logger = logging.getLogger(__name__)

        if not self.library_path.exists():
            self.logger.warning(f"⚠️ Standard Library path not found: {self.library_path}. Creating now...")
            self.library_path.mkdir(parents=True, exist_ok=True)

    def find_local_asset(self, query: str) -> Optional[Dict[str, str]]:
        """
        Phase 2: Local-First Search Logic.
        Scans C:/Vrinda_Assets/Temp/ for assets matching the query.
        """
        self.logger.info(f"🔍 Searching local library (C: Drive) for: {query}")
        
        # Search recursively through the library
        for file in self.library_path.rglob("*"):
            # Check for partial match in filename (e.g. 'SciFi_Crate' matches 'SciFi_Crate.fbx')
            if query.lower() in file.name.lower() and file.is_file():
                self.logger.info(f"✅ Found local match: {file.name}")
                return {
                    "name": file.stem,
                    "path": str(file.absolute()),
                    "type": self._determine_asset_type(file.suffix)
                }
        return None

    def ingest_to_production(self, asset_info: Dict, project_path: str) -> bool:
        """
        Copies found asset to project/Raw_Downloads and updates manifest.
        """
        source = Path(asset_info["path"])
        dest_folder = Path(project_path) / "Raw_Downloads"
        dest_folder.mkdir(parents=True, exist_ok=True)
        
        dest_path = dest_folder / source.name
        
        try:
            # Only copy if it doesn't already exist in the project
            if not dest_path.exists():
                shutil.copy2(source, dest_path)
            
            # Phase 1 Requirement: Unified Asset Manifest Registration
            self._register_in_manifest(
                project_path, 
                asset_info["name"], 
                asset_info["type"], 
                f"Raw_Downloads/{source.name}"
            )
            return True
        except Exception as e:
            self.logger.error(f"❌ Ingestion failed: {e}")
            return False

    def _register_in_manifest(self, project_path: str, name: str, asset_type: str, rel_path: str):
        """Updates project_assets.json with the new asset and description."""
        manifest_path = Path(project_path) / "project_assets.json"
        
        # Load existing manifest or create new
        if manifest_path.exists():
            with open(manifest_path, 'r') as f:
                manifest = json.load(f)
        else:
            manifest = {"assets": [], "last_asset_id": 0}

        new_id_num = manifest["last_asset_id"] + 1
        asset_id = f"{asset_type.upper()}_{str(new_id_num).zfill(3)}"
        
        new_entry = {
            "id": asset_id,
            "type": asset_type,
            "name": name,
            "description": f"Phase 2: Auto-ingested from {self.library_path}",
            "path": rel_path,
            "timestamp": datetime.now().isoformat()
        }

        manifest["assets"].append(new_entry)
        manifest["last_asset_id"] = new_id_num
        manifest["last_updated"] = datetime.now().isoformat()

        with open(manifest_path, 'w') as f:
            json.dump(manifest, f, indent=2)
        self.logger.info(f"📁 Manifest Updated: Registered {asset_id}")

    def _determine_asset_type(self, suffix: str) -> str:
        suffix = suffix.lower()
        if suffix in ['.fbx', '.obj', '.blend']: return "mesh"
        if suffix in ['.wav', '.mp3', '.ogg']: return "audio"
        if suffix in ['.exr', '.jpg', '.png']: return "render"
        return "generic"

def create_asset_manager(config: Dict) -> AssetManager:
    return AssetManager(config)