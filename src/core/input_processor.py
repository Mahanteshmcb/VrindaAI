"""
VrindaAI - Unified Input Processor
Handles ANY input format: text prompts, JSON, YAML, XML, natural language descriptions
Converts all inputs to normalized task specification for LLM processing
"""

import json
import re
import yaml
import xml.etree.ElementTree as ET
from enum import Enum
from typing import Union, Dict, List, Any, Optional, Tuple, cast
from dataclasses import dataclass, asdict
from pathlib import Path
import logging
import mimetypes

logger = logging.getLogger(__name__)


class InputType(Enum):
    """Types of input supported"""
    TEXT_PROMPT = "text_prompt"
    JSON_CONFIG = "json_config"
    SCENE_DESCRIPTION = "scene_description"
    FILE_PATH = "file_path"


class EngineType(Enum):
    """Supported 3D engines"""
    BLENDER = "blender"
    UNREAL_ENGINE = "unreal"
    DAVINCI_RESOLVE = "davinci"


class OutputType(Enum):
    """Types of output to generate"""
    CINEMATIC_VIDEO = "cinematic_video"
    GAME = "game"
    SCENE_3D = "3d_scene"  # Renamed from 3D_SCENE to SCENE_3D
    ANIMATION = "animation"
    RENDERED_SEQUENCE = "rendered_sequence"


@dataclass
class TextPrompt:
    """Simple text-based prompt"""
    description: str
    style: Optional[str] = "cinematic"
    duration: Optional[int] = 30
    resolution: Optional[str] = "1080p"
    engine: Optional[EngineType] = EngineType.BLENDER
    output_type: Optional[OutputType] = OutputType.CINEMATIC_VIDEO
    assets: Optional[List[str]] = None
    templates: Optional[List[str]] = None
    custom_params: Optional[Dict[str, Any]] = None


@dataclass
class JsonConfig:
    """JSON-based configuration"""
    type: str  # "cinematic", "game", "scene", "animation"
    engine: str  # "blender", "unreal", "davinci"
    description: str
    style: str = "realistic"
    duration: int = 30
    resolution: str = "1080p"
    fps: int = 24
    assets: Optional[List[str]] = None
    templates: Optional[List[str]] = None
    parameters: Optional[Dict[str, Any]] = None
    output_path: Optional[str] = None
    quality: str = "high"  # low, medium, high, ultra


@dataclass
class SceneObject:
    """3D scene object definition"""
    name: str
    type: str  # "model", "camera", "light", "particle", "text"
    properties: Dict[str, Any]
    position: Optional[List[float]] = None  # [x, y, z]
    rotation: Optional[List[float]] = None  # [x, y, z] in degrees
    scale: Optional[List[float]] = None      # [x, y, z]
    material: Optional[str] = None
    animation: Optional[Dict[str, Any]] = None


@dataclass
class SceneDescription:
    """Complete scene description"""
    name: str
    engine: str
    environment: str
    objects: List[SceneObject]
    camera: Dict[str, Any]
    lighting: Dict[str, Any]
    rendering: Optional[Dict[str, Any]] = None
    effects: Optional[List[Dict[str, Any]]] = None
    audio: Optional[Dict[str, Any]] = None


