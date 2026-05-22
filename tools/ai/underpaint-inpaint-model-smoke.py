#!/usr/bin/env python3
"""Run the same tiny Underpaint inpaint job across one or more model ids."""

from __future__ import annotations

import argparse
import json
import os
import subprocess
import sys
import tempfile
from pathlib import Path
from typing import Any

from PIL import Image, ImageDraw


SCRIPT_DIR = Path(__file__).resolve().parent
REPO_ROOT = SCRIPT_DIR.parent.parent
WORKER = SCRIPT_DIR / "underpaint-diffusers-worker.py"


def write_test_assets(job_dir: Path) -> tuple[Path, Path]:
    source = Image.new("RGB", (512, 512), (53, 63, 72))
    draw = ImageDraw.Draw(source)
    draw.rectangle((0, 0, 511, 250), fill=(50, 66, 92))
    draw.rectangle((0, 250, 511, 511), fill=(92, 75, 60))
    draw.ellipse((122, 112, 390, 390), fill=(215, 154, 92), outline=(250, 220, 175), width=6)
    draw.ellipse((172, 162, 340, 340), fill=(238, 190, 126), outline=(110, 78, 50), width=4)
    draw.rectangle((202, 188, 314, 316), fill=(22, 24, 28))
    draw.line((0, 250, 511, 250), fill=(130, 102, 75), width=4)

    mask = Image.new("L", source.size, 0)
    ImageDraw.Draw(mask).rectangle((202, 188, 314, 316), fill=255)

    source_path = job_dir / "source.png"
    mask_path = job_dir / "mask.png"
    source.save(source_path)
    mask.save(mask_path)
    return source_path, mask_path


def request_for_model(
    job_dir: Path,
    model_id: str,
    source_path: Path,
    mask_path: Path,
    args: argparse.Namespace,
) -> dict[str, Any]:
    return {
        "schema": "underpaint.ai-job.v1",
        "id": f"smoke-{model_id}",
        "operation": "inpaint",
        "inputs": [
            {"role": "source-image", "path": str(source_path), "mimeType": "image/png"},
            {"role": "mask", "path": str(mask_path), "mimeType": "image/png"},
        ],
        "parameters": {
            "modelId": model_id,
            "prompt": args.prompt,
            "negativePrompt": args.negative_prompt,
            "seed": args.seed,
            "cfg": args.cfg,
            "denoise": args.denoise,
            "steps": args.steps,
            "candidateCount": 1,
            "edgeFeatherPx": args.edge_feather,
            "previewEverySteps": max(1, args.steps // 2),
            "previewMaxEdge": 128,
        },
        "preferences": {
            "maxRenderEdge": args.render_edge,
            "safe4070Mode": True,
            "vaeTiling": True,
        },
        "source": {"documentName": "underpaint-smoke"},
        "provenance": {"createdBy": "underpaint-inpaint-model-smoke"},
    }


def run_model(model_id: str, args: argparse.Namespace) -> dict[str, Any]:
    job_dir = Path(tempfile.mkdtemp(prefix=f"underpaint-smoke-{model_id}-"))
    source_path, mask_path = write_test_assets(job_dir)
    request = request_for_model(job_dir, model_id, source_path, mask_path, args)
    request_path = job_dir / "request.json"
    response_path = job_dir / "response.json"
    request_path.write_text(json.dumps(request, indent=2), encoding="utf-8")

    env = os.environ.copy()
    env.setdefault("UNDERPAINT_AI_DEBUG", "1")
    if args.cpu_offload:
        env["UNDERPAINT_AI_CPU_OFFLOAD"] = "1"

    completed = subprocess.run(
        [
            sys.executable,
            str(WORKER),
            str(request_path),
            str(response_path),
            str(job_dir),
        ],
        cwd=REPO_ROOT,
        env=env,
        text=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        check=False,
    )

    response: dict[str, Any] = {}
    if response_path.is_file():
        response = json.loads(response_path.read_text(encoding="utf-8"))

    return {
        "modelId": model_id,
        "jobDir": str(job_dir),
        "returnCode": completed.returncode,
        "stdout": completed.stdout.strip().splitlines()[-10:],
        "stderr": completed.stderr.strip().splitlines()[-10:],
        "status": response.get("status", "missing-response"),
        "message": response.get("message", ""),
        "candidate": (response.get("candidates") or [{}])[0].get("imagePath"),
        "diagnostics": response.get("diagnostics", {}),
    }


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--model-id",
        action="append",
        dest="model_ids",
        default=[],
        help="Registry model id to test. May be passed more than once.",
    )
    parser.add_argument("--steps", type=int, default=8)
    parser.add_argument("--render-edge", type=int, default=512)
    parser.add_argument("--seed", type=int, default=24680)
    parser.add_argument("--cfg", type=float, default=5.5)
    parser.add_argument("--denoise", type=float, default=0.78)
    parser.add_argument("--edge-feather", type=int, default=8)
    parser.add_argument("--cpu-offload", action="store_true")
    parser.add_argument(
        "--prompt",
        default=(
            "warm glowing brass lantern with glass lens, realistic photographic "
            "texture, matched perspective, soft orange light"
        ),
    )
    parser.add_argument(
        "--negative-prompt",
        default="text, watermark, frame, border, black square, flat patch",
    )
    args = parser.parse_args()

    model_ids = args.model_ids or ["realvisxl-v4-inpaint-diffusers"]
    results = [run_model(model_id, args) for model_id in model_ids]
    print(json.dumps({"schema": "underpaint.inpaint-smoke.v1", "results": results}, indent=2))
    return 0 if all(result["status"] == "succeeded" for result in results) else 1


if __name__ == "__main__":
    raise SystemExit(main())
