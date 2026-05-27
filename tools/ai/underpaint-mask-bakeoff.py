#!/usr/bin/env python3
"""Compare Underpaint mask/decomposition candidates on one image.

The harness is intentionally a developer tool. It writes per-method masks,
cutouts, overlays, contact sheets, and JSON reports so segmentation and matting
changes can be compared before they are wired into the Qt workflow.
"""

from __future__ import annotations

import argparse
import json
import math
import os
import re
import subprocess
import sys
import tempfile
import time
from dataclasses import dataclass
from pathlib import Path
from typing import Any

from PIL import Image, ImageChops, ImageDraw, ImageFilter, ImageFont


SCRIPT_DIR = Path(__file__).resolve().parent
REPO_ROOT = SCRIPT_DIR.parent.parent
WORKER = SCRIPT_DIR / "underpaint-diffusers-worker.py"
DEFAULT_OBJECT_MODEL = Path.home() / ".underpaint/models/detection/yolo11n-seg.pt"
DEFAULT_PERSON_MODEL = (
    Path.home() / ".underpaint/models/detail/adetailer/person_yolov8n-seg.pt"
)
DEFAULT_REMBG_MODEL = Path.home() / ".u2net/u2net.onnx"
DEFAULT_BIREFNET_MODEL = Path.home() / ".underpaint/models/segmentation/birefnet"

DIRECT_METHODS = {"yolo-object", "yolo-person", "rembg-u2net", "birefnet"}
WORKER_METHODS = {
    "underpaint-current",
    "underpaint-sam-grid",
    "underpaint-sam-hq-grid",
    "underpaint-all-priors-grid",
}
FUTURE_METHODS = {
    "sam2",
    "grounded-sam2",
    "sam3",
    "ben2",
}
DEFAULT_METHODS = ["underpaint-current", "yolo-object", "yolo-person", "rembg-u2net"]
KNOWN_METHODS = sorted(DIRECT_METHODS | WORKER_METHODS | FUTURE_METHODS)
COLOR_SWATCHES = [
    (239, 83, 80),
    (255, 202, 40),
    (102, 187, 106),
    (38, 166, 154),
    (66, 165, 245),
    (126, 87, 194),
    (236, 64, 122),
    (255, 112, 67),
    (141, 110, 99),
    (120, 144, 156),
]


@dataclass
class Region:
    id: str
    label: str
    image_path: Path
    mask_path: Path
    metadata: dict[str, Any]


def expand_path(value: str | Path) -> Path:
    return Path(os.path.expandvars(os.path.expanduser(str(value))))


def slugify(value: str) -> str:
    slug = re.sub(r"[^a-z0-9]+", "-", value.strip().lower())
    return slug.strip("-") or "method"


def write_json(path: Path, payload: dict[str, Any]) -> None:
    path.write_text(json.dumps(payload, indent=2, sort_keys=True, default=str), encoding="utf-8")


def load_source(path: Path, max_edge: int) -> Image.Image:
    source = Image.open(path).convert("RGBA")
    if max_edge > 0:
        scale = min(1.0, float(max_edge) / max(source.width, source.height))
        if scale < 1.0:
            size = (max(1, round(source.width * scale)), max(1, round(source.height * scale)))
            source = source.resize(size, Image.Resampling.LANCZOS)
    return source


