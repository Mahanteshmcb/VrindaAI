"""
VrindaAI - Asset Manager
Handles the 'Direct-to-Project' asset pipeline.
Scans the Epic Vault and copies specific assets into the active Unreal Project.
"""

import os
import shutil
import logging
import json
from pathlib import Path
from typing import List, Optional, Dict

logger = logging.getLogger(__name__)

class AssetManager:
    def __init__(self, config: Dict):
        self.config = config
        self.vault_path = Path(self.config["paths"]["vault_cache"])
        self.logger = logging.getLogger(__name__)

        if not self.vault_path.exists():
            self.logger.warning(f"Vault Cache path not found: {self.vault_path}")

    def scan_vault(self) -> List[str]:
        """Returns a list of available asset packs in the Vault."""
        if not self.vault_path.exists():
            return []
        
        # Vault structure is usually folders named by Asset ID or Title
        return [d.name for d in self.vault_path.iterdir() if d.is_dir()]

    def ingest_asset_to_project(self, asset_folder_name: str, project_content_path: str) -> bool:
        """
        Copies an asset pack from Vault Cache to the Project's Content folder.
        
        Args:
            asset_folder_name: The folder name in VaultCache (e.g., 'VPTemple...')
            project_content_path: The absolute path to your Unreal project's Content folder
        """
        source_path = self.vault_path / asset_folder_name
        dest_path = Path(project_content_path)

        if not source_path.exists():
            self.logger.error(f"Asset not found in Vault: {source_path}")
            return False

        # Check if it's a 'data' folder structure (common in Vault)
        # Often Vault folders have a 'data' subfolder containing the actual 'Content'
        possible_content = source_path / "data" / "Content"
        if possible_content.exists():
            source_path = possible_content
        
        self.logger.info(f"Ingesting asset: {asset_folder_name}...")
        
        try:
            # We copy contents, merging folders if they exist
            self._copy_tree(source_path, dest_path)
            self.logger.info(f"Successfully ingested {asset_folder_name} to {dest_path}")
            return True
        except Exception as e:
            self.logger.error(f"Failed to ingest asset: {e}")
            return False

    def _copy_tree(self, source: Path, dest: Path):
        """Recursive copy that merges directories."""
        if not dest.exists():
            dest.mkdir(parents=True, exist_ok=True)
            
        for item in source.iterdir():
            dest_item = dest / item.name
            if item.is_dir():
                self._copy_tree(item, dest_item)
            else:
                if not dest_item.exists(): # Don't overwrite existing to save time
                    shutil.copy2(item, dest_item)

def create_asset_manager(config: Dict) -> AssetManager:
    return AssetManager(config)