class InputProcessor:
    """
    Unified input processor for VrindaAI
    Converts text, JSON, or scene descriptions into engine-agnostic task specifications
    """
    
    def __init__(self, llm_connector=None):
        """
        Initialize input processor
        
        Args:
            llm_connector: Optional LLM service for intelligent prompt analysis
        """
        self.llm_connector = llm_connector
        self.logger = logging.getLogger(__name__)
    
    def process_input(
        self,
        input_data: Union[str, Dict, Path],
        input_type: Optional[InputType] = None
    ) -> Dict[str, Any]:
        """
        Process any input type and return normalized task specification
        
        Args:
            input_data: The input (text, dict, or file path)
            input_type: Optional type hint (auto-detected if not provided)
            
        Returns:
            Normalized task specification dictionary
        """
        # Auto-detect input type if not provided
        if input_type is None:
            input_type = self._detect_input_type(input_data)
        
        self.logger.info(f"Processing {input_type.value} input")
        
        # Route to appropriate processor
        if input_type == InputType.TEXT_PROMPT:
            return self._process_text_prompt(str(input_data))
            
        elif input_type == InputType.JSON_CONFIG:
            # Ensure it is a Dict before passing
            config_data = json.loads(str(input_data)) if isinstance(input_data, str) else input_data
            return self._process_json_config(cast(Dict[str, Any], config_data))
            
        elif input_type == InputType.SCENE_DESCRIPTION:
            # Ensure it is a Dict before passing
            scene_data = json.loads(str(input_data)) if isinstance(input_data, str) else input_data
            return self._process_scene_description(cast(Dict[str, Any], scene_data))
            
        elif input_type == InputType.FILE_PATH:
            return self._process_file_path(Path(str(input_data)))
            
        else:
            raise ValueError(f"Unknown input type: {input_type}")
    
    def _detect_input_type(self, input_data: Union[str, Dict, Path]) -> InputType:
        """Detect the type of input provided"""
        
        if isinstance(input_data, Path) or (isinstance(input_data, str) and Path(input_data).exists()):
            return InputType.FILE_PATH
        
        if isinstance(input_data, dict):
            if "objects" in input_data and "environment" in input_data:
                return InputType.SCENE_DESCRIPTION
            if "type" in input_data and "engine" in input_data:
                return InputType.JSON_CONFIG
            return InputType.JSON_CONFIG
        
        if isinstance(input_data, str):
            try:
                json.loads(input_data)
                return InputType.JSON_CONFIG
            except json.JSONDecodeError:
                return InputType.TEXT_PROMPT
        
        return InputType.TEXT_PROMPT
    
    def _process_text_prompt(self, prompt: str) -> Dict[str, Any]:
        """
        Process natural language text prompt
        Uses LLM to extract intent, parameters, and engine selection
        """
        self.logger.info(f"Processing text prompt: {prompt[:100]}...")
        
        # Extract basic parameters from text
        engine = self._extract_engine_from_text(prompt)
        output_type = self._extract_output_type_from_text(prompt)
        style = self._extract_style_from_text(prompt)
        duration = self._extract_duration_from_text(prompt)
        
        task_spec = {
            "input_type": "text_prompt",
            "description": prompt,
            "engine": engine.value,
            "output_type": output_type.value,
            "style": style,
            "duration": duration,
            "resolution": "1080p",
            "fps": 24,
            "quality": "high",
            "parameters": {
                "auto_generated": True,
                "confidence": 0.8
            }
        }
        
        # If LLM connector available, use it for deeper analysis
        if self.llm_connector:
            enhanced = self._enhance_with_llm(prompt, task_spec)
            task_spec.update(enhanced)
        
        return task_spec
    
    def _process_json_config(self, config: Union[str, Dict]) -> Dict[str, Any]:
        """Process JSON configuration"""
        self.logger.info("Processing JSON configuration")
        
        config_dict: Dict[str, Any] = json.loads(config) if isinstance(config, str) else config
        
        # Validate required fields
        required = ["type", "engine", "description"]
        for field in required:
            if field not in config_dict:
                raise ValueError(f"Missing required field: {field}")
        
        # Normalize the config
        normalized = {
            "input_type": "json_config",
            "type": config_dict.get("type"),
            "engine": config_dict.get("engine"),
            "description": config_dict.get("description"),
            "style": config_dict.get("style", "realistic"),
            "duration": config_dict.get("duration", 30),
            "resolution": config_dict.get("resolution", "1080p"),
            "fps": config_dict.get("fps", 24),
            "quality": config_dict.get("quality", "high"),
            "assets": config_dict.get("assets", []),
            "templates": config_dict.get("templates", []),
            "parameters": config_dict.get("parameters", {}),
            "output_path": config_dict.get("output_path"),
        }
        
        return normalized
    
    def _process_scene_description(self, scene: Union[str, Dict]) -> Dict[str, Any]:
        """Process detailed scene description"""
        self.logger.info("Processing scene description")
        
        scene_dict: Dict[str, Any] = json.loads(scene) if isinstance(scene, str) else scene
        
        # Validate required fields
        required = ["name", "engine", "objects", "environment"]
        for field in required:
            if field not in scene_dict:
                raise ValueError(f"Missing required field: {field}")
        
        # Convert to normalized spec
        normalized = {
            "input_type": "scene_description",
            "type": "3d_scene",
            "name": scene_dict.get("name"),
            "engine": scene_dict.get("engine"),
            "description": f"Scene: {scene_dict.get('name')}",
            "environment": scene_dict.get("environment"),
            "objects": scene_dict.get("objects", []),
            "camera": scene_dict.get("camera", {}),
            "lighting": scene_dict.get("lighting", {}),
            "rendering": scene_dict.get("rendering", {}),
            "effects": scene_dict.get("effects", []),
            "audio": scene_dict.get("audio"),
            "parameters": scene_dict.get("parameters", {}),
        }
        
        return normalized
    
    def _process_file_path(self, file_path: Union[str, Path]) -> Dict[str, Any]:
        """Process input from file"""
        self.logger.info(f"Processing file: {file_path}")
        
        file_path = Path(file_path)
        if not file_path.exists():
            raise FileNotFoundError(f"Input file not found: {file_path}")
        
        with open(file_path, 'r', encoding='utf-8') as f:
            content = f.read()
        
        # Re-process the file content
        return self.process_input(content)
    
    def _extract_engine_from_text(self, text: str) -> EngineType:
        """Extract engine preference from text"""
        text_lower = text.lower()
        
        if "unreal" in text_lower or "ue5" in text_lower or "game" in text_lower:
            return EngineType.UNREAL_ENGINE
        elif "davinci" in text_lower or "resolve" in text_lower or "edit" in text_lower:
            return EngineType.DAVINCI_RESOLVE
        else:
            return EngineType.BLENDER  # Default to Blender for 3D content
    
    def _extract_output_type_from_text(self, text: str) -> OutputType:
        """Extract desired output type"""
        text_lower = text.lower()
        
        if "game" in text_lower:
            return OutputType.GAME
        elif "animation" in text_lower:
            return OutputType.ANIMATION
        elif "scene" in text_lower:
            return OutputType.SCENE_3D  # FIX: Updated to valid Enum
        elif "video" in text_lower or "movie" in text_lower or "cinematic" in text_lower:
            return OutputType.CINEMATIC_VIDEO
        else:
            return OutputType.CINEMATIC_VIDEO  # Default
    
    def _extract_style_from_text(self, text: str) -> str:
        """Extract style/aesthetic preference"""
        text_lower = text.lower()
        
        styles = ["cinematic", "realistic", "stylized", "cartoon", "anime", "scifi", "fantasy", "horror"]
        for style in styles:
            if style in text_lower:
                return style
        
        return "cinematic"  # Default
    
    def _extract_duration_from_text(self, text: str) -> int:
        """Extract duration in seconds from text"""
        # Look for patterns like "30 seconds", "2 minutes", "30s", "2m"
        patterns = [
            r'(\d+)\s*(?:seconds?|sec|s)(?!\w)',
            r'(\d+)\s*(?:minutes?|min|m)(?!\w)',
            r'(\d+)\s*(?:frames?)',
        ]
        
        for pattern in patterns:
            match = re.search(pattern, text, re.IGNORECASE)
            if match:
                duration = int(match.group(1))
                # Convert to seconds if needed
                if "minute" in pattern or pattern.endswith("m)"):
                    duration *= 60
                return max(1, min(duration, 600))  # Cap at 10 minutes
        
        return 30  # Default to 30 seconds
    
    def _enhance_with_llm(self, prompt: str, base_spec: Dict) -> Dict[str, Any]:
        """Enhance task specification using LLM"""
        try:
            # FIX: Check if self.llm_connector is not None explicitly
            if self.llm_connector and hasattr(self.llm_connector, 'analyze_prompt'):
                enhanced = self.llm_connector.analyze_prompt(prompt)
                self.logger.info("LLM analysis completed")
                return enhanced
        except Exception as e:
            self.logger.warning(f"LLM enhancement failed: {e}")
        
        return {}
    
    def validate_task_spec(self, spec: Dict[str, Any]) -> bool:
        """Validate task specification"""
        required_fields = ["engine", "description"]
        
        for field in required_fields:
            if field not in spec:
                self.logger.error(f"Missing required field: {field}")
                return False
        
        # Validate engine value
        valid_engines = [e.value for e in EngineType]
        if spec.get("engine") not in valid_engines:
            self.logger.error(f"Invalid engine: {spec.get('engine')}")
            return False
        
        return True
    
    def generate_job_manifest(self, task_spec: Dict[str, Any]) -> Dict[str, Any]:
        """
        Convert normalized task spec to job manifest for execution
        
        Returns:
            Job manifest dict compatible with JobManifestManager
        """
        engine = task_spec.get("engine")
        
        manifest = {
            "version": "1.0",
            "engine": engine,
            "description": task_spec.get("description"),
            "metadata": {
                "input_type": task_spec.get("input_type"),
                "quality": task_spec.get("quality", "high"),
                "duration": task_spec.get("duration", 30),
            },
            "parameters": task_spec.get("parameters", {}),
        }
        
        # Engine-specific configuration
        if engine == "blender":
            manifest["actions"] = self._generate_blender_actions(task_spec)
        elif engine == "unreal":
            manifest["actions"] = self._generate_unreal_actions(task_spec)
        elif engine == "davinci":
            manifest["actions"] = self._generate_davinci_actions(task_spec)
        
        return manifest
    
    def _generate_blender_actions(self, spec: Dict) -> List[Dict]:
        """Generate Blender-specific actions"""
        return [
            {
                "type": "render",
                "template": spec.get("templates", ["cinematic_master"])[0],
                "style": spec.get("style", "cinematic"),
                "duration": spec.get("duration", 30),
                "resolution": spec.get("resolution", "1080p"),
                "fps": spec.get("fps", 24),
                "samples": 128 if spec.get("quality") == "ultra" else 64,
            }
        ]
    
    def _generate_unreal_actions(self, spec: Dict) -> List[Dict]:
        """Generate Unreal Engine-specific actions"""
        return [
            {
                "type": "create_project",
                "template": spec.get("templates", ["game_starter"])[0],
                "game_type": spec.get("type", "game"),
                "quality": spec.get("quality", "high"),
            },
            {
                "type": "create_scene",
                "description": spec.get("description"),
            }
        ]
    
    def _generate_davinci_actions(self, spec: Dict) -> List[Dict]:
        """Generate DaVinci Resolve-specific actions"""
        return [
            {
                "type": "edit",
                "template": spec.get("templates", ["cinematic_color_profile"])[0],
                "style": spec.get("style", "cinematic"),
                "color_grade": "professional",
            }
        ]


def create_processor(llm_connector=None) -> InputProcessor:
    """Factory function to create input processor"""
    return InputProcessor(llm_connector)