def checkerboard(size: tuple[int, int], tile: int = 16) -> Image.Image:
    width, height = size
    image = Image.new("RGB", size, (82, 82, 82))
    draw = ImageDraw.Draw(image)
    for y in range(0, height, tile):
        for x in range(0, width, tile):
            color = (116, 116, 116) if ((x // tile) + (y // tile)) % 2 else (66, 66, 66)
            draw.rectangle((x, y, x + tile - 1, y + tile - 1), fill=color)
    return image


def fit_image(image: Image.Image, max_size: tuple[int, int]) -> Image.Image:
    image = image.copy()
    image.thumbnail(max_size, Image.Resampling.LANCZOS)
    return image


def label_text(draw: ImageDraw.ImageDraw, xy: tuple[int, int], text: str) -> None:
    try:
        font = ImageFont.load_default()
    except Exception:
        font = None
    draw.text(xy, text, fill=(235, 235, 235), font=font)


def mask_outline(mask: Image.Image) -> Image.Image:
    hard = mask.convert("L").point(lambda value: 255 if value > 8 else 0)
    return ImageChops.difference(
        hard.filter(ImageFilter.MaxFilter(3)),
        hard.filter(ImageFilter.MinFilter(3)),
    )


def make_overlay(
    source: Image.Image,
    regions: list[Region],
    output_path: Path,
    preview_edge: int,
) -> None:
    if not regions:
        return
    overlay = source.convert("RGBA")
    for index, region in enumerate(regions):
        color = COLOR_SWATCHES[index % len(COLOR_SWATCHES)]
        try:
            mask = Image.open(region.mask_path).convert("L")
        except Exception:
            continue
        if mask.size != source.size:
            mask = mask.resize(source.size, Image.Resampling.NEAREST)
        fill = Image.new("RGBA", source.size, (*color, 0))
        fill.putalpha(mask.point(lambda value: 68 if value > 8 else 0))
        overlay = Image.alpha_composite(overlay, fill)
        edge = mask_outline(mask)
        outline = Image.new("RGBA", source.size, (*color, 255))
        overlay = Image.composite(outline, overlay, edge)

    if preview_edge > 0:
        overlay.thumbnail((preview_edge, preview_edge), Image.Resampling.LANCZOS)
    overlay.convert("RGB").save(output_path)


def make_contact_sheet(
    regions: list[Region],
    output_path: Path,
    preview_edge: int,
    max_tiles: int,
) -> None:
    if not regions:
        return
    tile_w = max(180, min(260, preview_edge // 3 if preview_edge else 220))
    tile_h = tile_w + 34
    shown = regions[:max_tiles]
    columns = max(1, min(5, math.ceil(math.sqrt(len(shown) * 1.5))))
    rows = math.ceil(len(shown) / columns)
    sheet = Image.new("RGB", (columns * tile_w, rows * tile_h), (38, 38, 38))
    draw = ImageDraw.Draw(sheet)

    for index, region in enumerate(shown):
        x = (index % columns) * tile_w
        y = (index // columns) * tile_h
        tile = checkerboard((tile_w, tile_w), 16).convert("RGBA")
        try:
            cutout = Image.open(region.image_path).convert("RGBA")
        except Exception:
            cutout = Image.new("RGBA", (tile_w, tile_w), (0, 0, 0, 0))
        cutout = fit_image(cutout, (tile_w - 12, tile_w - 12))
        paste_x = x + (tile_w - cutout.width) // 2
        paste_y = y + (tile_w - cutout.height) // 2
        tile.alpha_composite(cutout, (paste_x - x, paste_y - y))
        sheet.paste(tile.convert("RGB"), (x, y))
        label = f"{index + 1}. {region.label}"[:42]
        draw.rectangle((x, y + tile_w, x + tile_w, y + tile_h), fill=(48, 48, 48))
        label_text(draw, (x + 6, y + tile_w + 7), label)

    sheet.save(output_path)


def region_summary(region: Region) -> dict[str, Any]:
    payload = {
        "id": region.id,
        "label": region.label,
        "imagePath": str(region.image_path),
        "maskPath": str(region.mask_path),
        "metadata": region.metadata,
    }
    try:
        mask = Image.open(region.mask_path).convert("L")
        bbox = mask.point(lambda value: 255 if value > 8 else 0).getbbox()
        payload["bbox"] = list(bbox) if bbox else None
        payload["maskSize"] = [mask.width, mask.height]
    except Exception as exc:  # noqa: BLE001
        payload["maskError"] = str(exc)
    return payload


def parse_methods(args: argparse.Namespace) -> list[str]:
    methods: list[str] = []
    for entry in args.method:
        methods.extend(part.strip() for part in entry.split(",") if part.strip())
    if args.all:
        methods = sorted(DIRECT_METHODS | WORKER_METHODS)
    if not methods:
        methods = list(DEFAULT_METHODS)
    unknown = sorted(set(methods) - set(KNOWN_METHODS))
    if unknown:
        raise SystemExit(f"Unknown method(s): {', '.join(unknown)}")
    return methods


def worker_parameters(method: str, args: argparse.Namespace) -> dict[str, Any]:
    params: dict[str, Any] = {
        "maxMasks": args.max_masks,
        "decompositionDepth": args.depth,
        "minRegionAreaPct": args.min_area_pct,
        "personPriorConfidence": args.person_confidence,
        "personPriorMaxRegions": args.max_masks,
        "personPriorMinAreaPct": args.person_min_area_pct,
        "objectPriorConfidence": args.object_confidence,
        "objectPriorMaxRegions": args.max_masks,
        "objectPriorMinAreaPct": args.object_min_area_pct,
        "objectPriorImageSize": args.object_image_size,
    }
    if method == "underpaint-current":
        params.update(
            {
                "personPriorEnabled": True,
                "objectPriorEnabled": True,
                "samGridFallbackEnabled": False,
                "segmentationBackend": "sam",
            }
        )
    elif method == "underpaint-all-priors-grid":
        params.update(
            {
                "personPriorEnabled": True,
                "objectPriorEnabled": True,
                "samGridFallbackEnabled": True,
                "segmentationBackend": "sam",
            }
        )
    elif method == "underpaint-sam-grid":
        params.update(
            {
                "personPriorEnabled": False,
                "objectPriorEnabled": False,
                "samGridFallbackEnabled": True,
                "segmentationBackend": "sam",
            }
        )
    elif method == "underpaint-sam-hq-grid":
        params.update(
            {
                "personPriorEnabled": False,
                "objectPriorEnabled": False,
                "samGridFallbackEnabled": True,
                "segmentationBackend": "sam-hq",
            }
        )
    return params


def run_worker_method(
    method: str,
    source_path: Path,
    method_dir: Path,
    args: argparse.Namespace,
) -> dict[str, Any]:
    job_dir = method_dir / "job"
    job_dir.mkdir(parents=True, exist_ok=True)
    request_path = method_dir / "request.json"
    response_path = method_dir / "response.json"
    request = {
        "schema": "underpaint.ai-job.v1",
        "id": f"mask-bakeoff-{method}",
        "operation": "object-decomposition",
        "inputs": [
            {"role": "source-image", "path": str(source_path), "mimeType": "image/png"}
        ],
        "parameters": worker_parameters(method, args),
        "preferences": {"maxRenderEdge": args.max_edge},
        "provenance": {"createdBy": "underpaint-mask-bakeoff"},
    }
    write_json(request_path, request)

    env = os.environ.copy()
    if args.allow_cpu:
        env["UNDERPAINT_AI_ALLOW_CPU"] = "1"
    if args.debug_worker:
        env["UNDERPAINT_AI_DEBUG"] = "1"

    started = time.monotonic()
    try:
        completed = subprocess.run(
            [sys.executable, str(WORKER), str(request_path), str(response_path), str(job_dir)],
            cwd=REPO_ROOT,
            env=env,
            text=True,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            timeout=args.timeout,
            check=False,
        )
    except subprocess.TimeoutExpired as exc:
        return {
            "method": method,
            "status": "failed",
            "message": f"Timed out after {args.timeout} seconds.",
            "stdout": (exc.stdout or "").splitlines()[-20:] if exc.stdout else [],
            "stderr": (exc.stderr or "").splitlines()[-20:] if exc.stderr else [],
            "regions": [],
            "elapsedMsec": int((time.monotonic() - started) * 1000),
        }

    response: dict[str, Any] = {}
    if response_path.is_file():
        response = json.loads(response_path.read_text(encoding="utf-8"))

    regions: list[Region] = []
    for candidate in response.get("candidates", []) or []:
        metadata = candidate.get("metadata", {}) if isinstance(candidate, dict) else {}
        if not args.include_base and metadata.get("maskRole") == "base-remainder":
            continue
        image_path = expand_path(candidate.get("imagePath", ""))
        mask_path = expand_path(candidate.get("maskPath", ""))
        if image_path.is_file() and mask_path.is_file():
            regions.append(
                Region(
                    id=str(candidate.get("id") or f"{method}-{len(regions) + 1}"),
                    label=str(candidate.get("label") or f"{method} {len(regions) + 1}"),
                    image_path=image_path,
                    mask_path=mask_path,
                    metadata=metadata,
                )
            )

    status = response.get("status") or ("succeeded" if completed.returncode == 0 else "failed")
    return {
        "method": method,
        "status": status,
        "message": response.get("message", ""),
        "returnCode": completed.returncode,
        "stdout": completed.stdout.strip().splitlines()[-20:],
        "stderr": completed.stderr.strip().splitlines()[-20:],
        "regions": regions,
        "diagnostics": response.get("diagnostics", {}),
        "elapsedMsec": int((time.monotonic() - started) * 1000),
        "requestPath": str(request_path),
        "responsePath": str(response_path),
        "jobDir": str(job_dir),
    }


def yolo_model_path(method: str, args: argparse.Namespace) -> Path:
    if method == "yolo-person":
        return expand_path(args.person_model)
    return expand_path(args.object_model)


def run_yolo_method(
    method: str,
    source: Image.Image,
    source_path: Path,
    method_dir: Path,
    args: argparse.Namespace,
) -> dict[str, Any]:
    model_path = yolo_model_path(method, args)
    if not model_path.is_file() and not args.allow_downloads:
        return {
            "method": method,
            "status": "skipped",
            "message": (
                f"Model is missing: {model_path}. Run the matching downloader or "
                "pass --allow-downloads."
            ),
            "regions": [],
        }
    try:
        import numpy as np
        from ultralytics import YOLO
    except Exception as exc:  # noqa: BLE001
        return {
            "method": method,
            "status": "skipped",
            "message": f"ultralytics/numpy dependency is unavailable: {exc}",
            "regions": [],
        }

    started = time.monotonic()
    try:
        model_arg = str(model_path) if model_path.is_file() else "yolo11n-seg.pt"
        model = YOLO(model_arg)
        results = model(
            str(source_path),
            conf=args.person_confidence if method == "yolo-person" else args.object_confidence,
            imgsz=args.object_image_size,
            retina_masks=True,
            verbose=False,
        )
    except Exception as exc:  # noqa: BLE001
        return {
            "method": method,
            "status": "failed",
            "message": str(exc),
            "regions": [],
            "elapsedMsec": int((time.monotonic() - started) * 1000),
        }

    regions: list[Region] = []
    result = results[0] if results else None
    masks = getattr(result, "masks", None)
    boxes = getattr(result, "boxes", None)
    names = getattr(result, "names", {}) if result is not None else {}
    mask_tensors = (
        masks.data.detach().cpu().numpy()
        if masks is not None and getattr(masks, "data", None) is not None
        else None
    )
    if result is None or boxes is None or mask_tensors is None:
        return {
            "method": method,
            "status": "succeeded",
            "message": "No segmentation masks returned.",
            "regions": [],
            "elapsedMsec": int((time.monotonic() - started) * 1000),
        }

    box_xyxy = boxes.xyxy.detach().cpu().numpy() if getattr(boxes, "xyxy", None) is not None else []
    confidences = boxes.conf.detach().cpu().numpy() if getattr(boxes, "conf", None) is not None else []
    classes = boxes.cls.detach().cpu().numpy() if getattr(boxes, "cls", None) is not None else []
    detections: list[dict[str, Any]] = []
    for index, mask_array in enumerate(mask_tensors):
        class_id = int(classes[index]) if index < len(classes) else -1
        class_name = str(names.get(class_id, class_id))
        if method == "yolo-person" and class_name != "person":
            continue
        mask_image = Image.fromarray((mask_array > 0.5).astype("uint8") * 255, mode="L")
        if mask_image.size != source.size:
            mask_image = mask_image.resize(source.size, Image.Resampling.NEAREST)
        area = int(np.count_nonzero(np.asarray(mask_image) > 0))
        if area < int(source.width * source.height * (args.direct_min_area_pct / 100.0)):
            continue
        detections.append(
            {
                "index": index,
                "mask": mask_image,
                "area": area,
                "confidence": float(confidences[index]) if index < len(confidences) else 0.0,
                "classId": class_id,
                "className": class_name,
                "bbox": [float(value) for value in box_xyxy[index]] if index < len(box_xyxy) else [],
            }
        )

    detections.sort(key=lambda item: (item["confidence"], item["area"]), reverse=True)
    for output_index, item in enumerate(detections[: args.max_masks], start=1):
        label = f"{item['className']} {output_index}"
        mask_path = method_dir / f"mask-{output_index:03d}.png"
        image_path = method_dir / f"cutout-{output_index:03d}.png"
        alpha = item["mask"]
        if args.direct_feather > 0:
            alpha = alpha.filter(ImageFilter.GaussianBlur(radius=args.direct_feather))
        cutout = source.copy()
        cutout.putalpha(alpha)
        alpha.save(mask_path)
        cutout.save(image_path)
        regions.append(
            Region(
                id=f"{method}-{output_index:03d}",
                label=label,
                image_path=image_path,
                mask_path=mask_path,
                metadata={
                    "method": method,
                    "classId": item["classId"],
                    "className": item["className"],
                    "confidence": item["confidence"],
                    "areaPx": item["area"],
                    "bbox": item["bbox"],
                    "detectorModel": model_arg,
                },
            )
        )

    return {
        "method": method,
        "status": "succeeded",
        "message": f"Generated {len(regions)} YOLO mask(s).",
        "regions": regions,
        "elapsedMsec": int((time.monotonic() - started) * 1000),
        "modelPath": str(model_path),
        "actualModel": model_arg,
    }


def run_rembg_method(
    source: Image.Image,
    method_dir: Path,
    args: argparse.Namespace,
) -> dict[str, Any]:
    if not DEFAULT_REMBG_MODEL.is_file() and not args.allow_downloads:
        return {
            "method": "rembg-u2net",
            "status": "skipped",
            "message": (
                f"Rembg model is missing: {DEFAULT_REMBG_MODEL}. Run once with "
                "--allow-downloads to let rembg fetch it."
            ),
            "regions": [],
        }
    try:
        from rembg import remove
    except Exception as exc:  # noqa: BLE001
        return {
            "method": "rembg-u2net",
            "status": "skipped",
            "message": f"rembg dependency is unavailable: {exc}",
            "regions": [],
        }

    started = time.monotonic()
    try:
        cutout = remove(source.convert("RGB")).convert("RGBA")
    except Exception as exc:  # noqa: BLE001
        return {
            "method": "rembg-u2net",
            "status": "failed",
            "message": str(exc),
            "regions": [],
            "elapsedMsec": int((time.monotonic() - started) * 1000),
        }
    if cutout.size != source.size:
        cutout = cutout.resize(source.size, Image.Resampling.LANCZOS)
    alpha = cutout.getchannel("A")
    image_path = method_dir / "foreground.png"
    mask_path = method_dir / "foreground-mask.png"
    cutout.save(image_path)
    alpha.save(mask_path)
    region = Region(
        id="rembg-u2net-foreground",
        label="Rembg Foreground",
        image_path=image_path,
        mask_path=mask_path,
        metadata={"method": "rembg-u2net", "model": str(DEFAULT_REMBG_MODEL)},
    )
    return {
        "method": "rembg-u2net",
        "status": "succeeded",
        "message": "Generated 1 foreground matte.",
        "regions": [region],
        "elapsedMsec": int((time.monotonic() - started) * 1000),
    }


def run_birefnet_method(
    source: Image.Image,
    method_dir: Path,
    args: argparse.Namespace,
) -> dict[str, Any]:
    model_path = expand_path(args.birefnet_model)
    model_id = str(model_path) if model_path.is_dir() else args.birefnet_repo
    if not model_path.is_dir() and not args.allow_downloads:
        return {
            "method": "birefnet",
            "status": "skipped",
            "message": (
                f"BiRefNet model is missing: {model_path}. Run "
                "tools/ai/download-underpaint-birefnet.sh or pass --allow-downloads."
            ),
            "regions": [],
        }
    try:
        import torch
        from torchvision import transforms
        from transformers import AutoModelForImageSegmentation
    except Exception as exc:  # noqa: BLE001
        return {
            "method": "birefnet",
            "status": "skipped",
            "message": f"BiRefNet dependencies are unavailable: {exc}",
            "regions": [],
        }

    if torch.cuda.is_available():
        device = torch.device("cuda")
    elif args.allow_cpu:
        device = torch.device("cpu")
    else:
        return {
            "method": "birefnet",
            "status": "skipped",
            "message": "CUDA is unavailable; pass --allow-cpu for a slow CPU run.",
            "regions": [],
        }

    started = time.monotonic()
    model = None
    try:
        model = AutoModelForImageSegmentation.from_pretrained(
            model_id,
            trust_remote_code=True,
        ).to(device)
        model.eval()
        use_half = device.type == "cuda" and not args.birefnet_fp32
        if use_half:
            model.half()

        input_size = (args.birefnet_size, args.birefnet_size)
        transform_image = transforms.Compose(
            [
                transforms.Resize(input_size),
                transforms.ToTensor(),
                transforms.Normalize([0.485, 0.456, 0.406], [0.229, 0.224, 0.225]),
            ]
        )
        tensor = transform_image(source.convert("RGB")).unsqueeze(0).to(device)
        if use_half:
            tensor = tensor.half()
        with torch.no_grad():
            prediction = model(tensor)[-1].sigmoid().detach().cpu()[0].squeeze()
        mask = transforms.ToPILImage()(prediction).resize(source.size, Image.Resampling.LANCZOS)
    except Exception as exc:  # noqa: BLE001
        return {
            "method": "birefnet",
            "status": "failed",
            "message": str(exc),
            "regions": [],
            "elapsedMsec": int((time.monotonic() - started) * 1000),
            "model": model_id,
        }
    finally:
        try:
            del model
            if torch.cuda.is_available():
                torch.cuda.empty_cache()
        except Exception:
            pass

    image_path = method_dir / "foreground.png"
    mask_path = method_dir / "foreground-mask.png"
    cutout = source.copy()
    cutout.putalpha(mask)
    cutout.save(image_path)
    mask.save(mask_path)
    region = Region(
        id="birefnet-foreground",
        label="BiRefNet Foreground",
        image_path=image_path,
        mask_path=mask_path,
        metadata={
            "method": "birefnet",
            "model": model_id,
            "inputSize": args.birefnet_size,
            "precision": "fp32" if args.birefnet_fp32 or device.type != "cuda" else "fp16",
        },
    )
    return {
        "method": "birefnet",
        "status": "succeeded",
        "message": "Generated 1 foreground matte.",
        "regions": [region],
        "elapsedMsec": int((time.monotonic() - started) * 1000),
        "model": model_id,
        "device": str(device),
    }


def run_future_method(method: str) -> dict[str, Any]:
    notes = {
        "sam2": "Not wired yet. Candidate SAM 2.1 promptable mask backbone.",
        "grounded-sam2": "Not wired yet. Candidate text-prompted detector-plus-SAM path.",
        "sam3": "Not wired yet. Candidate concept-prompted segmentation path.",
        "ben2": "Not wired yet. Candidate background removal and edge refinement path.",
    }
    return {
        "method": method,
        "status": "skipped",
        "message": notes.get(method, "Future candidate is not wired yet."),
        "regions": [],
    }


def run_method(
    method: str,
    source: Image.Image,
    source_path: Path,
    method_dir: Path,
    args: argparse.Namespace,
) -> dict[str, Any]:
    method_dir.mkdir(parents=True, exist_ok=True)
    if method in WORKER_METHODS:
        return run_worker_method(method, source_path, method_dir, args)
    if method in {"yolo-object", "yolo-person"}:
        return run_yolo_method(method, source, source_path, method_dir, args)
    if method == "rembg-u2net":
        return run_rembg_method(source, method_dir, args)
    if method == "birefnet":
        return run_birefnet_method(source, method_dir, args)
    return run_future_method(method)


def report_method(
    result: dict[str, Any],
    source: Image.Image,
    method_dir: Path,
    args: argparse.Namespace,
) -> dict[str, Any]:
    regions = result.pop("regions", [])
    if regions:
        make_overlay(source, regions, method_dir / "overlay.png", args.preview_edge)
        make_contact_sheet(
            regions,
            method_dir / "contact-sheet.png",
            args.preview_edge,
            args.max_tiles,
        )
    result["regionCount"] = len(regions)
    result["regions"] = [region_summary(region) for region in regions]
    if regions:
        result["overlayPath"] = str(method_dir / "overlay.png")
        result["contactSheetPath"] = str(method_dir / "contact-sheet.png")
    write_json(method_dir / "report.json", result)
    return result


def list_methods() -> None:
    print("Installed/local method harnesses:")
    for method in sorted(DIRECT_METHODS | WORKER_METHODS):
        default_marker = " (default)" if method in DEFAULT_METHODS else ""
        print(f"  {method}{default_marker}")
    print("\nTracked future candidates:")
    for method in sorted(FUTURE_METHODS):
        print(f"  {method}")


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("image", nargs="?", help="Image to test.")
    parser.add_argument("--output-dir", help="Directory for reports and previews.")
    parser.add_argument(
        "--method",
        action="append",
        default=[],
        help="Method name or comma-separated method list. May be repeated.",
    )
    parser.add_argument("--all", action="store_true", help="Run all wired methods.")
    parser.add_argument("--list-methods", action="store_true", help="List methods and exit.")
    parser.add_argument("--max-edge", type=int, default=1024)
    parser.add_argument("--preview-edge", type=int, default=1200)
    parser.add_argument("--max-tiles", type=int, default=40)
    parser.add_argument("--max-masks", type=int, default=32)
    parser.add_argument("--depth", default="balanced", choices=["clean", "balanced", "detailed", "exhaustive"])
    parser.add_argument("--min-area-pct", type=float, default=0.25)
    parser.add_argument("--direct-min-area-pct", type=float, default=0.03)
    parser.add_argument("--person-min-area-pct", type=float, default=0.03)
    parser.add_argument("--object-min-area-pct", type=float, default=0.03)
    parser.add_argument("--person-confidence", type=float, default=0.05)
    parser.add_argument("--object-confidence", type=float, default=0.12)
    parser.add_argument("--object-image-size", type=int, default=1280)
    parser.add_argument("--direct-feather", type=float, default=0.0)
    parser.add_argument("--object-model", default=str(DEFAULT_OBJECT_MODEL))
    parser.add_argument("--person-model", default=str(DEFAULT_PERSON_MODEL))
    parser.add_argument("--birefnet-model", default=str(DEFAULT_BIREFNET_MODEL))
    parser.add_argument("--birefnet-repo", default="ZhengPeng7/BiRefNet")
    parser.add_argument("--birefnet-size", type=int, default=1024)
    parser.add_argument("--birefnet-fp32", action="store_true")
    parser.add_argument("--allow-cpu", action="store_true", help="Let worker SAM paths run on CPU.")
    parser.add_argument(
        "--allow-downloads",
        action="store_true",
        help="Allow backends such as rembg/YOLO to download missing default weights.",
    )
    parser.add_argument("--include-base", action="store_true", help="Include base-remainder worker layer.")
    parser.add_argument("--debug-worker", action="store_true")
    parser.add_argument("--timeout", type=int, default=600)
    args = parser.parse_args()

    if args.list_methods:
        list_methods()
        return 0
    if not args.image:
        parser.error("image is required unless --list-methods is used")

    image_path = expand_path(args.image)
    if not image_path.is_file():
        raise SystemExit(f"Image does not exist: {image_path}")

    output_dir = (
        expand_path(args.output_dir)
        if args.output_dir
        else Path(tempfile.mkdtemp(prefix="underpaint-mask-bakeoff-"))
    )
    output_dir.mkdir(parents=True, exist_ok=True)

    source = load_source(image_path, args.max_edge)
    source_path = output_dir / "source.png"
    source.save(source_path)

    methods = parse_methods(args)
    summary: dict[str, Any] = {
        "schema": "underpaint.mask-bakeoff.v1",
        "sourceImage": str(image_path),
        "workingSourcePath": str(source_path),
        "outputDir": str(output_dir),
        "workingSize": [source.width, source.height],
        "methods": [],
    }

    for method in methods:
        method_dir = output_dir / slugify(method)
        result = run_method(method, source, source_path, method_dir, args)
        summary["methods"].append(report_method(result, source, method_dir, args))

    write_json(output_dir / "summary.json", summary)
    print(json.dumps(summary, indent=2, sort_keys=True, default=str))
    return 0 if all(item["status"] in {"succeeded", "skipped"} for item in summary["methods"]) else 1


if __name__ == "__main__":
    raise SystemExit(main())
