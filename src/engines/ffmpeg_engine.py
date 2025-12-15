"""
VrindaAI - FFmpeg Engine Wrapper
Replaces DaVinci Resolve for lightweight, headless video assembly.
"""

import subprocess
import logging
import os
from typing import Dict, Any, Optional, List
from pathlib import Path

logger = logging.getLogger(__name__)

class FFmpegEngine:
    """
    Automates video stitching, audio mixing, and format conversion using FFmpeg.
    """
    
    def __init__(self, ffmpeg_path: Optional[str] = None):
        self.logger = logging.getLogger(__name__)
        self.ffmpeg_path = ffmpeg_path or self._find_ffmpeg()
        if not self.ffmpeg_path:
            raise FileNotFoundError("FFmpeg executable not found. Please install FFmpeg and add to PATH.")

    def _find_ffmpeg(self) -> Optional[str]:
        import shutil
        return shutil.which("ffmpeg")

    def create_video_from_sequence(
        self,
        image_sequence_pattern: str, # e.g. "render_%04d.exr" or "render_*.jpg"
        output_file: str,
        framerate: int = 24,
        audio_file: Optional[str] = None,
        quality: str = "high"
    ) -> Dict[str, Any]:
        """
        Stitch an image sequence (EXR/PNG/JPG) into a video file.
        Supports both numbered sequences and wildcard patterns.
        """
        self.logger.info(f"Stitching sequence: {image_sequence_pattern} -> {output_file}")
        
        # Ensure output directory exists
        Path(output_file).parent.mkdir(parents=True, exist_ok=True)

        # Handle wildcard patterns (convert to FFmpeg format)
        if "*" in image_sequence_pattern:
            # Convert wildcard to FFmpeg %d format
            import glob
            matches = sorted(glob.glob(image_sequence_pattern))
            if matches:
                # Infer the pattern from matched files
                first_file = Path(matches[0])
                parent = first_file.parent
                stem = first_file.stem
                ext = first_file.suffix
                
                # Try to find the numeric pattern
                import re
                match = re.search(r'(\d+)$', stem)
                if match:
                    num_width = len(match.group(1))
                    base_stem = stem[:match.start()]
                    pattern = str(parent / f"{base_stem}%0{num_width}d{ext}").replace("\\", "/")
                    start_num = int(match.group(1))
                else:
                    pattern = image_sequence_pattern.replace("\\", "/")
                    start_num = 1
            else:
                self.logger.warning(f"No files match pattern: {image_sequence_pattern}")
                return {"status": "failed", "error": f"No files found matching {image_sequence_pattern}"}
        else:
            pattern = image_sequence_pattern.replace("\\", "/")
            start_num = 1

        # Quality settings
        if quality == "high":
            crf = "18"  # Visually lossless
            preset = "slow"
        elif quality == "medium":
            crf = "23"
            preset = "medium"
        else:  # low
            crf = "28"
            preset = "fast"

        cmd = [
            self.ffmpeg_path,
            "-y",  # Overwrite output
            "-framerate", str(framerate),
            "-start_number", str(start_num),
            "-i", pattern
        ]

        if audio_file and os.path.exists(audio_file):
            cmd.extend(["-i", audio_file, "-c:a", "aac", "-shortest"])

        # Video encoding settings
        cmd.extend([
            "-c:v", "libx264",
            "-preset", preset,
            "-crf", crf,
            "-pix_fmt", "yuv420p",
            output_file
        ])

        return self._execute_ffmpeg(cmd)

    def concat_clips(
        self,
        clip_paths: List[str],
        output_file: str
    ) -> Dict[str, Any]:
        """
        Concatenate multiple video clips into a single movie (The "Editor" role).
        """
        self.logger.info(f"Concatenating {len(clip_paths)} clips...")
        
        # Create a temporary file listing the inputs
        list_file_path = Path(output_file).parent / "concat_list.txt"
        with open(list_file_path, "w") as f:
            for path in clip_paths:
                # FFmpeg requires absolute paths in list files, safe quoting
                abs_path = Path(path).resolve()
                f.write(f"file '{str(abs_path).replace(os.sep, '/')}'\n")

        cmd = [
            self.ffmpeg_path,
            "-y",
            "-f", "concat",
            "-safe", "0",
            "-i", str(list_file_path),
            "-c", "copy", # Stream copy (very fast, no re-encoding)
            output_file
        ]

        result = self._execute_ffmpeg(cmd)
        
        # Cleanup
        if list_file_path.exists():
            list_file_path.unlink()
            
        return result

    def apply_background_music(self, video_file: str, music_file: str, output_file: str) -> Dict[str, Any]:
        """Mix background music into a video."""
        cmd = [
            self.ffmpeg_path,
            "-y",
            "-i", video_file,
            "-i", music_file,
            "-map", "0:v",
            "-map", "1:a",
            "-c:v", "copy",
            "-c:a", "aac",
            "-shortest", # Cut audio to video length
            output_file
        ]
        return self._execute_ffmpeg(cmd)

    def _execute_ffmpeg(self, cmd: List[str]) -> Dict[str, Any]:
        try:
            self.logger.debug(f"Running FFmpeg: {' '.join(cmd)}")
            result = subprocess.run(
                cmd,
                capture_output=True,
                text=True,
                check=False
            )
            
            if result.returncode == 0:
                return {"status": "success", "output": result.stdout}
            else:
                self.logger.error(f"FFmpeg Error: {result.stderr}")
                return {"status": "failed", "error": result.stderr}
        except Exception as e:
            return {"status": "failed", "error": str(e)}

def create_ffmpeg_engine() -> FFmpegEngine:
    return FFmpegEngine()