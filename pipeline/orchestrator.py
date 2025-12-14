"""
Orchestrator for autonomous content generation.
Accepts text, prompt, scene or JSON job and dispatches to engine modules (Blender/Unreal/DaVinci)
This is a high-level scaffold: actual engine implementations should be placed in `pipeline/engines/`.
"""
import argparse
import json
import os
from pathlib import Path
from datetime import datetime

from pipeline.llm_adapter import LLMAdapter

WORKFLOWS_DIR = Path("workflows")
WORKFLOWS_DIR.mkdir(exist_ok=True)


def create_blender_job(job):
    # In a full implementation this would create a Blender Python script and job manifest
    name = job.get("name", "blender_job")
    timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
    out = WORKFLOWS_DIR / f"{name}_{timestamp}.json"
    job["engine"] = "blender"
    job["status"] = "generated"
    with open(out, "w", encoding="utf-8") as f:
        json.dump(job, f, indent=2)
    print(f"Generated Blender job manifest: {out}")
    return out


def create_unreal_job(job):
    name = job.get("name", "unreal_job")
    timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
    out = WORKFLOWS_DIR / f"{name}_{timestamp}.json"
    job["engine"] = "unreal"
    job["status"] = "generated"
    with open(out, "w", encoding="utf-8") as f:
        json.dump(job, f, indent=2)
    print(f"Generated Unreal job manifest: {out}")
    return out


def create_davinci_job(job):
    name = job.get("name", "davinci_job")
    timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
    out = WORKFLOWS_DIR / f"{name}_{timestamp}.json"
    job["engine"] = "davinci"
    job["status"] = "generated"
    with open(out, "w", encoding="utf-8") as f:
        json.dump(job, f, indent=2)
    print(f"Generated DaVinci job manifest: {out}")
    return out


def load_input(path=None, text=None):
    if path:
        p = Path(path)
        if not p.exists():
            raise FileNotFoundError(p)
        if p.suffix.lower() == ".json":
            return json.loads(p.read_text(encoding="utf-8"))
        else:
            # treat as plain text file
            return {"description": p.read_text(encoding="utf-8")}
    if text:
        # Try to parse as JSON if possible
        try:
            return json.loads(text)
        except Exception:
            return {"description": text}
    raise ValueError("No input provided")


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--input", "-i", help="Path to JSON or text file describing job")
    parser.add_argument("--text", "-t", help="Direct text description or JSON string")
    parser.add_argument("--engine", "-e", choices=["blender", "unreal", "davinci", "auto"], default="auto")
    parser.add_argument("--name", help="Optional job name")
    args = parser.parse_args()

    job = load_input(args.input, args.text)
    if args.name:
        job["name"] = args.name

    # LLM step - expand/translate description into a job manifest / script plan
    llm = LLMAdapter()
    plan = llm.plan_from_description(job.get("description", ""))
    # merge plan into job
    job.setdefault("plan", {}).update(plan)

    chosen = args.engine
    if chosen == "auto":
        # simple heuristic: look for keywords
        desc = job.get("description", "").lower()
        if any(k in desc for k in ("game", "unreal", "play")):
            chosen = "unreal"
        elif any(k in desc for k in ("grade", "color", "davinci", "composite")):
            chosen = "davinci"
        else:
            chosen = "blender"

    job["requested_engine"] = chosen

    if chosen == "blender":
        create_blender_job(job)
    elif chosen == "unreal":
        create_unreal_job(job)
    elif chosen == "davinci":
        create_davinci_job(job)

    print("Orchestration complete. Generated job manifest(s) in `workflows/`.")


if __name__ == "__main__":
    main()
