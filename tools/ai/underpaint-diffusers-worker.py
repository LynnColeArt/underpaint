#!/usr/bin/env python3
"""Diffusers worker for the Underpaint AI job contract."""

from __future__ import annotations

import gc
import json
import math
import os
import random
import secrets
import shlex
import subprocess
import sys
import time
import traceback
from pathlib import Path
from typing import Any

from PIL import Image, ImageDraw, ImageFilter


SCHEMA = "underpaint.ai-job.v1"
DEFAULT_MODEL = "diffusers/stable-diffusion-xl-1.0-inpainting-0.1"
DEFAULT_REFINER_MODEL = "stabilityai/stable-diffusion-xl-refiner-1.0"
DEFAULT_SAM_MODEL = "facebook/sam-vit-base"
DEFAULT_SAM_HQ_MODEL = "syscv-community/sam-hq-vit-base"
DEFAULT_SAM_HQ_LOCAL_MODEL = (
    Path.home() / ".underpaint/models/segmentation/sam-hq-vit-base"
)
DEFAULT_GGUF_REFINER_MODEL = (
    Path.home()
    / ".underpaint/models/refiner/stable-diffusion-xl-refiner-1.0-GGUF/"
    / "stable-diffusion-xl-refiner-1.0-Q4_1.gguf"
)
DEFAULT_MODEL_REGISTRY = Path(__file__).with_name("model-registry.json")
DETAIL_MODEL_FILENAMES = {
    "face": "face_yolov8n.pt",
    "body": "person_yolov8n-seg.pt",
    "hands": "hand_yolov8n.pt",
}
DEFAULT_OBJECT_DETECTOR_MODEL = (
    Path.home() / ".underpaint/models/detection/yolo11n-seg.pt"
)
DEBUG_ENABLED = False
DEBUG_LOG_PATH: Path | None = None
DEBUG_EVENTS_PATH: Path | None = None
DEBUG_STARTED = time.monotonic()
os.environ.setdefault("PYTORCH_CUDA_ALLOC_CONF", "expandable_segments:True")


def env_flag(name: str) -> bool:
    return os.environ.get(name, "").strip().lower() in {"1", "true", "yes", "on"}


def configure_debug_logging(job_dir: Path) -> None:
    global DEBUG_ENABLED, DEBUG_EVENTS_PATH, DEBUG_LOG_PATH, DEBUG_STARTED
    DEBUG_ENABLED = env_flag("UNDERPAINT_AI_DEBUG")
    DEBUG_STARTED = time.monotonic()
    if not DEBUG_ENABLED:
        return
    DEBUG_LOG_PATH = job_dir / "worker-debug.jsonl"
    DEBUG_EVENTS_PATH = job_dir / "events.jsonl"
    for path in (DEBUG_LOG_PATH, DEBUG_EVENTS_PATH):
        try:
            path.unlink(missing_ok=True)
            path.touch()
        except OSError:
            pass
    debug_event(
        "worker-start",
        {
            "python": sys.version.split()[0],
            "pid": os.getpid(),
            "debugLogPath": str(DEBUG_LOG_PATH),
            "eventsPath": str(DEBUG_EVENTS_PATH),
        },
    )


def append_jsonl(path: Path, payload: dict[str, Any]) -> None:
    with path.open("a", encoding="utf-8") as handle:
        handle.write(json.dumps(payload, sort_keys=True, default=str) + "\n")


def debug_event(event_type: str, payload: dict[str, Any] | None = None) -> None:
    if not DEBUG_ENABLED or DEBUG_LOG_PATH is None:
        return
    record = {
        "schema": SCHEMA,
        "event": event_type,
        "elapsedMsec": int((time.monotonic() - DEBUG_STARTED) * 1000),
        **(payload or {}),
    }
    try:
        append_jsonl(DEBUG_LOG_PATH, record)
    except OSError:
        pass


def debug_progress_event(event: dict[str, Any]) -> None:
    if not DEBUG_ENABLED or DEBUG_EVENTS_PATH is None:
        return
    record = {
        "elapsedMsec": int((time.monotonic() - DEBUG_STARTED) * 1000),
        **event,
    }
    try:
        append_jsonl(DEBUG_EVENTS_PATH, record)
    except OSError:
        pass


def debug_diagnostics() -> dict[str, Any]:
    if not DEBUG_ENABLED or DEBUG_LOG_PATH is None or DEBUG_EVENTS_PATH is None:
        return {}
    return {
        "debug": {
            "logPath": str(DEBUG_LOG_PATH),
            "eventsPath": str(DEBUG_EVENTS_PATH),
        }
    }


def normalize_scheduler_name(value: Any) -> str:
    return str(value or "euler").strip().lower().replace("_", "-").replace(" ", "-")


def merge_prompt_text(*parts: str | None) -> str:
    return ", ".join(part.strip(" ,.;") for part in parts if part and part.strip(" ,.;"))


def outpaint_positive_prompt(prompt: str) -> str:
    return merge_prompt_text(
        prompt,
        (
            "continue the existing scene outward beyond the original image edges, "
            "natural scene continuation only, match the original camera angle, "
            "perspective, lens blur, color grading, lighting, texture, and depth "
            "of field, central image remains unchanged"
        ),
    )


def outpaint_negative_prompt(negative_prompt: str | None) -> str:
    return merge_prompt_text(
        negative_prompt,
        (
            "picture frame, border, mat, matte, poster edge, panel, artificial "
            "frame, metal frame, screws, rivets, bolts, vignette, decorative edge, "
            "flat color border, blank border, blurred border, floating rectangle, "
            "collage"
        ),
    )


def apply_scheduler(
    pipe: Any,
    scheduler_name: str,
    euler_discrete_scheduler: Any,
    euler_ancestral_discrete_scheduler: Any,
    dpm_solver_multistep_scheduler: Any,
    ddim_scheduler: Any,
    scheduler_config: Any | None = None,
) -> tuple[str, str]:
    normalized = normalize_scheduler_name(scheduler_name)
    config = scheduler_config or pipe.scheduler.config
    if normalized in ("euler", "euler-discrete", "eulerdiscrete"):
        pipe.scheduler = euler_discrete_scheduler.from_config(config)
        return "euler", pipe.scheduler.__class__.__name__
    if normalized in (
        "euler-a",
        "euler-ancestral",
        "euler-ancestral-discrete",
        "eulera",
    ):
        pipe.scheduler = euler_ancestral_discrete_scheduler.from_config(config)
        return "euler-a", pipe.scheduler.__class__.__name__
    if normalized in ("ddim", "ddim-scheduler"):
        pipe.scheduler = ddim_scheduler.from_config(config)
        return "ddim", pipe.scheduler.__class__.__name__
    if normalized in (
        "dpm",
        "dpmpp",
        "dpmpp-3m",
        "dpm++",
        "dpm++-3m",
        "dpmpp-2m",
        "dpm++-2m",
    ):
        pipe.scheduler = dpm_solver_multistep_scheduler.from_config(
            config,
            algorithm_type="dpmsolver++",
            solver_order=3,
            use_karras_sigmas=False,
        )
        return "dpmpp-3m", pipe.scheduler.__class__.__name__
    if normalized in (
        "dpm-karras",
        "dpmpp-karras",
        "dpmpp-3m-karras",
        "dpm++-karras",
        "dpm++-3m-karras",
        "dpmpp-2m-karras",
        "dpm++-2m-karras",
    ):
        pipe.scheduler = dpm_solver_multistep_scheduler.from_config(
            config,
            algorithm_type="dpmsolver++",
            solver_order=3,
            use_karras_sigmas=True,
        )
        return "dpmpp-3m-karras", pipe.scheduler.__class__.__name__
    raise ValueError(
        "Unsupported scheduler "
        f"{scheduler_name!r}. Expected euler, euler-a, ddim, dpmpp-3m, "
        "or dpmpp-3m-karras."
    )


def model_registry_path() -> Path:
    configured = os.environ.get("UNDERPAINT_MODEL_REGISTRY", "").strip()
    return Path(configured).expanduser() if configured else DEFAULT_MODEL_REGISTRY


def load_model_registry() -> dict[str, Any]:
    path = model_registry_path()
    try:
        return json.loads(path.read_text(encoding="utf-8"))
    except FileNotFoundError:
        debug_event("model-registry-missing", {"path": str(path)})
    except Exception as exc:  # noqa: BLE001
        debug_event("model-registry-error", {"path": str(path), "message": str(exc)})
    return {"models": []}


def model_registry_entry(model_id: str) -> dict[str, Any]:
    if not model_id:
        return {}
    for entry in load_model_registry().get("models", []):
        if isinstance(entry, dict) and entry.get("id") == model_id:
            return entry
    debug_event("model-registry-entry-missing", {"modelId": model_id})
    return {}


def expand_model_path(model: str) -> str:
    if not model:
        return model
    return os.path.expandvars(os.path.expanduser(model))


def normalized_generation_model(parameters: dict[str, Any]) -> dict[str, Any]:
    model_id = str(
        os.environ.get("UNDERPAINT_INPAINT_MODEL_ID")
        or parameters.get("modelId")
        or ""
    ).strip()
    registry_entry = model_registry_entry(model_id) if model_id else {}
    model = str(
        os.environ.get("UNDERPAINT_INPAINT_MODEL")
        or parameters.get("model")
        or registry_entry.get("model")
        or DEFAULT_MODEL
    ).strip()
    model_format = str(
        os.environ.get("UNDERPAINT_INPAINT_MODEL_FORMAT")
        or parameters.get("modelFormat")
        or registry_entry.get("format")
        or "diffusers_repo"
    ).strip().lower()
    if model_format not in {"diffusers_repo", "single_file_sdxl"}:
        model_format = "diffusers_repo"
    return {
        "modelId": model_id,
        "displayName": str(registry_entry.get("displayName", "")),
        "backend": str(registry_entry.get("backend") or "diffusers"),
        "format": model_format,
        "model": expand_model_path(model),
        "adapter": "masked_img2img" if model_format == "single_file_sdxl" else "inpaint",
    }


def normalized_segmentation_backend(parameters: dict[str, Any]) -> dict[str, Any]:
    model_id = str(
        os.environ.get("UNDERPAINT_SEGMENTATION_MODEL_ID")
        or parameters.get("segmentationModelId")
        or ""
    ).strip()
    registry_entry = model_registry_entry(model_id) if model_id else {}
    backend = str(
        os.environ.get("UNDERPAINT_SEGMENTATION_BACKEND")
        or parameters.get("segmentationBackend")
        or registry_entry.get("backend")
        or "sam"
    ).strip().lower()
    backend_aliases = {
        "hq-sam": "sam-hq",
        "hqsam": "sam-hq",
        "sam-hq-transformers": "sam-hq",
        "samhq": "sam-hq",
        "transformers-sam-hq": "sam-hq",
        "sam1": "sam",
        "sam-vit": "sam",
        "transformers-sam": "sam",
    }
    backend = backend_aliases.get(backend, backend)
    if backend not in {"sam", "sam-hq"}:
        backend = "sam"

    if backend == "sam-hq":
        default_model = (
            str(DEFAULT_SAM_HQ_LOCAL_MODEL)
            if DEFAULT_SAM_HQ_LOCAL_MODEL.exists()
            else DEFAULT_SAM_HQ_MODEL
        )
        env_model = (
            os.environ.get("UNDERPAINT_SAM_HQ_MODEL")
            or os.environ.get("UNDERPAINT_SEGMENTATION_MODEL")
        )
    else:
        default_model = DEFAULT_SAM_MODEL
        env_model = (
            os.environ.get("UNDERPAINT_SAM_MODEL")
            or os.environ.get("UNDERPAINT_SEGMENTATION_MODEL")
        )

    model = str(
        env_model
        or parameters.get("segmentationModel")
        or parameters.get("model")
        or registry_entry.get("model")
        or default_model
    ).strip()
    return {
        "backend": backend,
        "modelId": model_id,
        "displayName": str(registry_entry.get("displayName", "")),
        "model": expand_model_path(model),
    }


def effective_diffusion_steps(steps: int, strength: float) -> int:
    steps = max(1, int(steps))
    strength = max(0.0, min(float(strength), 1.0))
    return max(1, min(int(steps * strength), steps))


def normalized_refiner(parameters: dict[str, Any]) -> dict[str, Any]:
    raw = parameters.get("refiner", {})
    if not isinstance(raw, dict):
        raw = {}
    backend = str(
        os.environ.get("UNDERPAINT_REFINER_BACKEND")
        or raw.get("backend")
        or "diffusers"
    ).strip().lower()
    if backend not in {"diffusers", "gguf"}:
        backend = "diffusers"
    model_id = str(
        os.environ.get("UNDERPAINT_REFINER_MODEL_ID")
        or raw.get("modelId")
        or ""
    ).strip()
    registry_entry = model_registry_entry(model_id) if model_id else {}
    backend = str(
        os.environ.get("UNDERPAINT_REFINER_BACKEND")
        or raw.get("backend")
        or registry_entry.get("backend")
        or backend
    ).strip().lower()
    if backend not in {"diffusers", "gguf"}:
        backend = "diffusers"
    model = str(
        (
            os.environ.get("UNDERPAINT_REFINER_GGUF_MODEL")
            if backend == "gguf"
            else None
        )
        or os.environ.get("UNDERPAINT_REFINER_MODEL")
        or raw.get("model")
        or registry_entry.get("model")
        or DEFAULT_REFINER_MODEL
    ).strip()
    if not model:
        model = str(DEFAULT_GGUF_REFINER_MODEL) if backend == "gguf" else DEFAULT_REFINER_MODEL
    if backend == "gguf" and model == DEFAULT_REFINER_MODEL:
        model = str(DEFAULT_GGUF_REFINER_MODEL)
    placement = str(raw.get("placement", "before-detail")).strip().lower()
    if placement not in {"before-detail", "after-detail"}:
        placement = "before-detail"
    strength = max(0.05, min(float(raw.get("strength", 0.25)), 1.0))
    steps = max(1, min(int(raw.get("steps", 20)), 200))
    if strength > 0.0 and int(steps * strength) < 1:
        steps = max(steps, math.ceil(1.0 / strength))
    effective_steps = effective_diffusion_steps(steps, strength)
    return {
        "enabled": bool(raw.get("enabled", False)),
        "modelId": model_id,
        "backend": backend,
        "model": expand_model_path(model),
        "displayName": str(registry_entry.get("displayName", "")),
        "strength": strength,
        "steps": steps,
        "effectiveSteps": effective_steps,
        "scheduler": normalize_scheduler_name(raw.get("scheduler", "dpmpp-3m-karras")),
        "placement": placement,
        "runner": str(os.environ.get("UNDERPAINT_GGUF_REFINER_WORKER", "")).strip(),
    }


def refiner_diagnostics(refiner: dict[str, Any]) -> dict[str, Any]:
    diagnostics = {
        "enabled": refiner["enabled"],
        "backend": refiner["backend"],
        "model": refiner["model"],
        "strength": refiner["strength"],
        "steps": refiner["steps"],
        "effectiveSteps": refiner["effectiveSteps"],
        "scheduler": refiner["scheduler"],
        "placement": refiner["placement"],
        "appliedCandidates": 0,
    }
    diagnostics["status"] = "pending" if refiner["enabled"] else "disabled"
    return diagnostics


def normalized_detail_pass(parameters: dict[str, Any]) -> dict[str, Any]:
    raw = parameters.get("detailPass", {})
    if not isinstance(raw, dict):
        raw = {}
    enabled = bool(raw.get("enabled", False))
    denoise = max(0.05, min(float(raw.get("denoise", 0.35)), 1.0))
    steps = max(1, min(int(raw.get("steps", 28)), 200))
    return {
        "enabled": enabled,
        "faceEnabled": bool(raw.get("faceEnabled", True)),
        "bodyEnabled": bool(raw.get("bodyEnabled", False)),
        "handsEnabled": bool(raw.get("handsEnabled", False)),
        "detectionConfidence": max(
            0.01, min(float(raw.get("detectionConfidence", 0.5)), 1.0)
        ),
        "maxRegions": max(1, min(int(raw.get("maxRegions", 4)), 16)),
        "maskPaddingPx": max(0, min(int(raw.get("maskPaddingPx", 32)), 256)),
        "detailRenderEdge": max(256, min(int(raw.get("detailRenderEdge", 768)), 1536)),
        "minCropEdge": max(64, min(int(raw.get("minCropEdge", 256)), 1024)),
        "denoise": denoise,
        "steps": steps,
        "effectiveSteps": effective_diffusion_steps(steps, denoise),
        "scheduler": normalize_scheduler_name(raw.get("scheduler", "euler")),
        "fallbackToEditMask": False,
    }


def detail_pass_diagnostics(detail_pass: dict[str, Any]) -> dict[str, Any]:
    enabled_regions = [
        name
        for name, enabled in (
            ("face", detail_pass["faceEnabled"]),
            ("body", detail_pass["bodyEnabled"]),
            ("hands", detail_pass["handsEnabled"]),
        )
        if enabled
    ]
    diagnostics = {
        "enabled": detail_pass["enabled"],
        "regions": enabled_regions,
        "detectionConfidence": detail_pass["detectionConfidence"],
        "maxRegions": detail_pass["maxRegions"],
        "maskPaddingPx": detail_pass["maskPaddingPx"],
        "detailRenderEdge": detail_pass["detailRenderEdge"],
        "minCropEdge": detail_pass["minCropEdge"],
        "denoise": detail_pass["denoise"],
        "steps": detail_pass["steps"],
        "effectiveSteps": detail_pass["effectiveSteps"],
        "scheduler": detail_pass["scheduler"],
        "fallbackToEditMask": detail_pass["fallbackToEditMask"],
        "detectedRegions": 0,
        "appliedRegions": 0,
        "fallbackRegions": 0,
    }
    if not detail_pass["enabled"]:
        diagnostics["status"] = "disabled"
    elif not enabled_regions:
        diagnostics["status"] = "no-regions-enabled"
    else:
        diagnostics["status"] = "pending"
    return diagnostics


def detail_model_dir() -> Path:
    return Path(
        os.environ.get(
            "UNDERPAINT_DETAIL_MODEL_DIR",
            str(Path.home() / ".underpaint/models/detail/adetailer"),
        )
    )


def enabled_detail_regions(detail_pass: dict[str, Any]) -> list[str]:
    return [
        name
        for name, enabled in (
            ("face", detail_pass["faceEnabled"]),
            ("body", detail_pass["bodyEnabled"]),
            ("hands", detail_pass["handsEnabled"]),
        )
        if enabled
    ]


def detail_model_path(region: str) -> Path:
    env_name = f"UNDERPAINT_DETAIL_{region.upper()}_MODEL"
    configured = os.environ.get(env_name)
    if configured:
        return Path(configured)
    return detail_model_dir() / DETAIL_MODEL_FILENAMES[region]


def object_detector_model_path() -> Path:
    configured = os.environ.get("UNDERPAINT_OBJECT_DETECTOR_MODEL", "").strip()
    return Path(configured).expanduser() if configured else DEFAULT_OBJECT_DETECTOR_MODEL


def load_detail_detectors(detail_pass: dict[str, Any]) -> tuple[list[dict[str, Any]], dict[str, Any]]:
    report = detail_pass_diagnostics(detail_pass)
    if report["status"] != "pending":
        debug_event("detail-detectors-skipped", {"status": report["status"]})
        return [], report

    regions = enabled_detail_regions(detail_pass)
    missing = [
        {"region": region, "path": str(detail_model_path(region))}
        for region in regions
        if not detail_model_path(region).is_file()
    ]
    if missing:
        report["status"] = "models-missing"
        report["missingModels"] = missing
        report["message"] = "One or more detail detector models are missing."
        debug_event("detail-detectors-missing", {"missingModels": missing})
        return [], report

    try:
        from ultralytics import YOLO
    except Exception as exc:  # noqa: BLE001
        report["status"] = "dependency-unavailable"
        report["message"] = f"Ultralytics is not installed: {exc}"
        debug_event("detail-detectors-dependency-error", {"message": str(exc)})
        return [], report

    detectors: list[dict[str, Any]] = []
    for region in regions:
        path = detail_model_path(region)
        detectors.append({"region": region, "path": str(path), "model": YOLO(str(path))})

    report["status"] = "ready"
    report["models"] = [
        {"region": detector["region"], "path": detector["path"]}
        for detector in detectors
    ]
    debug_event("detail-detectors-ready", {"models": report["models"]})
    return detectors, report


def load_pipeline(pipeline_class: Any, model_id: str, load_kwargs: dict[str, Any]) -> Any:
    kwargs = dict(load_kwargs)
    try:
        return pipeline_class.from_pretrained(model_id, **kwargs)
    except Exception as exc:  # noqa: BLE001
        if kwargs.get("variant") == "fp16":
            kwargs.pop("variant", None)
            debug_event(
                "model-load-fp16-variant-fallback",
                {"model": model_id, "message": str(exc)},
            )
            return pipeline_class.from_pretrained(model_id, **kwargs)
        raise


def load_single_file_pipeline(
    pipeline_class: Any, checkpoint_path: str, load_kwargs: dict[str, Any]
) -> Any:
    kwargs = dict(load_kwargs)
    kwargs.pop("variant", None)
    try:
        return pipeline_class.from_single_file(checkpoint_path, **kwargs)
    except TypeError:
        kwargs.pop("use_safetensors", None)
        return pipeline_class.from_single_file(checkpoint_path, **kwargs)


def unload_pipeline(pipe: Any, torch: Any, label: str) -> None:
    if pipe is None:
        return
    debug_event("model-unload-start", {"label": label})
    try:
        pipe.to("cpu")
    except Exception as exc:  # noqa: BLE001
        debug_event("model-unload-cpu-error", {"label": label, "message": str(exc)})
    del pipe
    gc.collect()
    if getattr(torch, "cuda", None) is not None and torch.cuda.is_available():
        torch.cuda.empty_cache()
        try:
            torch.cuda.ipc_collect()
        except Exception:
            pass
    debug_event("model-unload-complete", {"label": label})


def release_cuda_memory() -> None:
    gc.collect()
    try:
        import torch

        if torch.cuda.is_available():
            torch.cuda.empty_cache()
            try:
                torch.cuda.ipc_collect()
            except Exception:
                pass
    except Exception:
        pass


def prepare_pipeline_for_device(
    pipe: Any,
    device: str,
    preferences: dict[str, Any],
    label: str,
    *,
    prefer_cpu_offload: bool = False,
) -> str:
    safe_4070 = bool(preferences.get("safe4070Mode", True))
    use_cpu_offload = device == "cuda" and (
        env_flag("UNDERPAINT_AI_CPU_OFFLOAD")
        or env_flag("UNDERPAINT_AI_MODEL_CPU_OFFLOAD")
        or (prefer_cpu_offload and safe_4070)
    )
    if use_cpu_offload and hasattr(pipe, "enable_model_cpu_offload"):
        pipe.enable_model_cpu_offload()
        debug_event(
            "model-device-ready",
            {"label": label, "device": device, "mode": "model-cpu-offload"},
        )
        return "model-cpu-offload"

    pipe.to(device)
    debug_event("model-device-ready", {"label": label, "device": device, "mode": "direct"})
    return "direct"


def is_cuda_oom(exc: BaseException) -> bool:
    return "CUDA out of memory" in str(exc)


def cuda_oom_message(
    exc: BaseException,
    *,
    refiner: dict[str, Any],
    detail_pass: dict[str, Any],
    target_render_edge: int,
    candidate_count: int,
) -> str:
    active_parts = []
    if refiner.get("enabled"):
        active_parts.append(f"refiner={refiner.get('backend', 'unknown')}")
    if detail_pass.get("enabled"):
        active_parts.append("detail pass=on")
    active = ", ".join(active_parts) if active_parts else "no optional post-pass"
    return (
        "CUDA ran out of VRAM during the local Diffusers job. "
        f"Render edge was {target_render_edge}px with {candidate_count} candidate(s) "
        f"and {active}. Try turning off the refiner first, then detail pass, "
        "or reduce the render size. Close other GPU-heavy apps if possible. "
        "Underpaint now sets PYTORCH_CUDA_ALLOC_CONF=expandable_segments:True by "
        "default; for slower but lower-VRAM model loading, launch with "
        "UNDERPAINT_AI_CPU_OFFLOAD=1. Original CUDA error: "
        f"{str(exc).splitlines()[0]}"
    )


def prefill_masked_img2img_source(
    source: Image.Image,
    mask: Image.Image,
    seed: int,
    *,
    noise_mix: float = 0.72,
) -> Image.Image:
    """Give non-inpaint SDXL checkpoints editable pixels they can actually change."""
    mask_luma = mask.convert("L").point(lambda pixel: 255 if pixel > 8 else 0)
    if mask_luma.getbbox() is None:
        return source

    blur_radius = max(12, min(max(source.size) // 12, 96))
    blurred = source.convert("RGB").filter(ImageFilter.GaussianBlur(radius=blur_radius))
    rng = random.Random(seed if seed >= 0 else secrets.randbits(63))
    noise = Image.frombytes(
        "RGB", source.size, rng.randbytes(source.width * source.height * 3)
    )
    fill = Image.blend(blurred, noise, max(0.0, min(noise_mix, 1.0)))
    return Image.composite(fill, source.convert("RGB"), mask_luma)


def masked_img2img_strength(requested_strength: float) -> float:
    configured = os.environ.get("UNDERPAINT_SINGLE_FILE_MAX_DENOISE", "").strip()
    try:
        max_strength = float(configured) if configured else 0.68
    except ValueError:
        max_strength = 0.68
    max_strength = max(0.05, min(max_strength, 1.0))
    return max(0.05, min(requested_strength, max_strength))


def generate_base_candidate(
    pipe: Any,
    torch: Any,
    model_config: dict[str, Any],
    source: Image.Image,
    mask: Image.Image,
    prompt: str,
    negative_prompt: str | None,
    cfg: float,
    strength: float,
    steps: int,
    seed: int,
    device: str,
    callback: Any,
) -> Image.Image:
    if model_config["format"] == "single_file_sdxl":
        try:
            noise_mix = float(os.environ.get("UNDERPAINT_SINGLE_FILE_PREFILL_NOISE", 0.35))
        except ValueError:
            noise_mix = 0.35
        source = prefill_masked_img2img_source(source, mask, seed, noise_mix=noise_mix)
        return pipe(
            prompt=prompt,
            negative_prompt=negative_prompt,
            image=source,
            guidance_scale=cfg,
            strength=strength,
            num_inference_steps=steps,
            generator=generator_for_seed(torch, seed, device),
            callback_on_step_end=callback,
            callback_on_step_end_tensor_inputs=["latents"],
        ).images[0]

    return pipe(
        prompt=prompt,
        negative_prompt=negative_prompt,
        image=source,
        mask_image=mask,
        height=source.height,
        width=source.width,
        guidance_scale=cfg,
        strength=strength,
        num_inference_steps=steps,
        generator=generator_for_seed(torch, seed, device),
        callback_on_step_end=callback,
        callback_on_step_end_tensor_inputs=["latents"],
    ).images[0]


def write_json(path: Path, payload: dict[str, Any]) -> None:
    path.write_text(json.dumps(payload, indent=2), encoding="utf-8")


def emit_progress(payload: dict[str, Any]) -> None:
    event = {"schema": SCHEMA, **payload}
    debug_progress_event(event)
    print(json.dumps(event, separators=(",", ":")), flush=True)


def response(
    request_id: str,
    status: str,
    message: str,
    *,
    candidates: list[dict[str, Any]] | None = None,
    diagnostics: dict[str, Any] | None = None,
    provenance: dict[str, Any] | None = None,
) -> dict[str, Any]:
    return {
        "schema": SCHEMA,
        "id": request_id,
        "status": status,
        "message": message,
        "candidates": candidates or [],
        "diagnostics": diagnostics or {},
        "provenance": provenance or {},
    }


def fail(response_path: Path, request_id: str, message: str, code: int = 1) -> int:
    diagnostics = debug_diagnostics()
    debug_event(
        "failure",
        {
            "requestId": request_id,
            "message": message,
            "code": code,
        },
    )
    write_json(
        response_path,
        response(
            request_id,
            "failed",
            message,
            diagnostics=diagnostics,
            provenance={"backend": "diffusers", "schema": SCHEMA},
        ),
    )
    return code


def asset_path(request: dict[str, Any], role: str) -> Path | None:
    for asset in request.get("inputs", []):
        if asset.get("role") == role and asset.get("path"):
            return Path(asset["path"])
    return None


def region_label(index: int, count: int) -> str:
    labels = ["Shadows", "Darks", "Midtones", "Lights", "Highlights"]
    if count == len(labels):
        return labels[index]
    return f"Region {index + 1}"


def decomposition_depth_label(depth: str) -> str:
    labels = {
        "clean": "Clean",
        "balanced": "Balanced",
        "detailed": "Detailed",
        "exhaustive": "Exhaustive",
    }
    return labels.get(depth, "Balanced")


def slugify(value: str) -> str:
    slug = []
    previous_dash = False
    for char in value.strip().lower():
        if char.isalnum():
            slug.append(char)
            previous_dash = False
        elif not previous_dash:
            slug.append("-")
            previous_dash = True
    return "".join(slug).strip("-") or "region-group"


def placeholder_region_group(label: str, index: int, grouped: bool) -> tuple[str, str]:
    if not grouped:
        group_label = f"{label} Regions"
        return slugify(group_label), group_label
    if label in {"Shadows", "Darks", "Midtones", "Lights", "Highlights"}:
        return "tonal-regions", "Tonal Regions"
    if index >= 8:
        return "small-repeating-regions", "Small Repeating Regions"
    return "foreground-regions", "Foreground Regions"


def decomposition_grid_size(depth: str) -> int:
    if depth == "clean":
        return 6
    if depth == "detailed":
        return 12
    if depth == "exhaustive":
        return 16
    return 9


def mask_bounds(mask: Any) -> tuple[int, int, int, int] | None:
    import numpy as np

    ys, xs = np.where(mask)
    if xs.size == 0 or ys.size == 0:
        return None
    return int(xs.min()), int(ys.min()), int(xs.max()) + 1, int(ys.max()) + 1


def mask_iou(left: Any, right: Any) -> float:
    import numpy as np

    intersection = np.logical_and(left, right).sum()
    union = np.logical_or(left, right).sum()
    return float(intersection) / float(union) if union else 0.0


def mask_overlap_of_smaller(left: Any, right: Any) -> float:
    import numpy as np

    intersection = int(np.logical_and(left, right).sum())
    smaller = min(int(np.count_nonzero(left)), int(np.count_nonzero(right)))
    return float(intersection) / float(smaller) if smaller else 0.0


def mask_edge_count(bounds: tuple[int, int, int, int], width: int, height: int, margin: int = 2) -> int:
    left, top, right, bottom = bounds
    return int(left <= margin) + int(top <= margin) + int(right >= width - margin) + int(bottom >= height - margin)


def is_background_like_mask(
    mask: Any,
    bounds: tuple[int, int, int, int],
    width: int,
    height: int,
) -> bool:
    import numpy as np

    total_pixels = max(1, width * height)
    left, top, right, bottom = bounds
    area_ratio = float(np.count_nonzero(mask)) / total_pixels
    bounds_ratio = float((right - left) * (bottom - top)) / total_pixels
    edge_count = mask_edge_count(bounds, width, height)
    return (
        area_ratio >= 0.92
        or (bounds_ratio >= 0.96 and edge_count >= 3)
        or (area_ratio >= 0.72 and edge_count == 4)
    )


def is_sparse_sprawling_mask(
    mask: Any,
    bounds: tuple[int, int, int, int],
    width: int,
    height: int,
) -> bool:
    import numpy as np

    total_pixels = max(1, width * height)
    left, top, right, bottom = bounds
    bounds_width = max(1, right - left)
    bounds_height = max(1, bottom - top)
    bounds_area = bounds_width * bounds_height
    bounds_ratio = float(bounds_area) / total_pixels
    density = float(np.count_nonzero(mask)) / bounds_area
    return (
        bounds_ratio >= 0.16
        and density <= 0.025
        and (bounds_width >= width * 0.45 or bounds_height >= height * 0.45)
    )


def object_mask_utility(
    area: int,
    bounds: tuple[int, int, int, int],
    width: int,
    height: int,
    sam_score: float,
    prior: str = "",
) -> float:
    total_pixels = max(1, width * height)
    area_ratio = float(area) / total_pixels
    edge_count = mask_edge_count(bounds, width, height)
    left, top, right, bottom = bounds
    bounds_ratio = float((right - left) * (bottom - top)) / total_pixels
    utility = sam_score

    if area_ratio < 0.015:
        utility -= 0.08
    elif area_ratio <= 0.18:
        utility += 0.12
    elif area_ratio <= 0.32:
        utility -= 0.15
    else:
        utility -= 0.35

    utility -= edge_count * 0.08
    if edge_count >= 2 and bounds_ratio > 0.35:
        utility -= 0.25
    if edge_count >= 3:
        utility -= 0.25
    if prior == "sam-grid" and edge_count <= 1:
        # Grid masks are the catch-all path for props, notes, signs, odd
        # creatures, and background objects that generic detectors miss.
        if 0.002 <= area_ratio <= 0.08:
            utility += 0.12
        elif area_ratio < 0.015:
            utility += 0.05
    return utility


def remove_small_mask_components(mask: Any, min_component_area: int) -> Any:
    import numpy as np

    try:
        import cv2

        labels_count, labels, stats, _ = cv2.connectedComponentsWithStats(
            mask.astype("uint8"), 8
        )
        if labels_count <= 1:
            return np.zeros(mask.shape, dtype=bool)
        keep = stats[:, cv2.CC_STAT_AREA] >= min_component_area
        keep[0] = False
        return keep[labels]
    except Exception:
        pass

    height, width = mask.shape
    visited = np.zeros(mask.shape, dtype=bool)
    cleaned = np.zeros(mask.shape, dtype=bool)
    ys, xs = np.where(mask)
    for start_y, start_x in zip(ys, xs):
        if visited[start_y, start_x]:
            continue
        stack = [(int(start_x), int(start_y))]
        visited[start_y, start_x] = True
        component: list[tuple[int, int]] = []
        while stack:
            x, y = stack.pop()
            component.append((x, y))
            for next_x, next_y in (
                (x - 1, y),
                (x + 1, y),
                (x, y - 1),
                (x, y + 1),
            ):
                if (
                    0 <= next_x < width
                    and 0 <= next_y < height
                    and mask[next_y, next_x]
                    and not visited[next_y, next_x]
                ):
                    visited[next_y, next_x] = True
                    stack.append((next_x, next_y))
        if len(component) >= min_component_area:
            for x, y in component:
                cleaned[y, x] = True
    return cleaned


def fill_small_mask_holes(mask: Any, max_hole_area: int) -> Any:
    import numpy as np

    try:
        import cv2

        background = (~mask).astype("uint8")
        labels_count, labels, stats, _ = cv2.connectedComponentsWithStats(
            background, 8
        )
        if labels_count <= 1:
            return mask
        height, width = mask.shape
        cleaned = mask.copy()
        for label in range(1, labels_count):
            component = labels == label
            ys, xs = np.where(component)
            if xs.size == 0 or ys.size == 0:
                continue
            touches_edge = (
                xs.min() == 0
                or ys.min() == 0
                or xs.max() == width - 1
                or ys.max() == height - 1
            )
            if not touches_edge and stats[label, cv2.CC_STAT_AREA] <= max_hole_area:
                cleaned[component] = True
        return cleaned
    except Exception:
        pass

    height, width = mask.shape
    outside = np.zeros(mask.shape, dtype=bool)
    stack: list[tuple[int, int]] = []

    def push_if_background(x: int, y: int) -> None:
        if not mask[y, x] and not outside[y, x]:
            outside[y, x] = True
            stack.append((x, y))

    for x in range(width):
        push_if_background(x, 0)
        push_if_background(x, height - 1)
    for y in range(height):
        push_if_background(0, y)
        push_if_background(width - 1, y)

    while stack:
        x, y = stack.pop()
        for next_x, next_y in (
            (x - 1, y),
            (x + 1, y),
            (x, y - 1),
            (x, y + 1),
        ):
            if 0 <= next_x < width and 0 <= next_y < height:
                push_if_background(next_x, next_y)

    cleaned = mask.copy()
    visited_holes = outside.copy()
    hole_ys, hole_xs = np.where(np.logical_and(~mask, ~visited_holes))
    for start_y, start_x in zip(hole_ys, hole_xs):
        if visited_holes[start_y, start_x] or mask[start_y, start_x]:
            continue
        stack = [(int(start_x), int(start_y))]
        visited_holes[start_y, start_x] = True
        component: list[tuple[int, int]] = []
        while stack:
            x, y = stack.pop()
            component.append((x, y))
            for next_x, next_y in (
                (x - 1, y),
                (x + 1, y),
                (x, y - 1),
                (x, y + 1),
            ):
                if (
                    0 <= next_x < width
                    and 0 <= next_y < height
                    and not mask[next_y, next_x]
                    and not visited_holes[next_y, next_x]
                ):
                    visited_holes[next_y, next_x] = True
                    stack.append((next_x, next_y))
        if len(component) <= max_hole_area:
            for x, y in component:
                cleaned[y, x] = True
    return cleaned


def clean_object_mask(mask: Any, source_alpha: Any, min_component_area: int) -> Any:
    import numpy as np

    cleaned = np.logical_and(mask, source_alpha)
    cleaned = remove_small_mask_components(cleaned, min_component_area)
    if np.count_nonzero(cleaned) == 0:
        return cleaned
    cleaned = fill_small_mask_holes(cleaned, max(64, min_component_area // 2))
    cleaned = np.logical_and(cleaned, source_alpha)
    return cleaned


def soft_object_mask(mask: Any, radius: float) -> Image.Image:
    hard_mask = Image.fromarray((mask.astype("uint8") * 255), mode="L")
    if radius <= 0.0:
        return hard_mask
    return hard_mask.filter(ImageFilter.GaussianBlur(radius=radius))


def erode_boolean_mask(mask: Any, radius: int) -> Any:
    import numpy as np

    if radius <= 0:
        return mask
    size = max(3, radius * 2 + 1)
    if size % 2 == 0:
        size += 1
    mask_image = Image.fromarray((mask.astype("uint8") * 255), mode="L")
    eroded = mask_image.filter(ImageFilter.MinFilter(size=size))
    return np.asarray(eroded) > 0


def dilate_boolean_mask(mask: Any, radius: int) -> Any:
    import numpy as np

    if radius <= 0:
        return mask
    size = max(3, radius * 2 + 1)
    if size % 2 == 0:
        size += 1
    mask_image = Image.fromarray((mask.astype("uint8") * 255), mode="L")
    dilated = mask_image.filter(ImageFilter.MaxFilter(size=size))
    return np.asarray(dilated) > 0


def source_with_alpha(source: Image.Image, alpha: Image.Image) -> Image.Image:
    layer = source.copy()
    layer.putalpha(alpha)
    return layer


def load_segmentation_model(segmentation: dict[str, Any], device: str) -> tuple[Any, Any]:
    if segmentation["backend"] == "sam-hq":
        from transformers import SamHQModel, SamHQProcessor

        processor = SamHQProcessor.from_pretrained(segmentation["model"])
        model = SamHQModel.from_pretrained(segmentation["model"]).to(device)
    else:
        from transformers import SamModel, SamProcessor

        processor = SamProcessor.from_pretrained(segmentation["model"])
        model = SamModel.from_pretrained(segmentation["model"]).to(device)
    return processor, model


def clamp_box(
    box: tuple[float, float, float, float] | list[float],
    width: int,
    height: int,
) -> tuple[float, float, float, float]:
    left, top, right, bottom = [float(value) for value in box]
    left = max(0.0, min(left, float(width - 1)))
    top = max(0.0, min(top, float(height - 1)))
    right = max(left + 1.0, min(right, float(width)))
    bottom = max(top + 1.0, min(bottom, float(height)))
    return left, top, right, bottom


def expand_box(
    box: tuple[float, float, float, float],
    width: int,
    height: int,
    pct: float = 0.06,
    min_px: int = 10,
) -> tuple[float, float, float, float]:
    left, top, right, bottom = box
    pad_x = max(float(min_px), (right - left) * pct)
    pad_y = max(float(min_px), (bottom - top) * pct)
    return clamp_box((left - pad_x, top - pad_y, right + pad_x, bottom + pad_y), width, height)


def box_area(box: tuple[float, float, float, float]) -> float:
    left, top, right, bottom = box
    return max(0.0, right - left) * max(0.0, bottom - top)


def box_overlap_of_smaller(
    left_box: tuple[float, float, float, float],
    right_box: tuple[float, float, float, float],
) -> float:
    left = max(left_box[0], right_box[0])
    top = max(left_box[1], right_box[1])
    right = min(left_box[2], right_box[2])
    bottom = min(left_box[3], right_box[3])
    intersection = max(0.0, right - left) * max(0.0, bottom - top)
    smaller = min(box_area(left_box), box_area(right_box))
    return intersection / smaller if smaller > 0.0 else 0.0


def merged_person_detection_clusters(
    detections: list[dict[str, Any]],
    width: int,
    height: int,
    overlap_threshold: float = 0.50,
) -> list[dict[str, Any]]:
    import numpy as np

    clusters: list[dict[str, Any]] = []
    for detection in sorted(detections, key=lambda item: item["score"], reverse=True):
        detection_box = detection["box"]
        detection_mask = detection.get("yoloMask")
        matched: dict[str, Any] | None = None
        for cluster in clusters:
            cluster_mask = cluster.get("yoloMask")
            if detection_mask is not None and cluster_mask is not None:
                overlap = mask_overlap_of_smaller(detection_mask, cluster_mask)
            else:
                overlap = box_overlap_of_smaller(detection_box, cluster["box"])
            if overlap >= overlap_threshold:
                matched = cluster
                break
        if matched is None:
            clusters.append(
                {
                    "box": detection_box,
                    "score": detection["score"],
                    "detections": [detection],
                    "yoloMask": detection_mask,
                }
            )
            continue

        left = min(matched["box"][0], detection_box[0])
        top = min(matched["box"][1], detection_box[1])
        right = max(matched["box"][2], detection_box[2])
        bottom = max(matched["box"][3], detection_box[3])
        matched["box"] = clamp_box((left, top, right, bottom), width, height)
        matched["score"] = max(float(matched["score"]), float(detection["score"]))
        matched["detections"].append(detection)
        if detection_mask is not None:
            if matched.get("yoloMask") is None:
                matched["yoloMask"] = detection_mask
            else:
                matched["yoloMask"] = np.logical_or(matched["yoloMask"], detection_mask)
    return clusters


def yolo_person_prior_masks(
    source: Image.Image,
    source_alpha_mask: Any,
    person_min_area: int,
    person_min_area_pct: float,
    device: str,
    confidence: float,
    max_regions: int,
) -> tuple[list[dict[str, Any]], dict[str, Any]]:
    import numpy as np

    report: dict[str, Any] = {
        "enabled": True,
        "status": "pending",
        "model": str(detail_model_path("body")),
        "rawDetections": 0,
        "acceptedDetections": 0,
        "rejectedDetections": 0,
        "mergedDetections": 0,
        "maskBackend": "sam-box",
        "minAreaPixels": person_min_area,
        "minAreaPct": person_min_area_pct,
        "confidence": confidence,
        "maxRegions": max_regions,
        "rejectionCounts": {},
    }
    model_path = detail_model_path("body")
    if not model_path.is_file():
        report["status"] = "model-missing"
        return [], report

    try:
        from ultralytics import YOLO
    except Exception as exc:  # noqa: BLE001
        report["status"] = "dependency-unavailable"
        report["message"] = f"Ultralytics is not installed: {exc}"
        return [], report

    try:
        model = YOLO(str(model_path))
        results = model.predict(
            source=source.convert("RGB"),
            conf=confidence,
            verbose=False,
            device=0 if device == "cuda" else "cpu",
            retina_masks=True,
        )
    except Exception as exc:  # noqa: BLE001
        report["status"] = "failed"
        report["message"] = str(exc)
        return [], report

    detections: list[dict[str, Any]] = []
    rejection_counts: dict[str, int] = {}

    def reject(reason: str) -> None:
        rejection_counts[reason] = rejection_counts.get(reason, 0) + 1
        report["rejectedDetections"] = int(report["rejectedDetections"]) + 1

    for result in results:
        boxes = getattr(result, "boxes", None)
        if boxes is None or boxes.xyxy is None:
            continue
        xyxy = boxes.xyxy.detach().cpu().tolist()
        confidences = boxes.conf.detach().cpu().tolist()
        classes = (
            boxes.cls.detach().cpu().tolist()
            if getattr(boxes, "cls", None) is not None
            else [None] * len(xyxy)
        )
        result_masks = getattr(result, "masks", None)
        mask_tensors = (
            result_masks.data.detach().cpu().numpy()
            if result_masks is not None and getattr(result_masks, "data", None) is not None
            else None
        )
        names = getattr(model, "names", {})
        for detection_index, (box, confidence_value, class_id) in enumerate(
            zip(xyxy, confidences, classes)
        ):
            report["rawDetections"] = int(report["rawDetections"]) + 1
            class_name = ""
            try:
                if isinstance(names, dict):
                    class_name = str(names.get(int(class_id), "")).strip().lower()
            except Exception:
                class_name = ""
            if class_name and class_name != "person":
                reject(f"class:{class_name}")
                continue

            clamped_box = clamp_box(tuple(box), source.width, source.height)
            if box_area(clamped_box) < person_min_area:
                reject("box-too-small")
                continue

            yolo_mask = None
            if mask_tensors is not None and detection_index < len(mask_tensors):
                mask_image = Image.fromarray(
                    (mask_tensors[detection_index] > 0.5).astype("uint8") * 255,
                    mode="L",
                )
                if mask_image.size != source.size:
                    mask_image = mask_image.resize(source.size, Image.Resampling.NEAREST)
                yolo_mask = np.logical_and(np.asarray(mask_image) > 0, source_alpha_mask)

            detections.append(
                {
                    "box": clamped_box,
                    "score": float(confidence_value),
                    "className": class_name or "person",
                    "detectionIndex": detection_index,
                    "yoloMask": yolo_mask,
                }
            )

    clusters = merged_person_detection_clusters(detections, source.width, source.height)
    clusters = sorted(clusters, key=lambda item: item["score"], reverse=True)[:max_regions]
    report["mergedDetections"] = len(clusters)
    report["mergedAwayDetections"] = max(0, len(detections) - len(clusters))

    if not clusters:
        report["rejectionCounts"] = rejection_counts
        report["status"] = "no-detections"
        return [], report

    person_sam_model = expand_model_path(
        os.environ.get("UNDERPAINT_PERSON_PRIOR_SAM_MODEL") or DEFAULT_SAM_MODEL
    )
    report["samModel"] = person_sam_model
    try:
        import torch
        from transformers import SamModel, SamProcessor

        processor = SamProcessor.from_pretrained(person_sam_model)
        model = SamModel.from_pretrained(person_sam_model).to(device)
        model.eval()
        box_prompts = [
            list(expand_box(cluster["box"], source.width, source.height))
            for cluster in clusters
        ]
        inputs = processor(source.convert("RGB"), input_boxes=[box_prompts], return_tensors="pt")
        inputs = {key: value.to(device) for key, value in inputs.items()}
        with torch.no_grad():
            outputs = model(**inputs)
        processed_masks = processor.image_processor.post_process_masks(
            outputs.pred_masks.cpu(),
            inputs["original_sizes"].cpu(),
            inputs["reshaped_input_sizes"].cpu(),
        )[0]
        iou_scores = outputs.iou_scores.detach().cpu()[0]
    except Exception as exc:  # noqa: BLE001
        report["status"] = "sam-box-failed"
        report["message"] = str(exc)
        processed_masks = None
        iou_scores = None
    finally:
        try:
            if "model" in locals():
                del model
            if torch.cuda.is_available():
                torch.cuda.empty_cache()
        except Exception:
            pass

    masks: list[dict[str, Any]] = []
    for cluster_index, cluster in enumerate(clusters):
        fallback_masks = [
            detection["yoloMask"]
            for detection in cluster["detections"]
            if detection.get("yoloMask") is not None
        ]
        yolo_union_mask = None
        if fallback_masks:
            yolo_union_mask = fallback_masks[0].copy()
            for fallback in fallback_masks[1:]:
                yolo_union_mask = np.logical_or(yolo_union_mask, fallback)
            yolo_union_mask = np.logical_and(yolo_union_mask, source_alpha_mask)

        selected_mask = None
        selected_score = float(cluster["score"])
        selected_option = -1
        selected_backend = "sam-box"
        if processed_masks is not None and iou_scores is not None:
            option_scores = iou_scores[cluster_index]
            option_indices = np.argsort(option_scores.numpy())[::-1]
            for option_index in option_indices:
                mask = processed_masks[cluster_index][int(option_index)].numpy().astype(bool)
                mask = np.logical_and(mask, source_alpha_mask)
                area = int(np.count_nonzero(mask))
                bounds = mask_bounds(mask)
                if area < person_min_area or bounds is None:
                    reject("sam-too-small")
                    continue
                if is_background_like_mask(mask, bounds, source.width, source.height):
                    reject("sam-background-like")
                    continue
                if is_sparse_sprawling_mask(mask, bounds, source.width, source.height):
                    reject("sam-sparse-sprawl")
                    continue
                selected_mask = mask
                selected_score = float(option_scores[int(option_index)].item())
                selected_option = int(option_index)
                break

        if selected_mask is not None and yolo_union_mask is not None:
            yolo_area = int(np.count_nonzero(yolo_union_mask))
            sam_area = int(np.count_nonzero(selected_mask))
            yolo_overlap = mask_overlap_of_smaller(selected_mask, yolo_union_mask)
            if (
                yolo_area >= person_min_area
                and (
                    yolo_overlap < 0.70
                    or sam_area < yolo_area * 0.65
                    or sam_area > yolo_area * 1.80
                )
            ):
                selected_mask = yolo_union_mask
                selected_backend = "yolo-mask-correction"
                report["correctedWithYoloMask"] = (
                    int(report.get("correctedWithYoloMask", 0)) + 1
                )

        if selected_mask is None:
            if yolo_union_mask is not None:
                selected_mask = yolo_union_mask
                selected_backend = "yolo-mask-fallback"
                report["fallbackDetections"] = int(report.get("fallbackDetections", 0)) + 1
            else:
                reject("no-valid-mask")
                continue

        selected_mask = np.logical_and(selected_mask, source_alpha_mask)
        area = int(np.count_nonzero(selected_mask))
        bounds = mask_bounds(selected_mask)
        if area < person_min_area or bounds is None:
            reject("too-small")
            continue
        if is_background_like_mask(selected_mask, bounds, source.width, source.height):
            reject("background-like")
            continue

        masks.append(
            {
                "mask": selected_mask,
                "area": area,
                "rawArea": area,
                "bounds": bounds,
                "rawBounds": bounds,
                "score": selected_score,
                "utility": object_mask_utility(
                    area, bounds, source.width, source.height, selected_score
                )
                + 0.45,
                "pointIndex": -1,
                "optionIndex": selected_option,
                "prior": "person-yolo-sam-box"
                if selected_backend == "sam-box"
                else selected_backend,
                "semanticName": "person",
                "groupId": "people",
                "groupLabel": "People",
                "className": "person",
                "boxPrompt": list(expand_box(cluster["box"], source.width, source.height)),
                "mergedDetectionCount": len(cluster["detections"]),
                "minAreaOverride": person_min_area,
            }
        )
        report["acceptedDetections"] = int(report["acceptedDetections"]) + 1

    report["rejectionCounts"] = rejection_counts
    report["status"] = "applied" if masks else "no-detections"
    release_cuda_memory()
    return masks, report


def object_detector_group(class_name: str) -> tuple[str, str]:
    normalized = class_name.strip().lower()
    if normalized in {
        "bicycle",
        "car",
        "motorcycle",
        "airplane",
        "bus",
        "train",
        "truck",
        "boat",
    }:
        return "vehicles", "Vehicles"
    if normalized in {
        "bird",
        "cat",
        "dog",
        "horse",
        "sheep",
        "cow",
        "elephant",
        "bear",
        "zebra",
        "giraffe",
    }:
        return "animals", "Animals"
    if normalized in {
        "chair",
        "couch",
        "potted plant",
        "bed",
        "dining table",
        "toilet",
        "bench",
    }:
        return "furniture-and-fixtures", "Furniture and Fixtures"
    if normalized in {
        "traffic light",
        "fire hydrant",
        "stop sign",
        "parking meter",
        "sign",
    }:
        return "signs-and-street-objects", "Signs and Street Objects"
    if normalized in {
        "backpack",
        "umbrella",
        "handbag",
        "tie",
        "suitcase",
        "sports ball",
        "kite",
        "baseball bat",
        "baseball glove",
        "skateboard",
        "surfboard",
        "tennis racket",
    }:
        return "held-and-portable-objects", "Held and Portable Objects"
    if normalized in {
        "bottle",
        "wine glass",
        "cup",
        "fork",
        "knife",
        "spoon",
        "bowl",
        "banana",
        "apple",
        "sandwich",
        "orange",
        "broccoli",
        "carrot",
        "hot dog",
        "pizza",
        "donut",
        "cake",
    }:
        return "food-and-table-objects", "Food and Table Objects"
    if normalized in {
        "tv",
        "laptop",
        "mouse",
        "remote",
        "keyboard",
        "cell phone",
        "microwave",
        "oven",
        "toaster",
        "sink",
        "refrigerator",
        "clock",
    }:
        return "devices-and-appliances", "Devices and Appliances"
    return "detected-objects", "Detected Objects"


def yolo_object_prior_masks(
    source: Image.Image,
    source_alpha_mask: Any,
    object_min_area: int,
    object_min_area_pct: float,
    device: str,
    confidence: float,
    max_regions: int,
    image_size: int,
    *,
    skip_people: bool,
) -> tuple[list[dict[str, Any]], dict[str, Any]]:
    import numpy as np

    model_path = object_detector_model_path()
    report: dict[str, Any] = {
        "enabled": True,
        "status": "pending",
        "model": str(model_path),
        "rawDetections": 0,
        "acceptedDetections": 0,
        "rejectedDetections": 0,
        "minAreaPixels": object_min_area,
        "minAreaPct": object_min_area_pct,
        "confidence": confidence,
        "maxRegions": max_regions,
        "imageSize": image_size,
        "skipPeople": skip_people,
        "rejectionCounts": {},
    }
    if not model_path.is_file():
        report["status"] = "model-missing"
        return [], report

    try:
        from ultralytics import YOLO
    except Exception as exc:  # noqa: BLE001
        report["status"] = "dependency-unavailable"
        report["message"] = f"Ultralytics is not installed: {exc}"
        return [], report

    try:
        model = YOLO(str(model_path))
        results = model.predict(
            source=source.convert("RGB"),
            conf=confidence,
            imgsz=image_size,
            verbose=False,
            device=0 if device == "cuda" else "cpu",
            retina_masks=True,
        )
    except Exception as exc:  # noqa: BLE001
        report["status"] = "failed"
        report["message"] = str(exc)
        return [], report

    masks: list[dict[str, Any]] = []
    rejection_counts: dict[str, int] = {}

    def reject(reason: str) -> None:
        rejection_counts[reason] = rejection_counts.get(reason, 0) + 1
        report["rejectedDetections"] = int(report["rejectedDetections"]) + 1

    names = getattr(model, "names", {})
    for result in results:
        boxes = getattr(result, "boxes", None)
        if boxes is None or boxes.xyxy is None:
            continue
        xyxy = boxes.xyxy.detach().cpu().tolist()
        confidences = boxes.conf.detach().cpu().tolist()
        classes = (
            boxes.cls.detach().cpu().tolist()
            if getattr(boxes, "cls", None) is not None
            else [None] * len(xyxy)
        )
        result_masks = getattr(result, "masks", None)
        mask_tensors = (
            result_masks.data.detach().cpu().numpy()
            if result_masks is not None and getattr(result_masks, "data", None) is not None
            else None
        )
        for detection_index, (box, confidence_value, class_id) in enumerate(
            zip(xyxy, confidences, classes)
        ):
            report["rawDetections"] = int(report["rawDetections"]) + 1
            class_name = ""
            try:
                if isinstance(names, dict):
                    class_name = str(names.get(int(class_id), "")).strip().lower()
            except Exception:
                class_name = ""
            if not class_name:
                reject("unknown-class")
                continue
            if skip_people and class_name == "person":
                reject("person-handled-by-person-prior")
                continue
            clamped_box = clamp_box(tuple(box), source.width, source.height)
            if box_area(clamped_box) < object_min_area:
                reject("box-too-small")
                continue
            if mask_tensors is None or detection_index >= len(mask_tensors):
                reject("mask-missing")
                continue
            mask_image = Image.fromarray(
                (mask_tensors[detection_index] > 0.5).astype("uint8") * 255,
                mode="L",
            )
            if mask_image.size != source.size:
                mask_image = mask_image.resize(source.size, Image.Resampling.NEAREST)
            mask = np.logical_and(np.asarray(mask_image) > 0, source_alpha_mask)
            area = int(np.count_nonzero(mask))
            if area < object_min_area:
                reject("mask-too-small")
                continue
            bounds = mask_bounds(mask)
            if bounds is None:
                reject("mask-empty")
                continue
            if is_background_like_mask(mask, bounds, source.width, source.height):
                reject("background-like")
                continue
            if is_sparse_sprawling_mask(mask, bounds, source.width, source.height):
                reject("sparse-sprawl")
                continue

            group_id, group_label = object_detector_group(class_name)
            masks.append(
                {
                    "mask": mask,
                    "area": area,
                    "rawArea": area,
                    "bounds": bounds,
                    "rawBounds": bounds,
                    "score": float(confidence_value),
                    "utility": object_mask_utility(
                        area, bounds, source.width, source.height, float(confidence_value)
                    )
                    + 0.36,
                    "pointIndex": -1,
                    "optionIndex": -1,
                    "prior": "object-yolo-seg",
                    "semanticName": class_name,
                    "groupId": group_id,
                    "groupLabel": group_label,
                    "className": class_name,
                    "boxPrompt": list(clamped_box),
                    "mergedDetectionCount": 1,
                    "minAreaOverride": object_min_area,
                    "detectorModel": str(model_path),
                }
            )

    masks.sort(key=lambda item: (item["utility"], item["score"], item["area"]), reverse=True)
    masks = masks[:max_regions]
    report["acceptedDetections"] = len(masks)
    report["rejectionCounts"] = rejection_counts
    report["classes"] = sorted({str(item["className"]) for item in masks})
    report["status"] = "applied" if masks else "no-detections"
    release_cuda_memory()
    return masks, report


def object_region_group(area_ratio: float) -> tuple[str, str]:
    if area_ratio >= 0.18:
        return "large-objects", "Large Objects"
    if area_ratio >= 0.045:
        return "object-parts", "Object Parts"
    return "small-parts", "Small Parts"


def write_object_decomposition_response(
    request: dict[str, Any],
    response_path: Path,
    request_id: str,
    job_dir: Path,
    started: float,
) -> int:
    debug_event("object-decomposition-start", {"requestId": request_id})
    source_path = asset_path(request, "source-image")
    if source_path is None:
        return fail(response_path, request_id, "source-image is required.")

    try:
        source = Image.open(source_path).convert("RGBA")
    except Exception as exc:  # noqa: BLE001
        return fail(response_path, request_id, f"Could not load source image: {exc}", 2)

    parameters = request.get("parameters", {})
    segmentation = normalized_segmentation_backend(parameters)
    segmentation_backend = segmentation["backend"]
    segmentation_model = segmentation["model"]

    try:
        import numpy as np
        import torch
    except Exception as exc:  # noqa: BLE001
        return fail(
            response_path,
            request_id,
            f"Object decomposition dependencies are not installed: {exc}",
            2,
        )

    requested_mask_count = int(parameters.get("maxMasks", parameters.get("maxRegions", 24)))
    max_masks = max(1, min(requested_mask_count, 200))
    min_area_pct = max(0.05, min(float(parameters.get("minRegionAreaPct", 0.12)), 20.0))
    min_area = int(source.width * source.height * (min_area_pct / 100.0))
    person_prior_enabled = bool(parameters.get("personPriorEnabled", True))
    person_prior_confidence = max(
        0.01, min(float(parameters.get("personPriorConfidence", 0.05)), 1.0)
    )
    person_prior_max_regions = max(
        1, min(int(parameters.get("personPriorMaxRegions", 64)), 128)
    )
    person_prior_min_area_pct = max(
        0.01, min(float(parameters.get("personPriorMinAreaPct", 0.05)), 10.0)
    )
    person_prior_min_area = max(
        1, int(source.width * source.height * (person_prior_min_area_pct / 100.0))
    )
    object_prior_enabled = bool(parameters.get("objectPriorEnabled", True))
    object_prior_confidence = max(
        0.01, min(float(parameters.get("objectPriorConfidence", 0.12)), 1.0)
    )
    object_prior_max_regions = max(
        1, min(int(parameters.get("objectPriorMaxRegions", 64)), 128)
    )
    object_prior_min_area_pct = max(
        0.01, min(float(parameters.get("objectPriorMinAreaPct", 0.03)), 10.0)
    )
    object_prior_min_area = max(
        1, int(source.width * source.height * (object_prior_min_area_pct / 100.0))
    )
    object_prior_image_size = max(
        640, min(int(parameters.get("objectPriorImageSize", 1280)), 2048)
    )
    sam_grid_fallback_enabled = bool(
        parameters.get("samGridFallbackEnabled", True)
    )
    raw_depth = str(parameters.get("decompositionDepth", "detailed")).strip().lower()
    decomposition_depth = raw_depth if raw_depth in {
        "clean",
        "balanced",
        "detailed",
        "exhaustive",
    } else "balanced"
    allow_cpu = os.environ.get("UNDERPAINT_AI_ALLOW_CPU") == "1"
    if torch.cuda.is_available():
        device = "cuda"
    elif allow_cpu:
        device = "cpu"
    else:
        return fail(
            response_path,
            request_id,
            "CUDA is not available for object decomposition. Reboot or repair "
            "the NVIDIA driver, or set UNDERPAINT_AI_ALLOW_CPU=1 for a very "
            "slow CPU test.",
        )

    rgb = source.convert("RGB")
    source_alpha_mask = np.asarray(source.getchannel("A")) > 0
    total_pixels = max(1, source.width * source.height)
    grid_size = decomposition_grid_size(decomposition_depth)

    rejection_counts = {
        "rawTooSmall": 0,
        "rawEmpty": 0,
        "rawBackgroundLike": 0,
        "rawSparseSprawl": 0,
        "cleanedTooSmall": 0,
        "cleanedEmpty": 0,
        "cleanedBackgroundLike": 0,
        "cleanedSparseSprawl": 0,
        "overlapRejected": 0,
        "duplicateRejected": 0,
        "personPriorDuplicateRejected": 0,
    }
    raw_masks: list[dict[str, Any]] = []
    person_prior_report: dict[str, Any] = {
        "enabled": person_prior_enabled,
        "status": "disabled" if not person_prior_enabled else "pending",
    }
    if person_prior_enabled:
        person_prior_masks, person_prior_report = yolo_person_prior_masks(
            source,
            source_alpha_mask,
            person_prior_min_area,
            person_prior_min_area_pct,
            device,
            person_prior_confidence,
            person_prior_max_regions,
        )
        raw_masks.extend(person_prior_masks)
        debug_event(
            "object-decomposition-person-prior",
            {
                "requestId": request_id,
                "report": person_prior_report,
            },
        )
    object_prior_report: dict[str, Any] = {
        "enabled": object_prior_enabled,
        "status": "disabled" if not object_prior_enabled else "pending",
    }
    if object_prior_enabled:
        object_prior_masks, object_prior_report = yolo_object_prior_masks(
            source,
            source_alpha_mask,
            object_prior_min_area,
            object_prior_min_area_pct,
            device,
            object_prior_confidence,
            object_prior_max_regions,
            object_prior_image_size,
            skip_people=person_prior_enabled,
        )
        raw_masks.extend(object_prior_masks)
        debug_event(
            "object-decomposition-object-prior",
            {
                "requestId": request_id,
                "report": object_prior_report,
            },
        )

    sam_grid_report: dict[str, Any] = {
        "enabled": sam_grid_fallback_enabled,
        "status": "disabled" if not sam_grid_fallback_enabled else "pending",
        "gridSize": grid_size,
    }
    should_run_sam_grid = sam_grid_fallback_enabled or (
        not raw_masks and not (person_prior_enabled or object_prior_enabled)
    )
    if should_run_sam_grid:
        point_batch: list[list[list[int]]] = []
        for gy in range(grid_size):
            y = round((gy + 0.5) * source.height / grid_size)
            for gx in range(grid_size):
                x = round((gx + 0.5) * source.width / grid_size)
                point_batch.append([[int(x), int(y)]])

        sam_grid_batch_size = max(
            1,
            min(
                int(
                    parameters.get(
                        "samGridBatchSize",
                        24 if segmentation_backend == "sam-hq" else 48,
                    )
                ),
                64,
            ),
        )
        sam_grid_report["batchSize"] = sam_grid_batch_size
        grid_raw_mask_count = 0
        processed_point_count = 0
        try:
            release_cuda_memory()
            processor, model = load_segmentation_model(segmentation, device)
            model.eval()
            point_offset = 0
            active_batch_size = sam_grid_batch_size
            oom_retries = 0
            while point_offset < len(point_batch):
                point_chunk = point_batch[point_offset:point_offset + active_batch_size]
                try:
                    inputs = processor(
                        rgb,
                        input_points=[point_chunk],
                        return_tensors="pt",
                    )
                    inputs = {key: value.to(device) for key, value in inputs.items()}
                    with torch.no_grad():
                        outputs = model(**inputs)
                    processed_masks = processor.image_processor.post_process_masks(
                        outputs.pred_masks.cpu(),
                        inputs["original_sizes"].cpu(),
                        inputs["reshaped_input_sizes"].cpu(),
                    )[0]
                    iou_scores = outputs.iou_scores.detach().cpu()[0]
                except RuntimeError as exc:
                    if "out of memory" in str(exc).lower() and active_batch_size > 1:
                        oom_retries += 1
                        active_batch_size = max(1, active_batch_size // 2)
                        sam_grid_report["oomRetries"] = oom_retries
                        sam_grid_report["batchSize"] = active_batch_size
                        release_cuda_memory()
                        continue
                    raise

                for local_point_index in range(processed_masks.shape[0]):
                    point_index = point_offset + local_point_index
                    for option_index in range(processed_masks.shape[1]):
                        score = float(iou_scores[local_point_index][option_index].item())
                        mask = (
                            processed_masks[local_point_index][option_index]
                            .numpy()
                            .astype(bool)
                        )
                        raw_area = int(np.count_nonzero(mask))
                        if raw_area < min_area:
                            rejection_counts["rawTooSmall"] += 1
                            continue
                        raw_bounds = mask_bounds(mask)
                        if raw_bounds is None:
                            rejection_counts["rawEmpty"] += 1
                            continue
                        if is_background_like_mask(
                            mask, raw_bounds, source.width, source.height
                        ):
                            rejection_counts["rawBackgroundLike"] += 1
                            continue
                        if is_sparse_sprawling_mask(
                            mask, raw_bounds, source.width, source.height
                        ):
                            rejection_counts["rawSparseSprawl"] += 1
                            continue

                        raw_masks.append(
                            {
                                "mask": mask,
                                "area": raw_area,
                                "rawArea": raw_area,
                                "bounds": raw_bounds,
                                "rawBounds": raw_bounds,
                                "score": score,
                                "utility": object_mask_utility(
                                    raw_area,
                                    raw_bounds,
                                    source.width,
                                    source.height,
                                    score,
                                    "sam-grid",
                                ),
                                "pointIndex": point_index,
                                "optionIndex": option_index,
                                "prior": "sam-grid",
                            }
                        )
                        grid_raw_mask_count += 1
                processed_point_count += len(point_chunk)
                point_offset += len(point_chunk)
                del inputs, outputs, processed_masks, iou_scores
                release_cuda_memory()
            sam_grid_report["status"] = "applied"
            sam_grid_report["pointCount"] = len(point_batch)
        except Exception as exc:  # noqa: BLE001
            sam_grid_report["status"] = (
                "partial-failed" if processed_point_count else "failed"
            )
            sam_grid_report["message"] = str(exc)
            if not raw_masks:
                return fail(
                    response_path,
                    request_id,
                    f"{segmentation_backend} object decomposition failed with "
                    f"{segmentation_model}: {exc}",
                    2,
                )
        finally:
            try:
                if "model" in locals():
                    try:
                        model.to("cpu")
                    except Exception:
                        pass
                    del model
                if "processor" in locals():
                    del processor
            except Exception:
                pass
            release_cuda_memory()
        sam_grid_report["processedPointCount"] = processed_point_count
        sam_grid_report["rawMaskCount"] = grid_raw_mask_count

    raw_masks.sort(key=lambda item: (item["utility"], item["score"], item["area"]), reverse=True)
    preselection_raw_mask_count = len(raw_masks)
    raw_mask_preselection_limit = max(max_masks, 96)
    if len(raw_masks) > raw_mask_preselection_limit:
        raw_masks = raw_masks[:raw_mask_preselection_limit]
    selected_masks: list[dict[str, Any]] = []
    selected_union = np.zeros(source_alpha_mask.shape, dtype=bool)
    for candidate in raw_masks:
        raw_area = candidate["rawArea"]
        candidate_min_area = int(candidate.get("minAreaOverride", min_area))
        min_component_area = max(96, int(total_pixels * 0.0004), int(raw_area * 0.006))
        mask = clean_object_mask(candidate["mask"], source_alpha_mask, min_component_area)
        area = int(np.count_nonzero(mask))
        if area < candidate_min_area:
            rejection_counts["cleanedTooSmall"] += 1
            continue
        bounds = mask_bounds(mask)
        if bounds is None:
            rejection_counts["cleanedEmpty"] += 1
            continue
        if is_background_like_mask(mask, bounds, source.width, source.height):
            rejection_counts["cleanedBackgroundLike"] += 1
            continue
        if is_sparse_sprawling_mask(mask, bounds, source.width, source.height):
            rejection_counts["cleanedSparseSprawl"] += 1
            continue
        overlap_with_selected = float(np.logical_and(mask, selected_union).sum()) / float(area)
        # Decomposition should favor independently movable pieces. Broad later
        # masks that mostly cover already-selected objects create muddy
        # duplicate layers and confuse the repaired base plate.
        if overlap_with_selected >= 0.45:
            rejection_counts["overlapRejected"] += 1
            continue
        candidate = {
            **candidate,
            "mask": mask,
            "area": area,
            "bounds": bounds,
            "utility": object_mask_utility(
                area,
                bounds,
                source.width,
                source.height,
                candidate["score"],
                str(candidate.get("prior", "")),
            ),
            "overlapWithSelected": overlap_with_selected,
            "minComponentArea": min_component_area,
        }
        duplicate = False
        for selected in selected_masks:
            if mask_iou(candidate["mask"], selected["mask"]) >= 0.82:
                rejection_counts["duplicateRejected"] += 1
                duplicate = True
                break
            if (
                str(candidate.get("prior", "")).startswith("person-yolo")
                and str(selected.get("prior", "")).startswith("person-yolo")
                and mask_overlap_of_smaller(candidate["mask"], selected["mask"]) >= 0.22
            ):
                rejection_counts["personPriorDuplicateRejected"] += 1
                duplicate = True
                break
        if duplicate:
            continue
        selected_masks.append(candidate)
        selected_union = np.logical_or(selected_union, candidate["mask"])
        if len(selected_masks) >= max_masks:
            break

    candidates: list[dict[str, Any]] = []
    region_set_id = f"object-decomposition-{decomposition_depth}"
    region_set_label = f"Object Decomposition - {decomposition_depth_label(decomposition_depth)}"
    if selected_masks:
        union_mask = np.zeros(source_alpha_mask.shape, dtype=bool)
        for item in selected_masks:
            union_mask = np.logical_or(union_mask, item["mask"])
        base_overlap_px = max(
            0,
            min(
                int(parameters.get("baseRemainderOverlapPx", 0)),
                32,
            ),
        )
        # Keep the visible base plate tight by default. Expanding this hole
        # creates a transparent moat around every extracted object once the
        # source layer is hidden. Repair masks expand separately in the editor.
        base_cutout_mask = (
            dilate_boolean_mask(union_mask, base_overlap_px)
            if base_overlap_px > 0
            else union_mask
        )
        remainder_mask = np.logical_and(source_alpha_mask, ~base_cutout_mask)
        remainder_area = int(np.count_nonzero(remainder_mask))
        if remainder_area > 0:
            remainder_mask_image = Image.fromarray(
                (remainder_mask.astype("uint8") * 255), mode="L"
            )
            remainder_layer = source_with_alpha(source, remainder_mask_image)
            remainder_image_path = job_dir / "base-remainder.png"
            remainder_mask_path = job_dir / "base-remainder-mask.png"
            remainder_layer.save(remainder_image_path)
            remainder_mask_image.save(remainder_mask_path)
            candidates.append(
                {
                    "id": "base-remainder",
                    "label": "Base Remainder",
                    "imagePath": str(remainder_image_path),
                    "maskPath": str(remainder_mask_path),
                    "metadata": {
                        "operation": "object-decomposition",
                        "modelRole": "object-decomposition",
                        "model": segmentation_model,
                        "modelId": segmentation["modelId"],
                        "backend": segmentation_backend,
                        "segmentationBackend": segmentation_backend,
                        "segmentationModel": segmentation_model,
                        "regionSchema": "underpaint.segmentation-region.v1",
                        "regionSetId": region_set_id,
                        "regionSetLabel": region_set_label,
                        "groupId": "base-remainder",
                        "groupLabel": "Base Remainder",
                        "regionIndex": 0,
                        "regionCount": len(selected_masks) + 1,
                        "requestedRegionCount": requested_mask_count,
                        "minRegionAreaPct": min_area_pct,
                        "decompositionDepth": decomposition_depth,
                        "maskRole": "base-remainder",
                        "baseOverlapPx": base_overlap_px,
                        "areaPixels": remainder_area,
                        "areaPx": remainder_area,
                        "areaRatio": remainder_area / total_pixels,
                        "bounds": list(mask_bounds(remainder_mask) or (0, 0, 0, 0)),
                        "bbox": list(mask_bounds(remainder_mask) or (0, 0, 0, 0)),
                        "labelStatus": "manual",
                        "helperStatus": "not-needed",
                    },
                }
            )
    for index, item in enumerate(selected_masks):
        mask = item["mask"]
        edge_feather = max(0.25, min(1.0, min(source.width, source.height) / 1536.0))
        mask_image = soft_object_mask(mask, edge_feather)
        layer = source_with_alpha(source, mask_image)
        candidate_id = f"object-{index + 1}"
        raw_label = str(item.get("semanticName") or "").strip()
        label = (
            f"{raw_label.title()} {index + 1}"
            if raw_label
            else f"Object {index + 1}"
        )
        area_ratio = item["area"] / total_pixels
        default_group_id, default_group_label = object_region_group(area_ratio)
        group_id = str(item.get("groupId") or default_group_id)
        group_label = str(item.get("groupLabel") or default_group_label)
        image_path = job_dir / f"{candidate_id}.png"
        mask_path = job_dir / f"{candidate_id}-mask.png"
        layer.save(image_path)
        mask_image.save(mask_path)
        emit_progress(
            {
                "type": "candidate",
                "id": candidate_id,
                "candidate": index + 1,
                "label": label,
                "groupLabel": group_label,
                "imagePath": str(image_path),
            }
        )
        candidates.append(
            {
                "id": candidate_id,
                "label": label,
                "imagePath": str(image_path),
                "maskPath": str(mask_path),
                "metadata": {
                    "operation": "object-decomposition",
                    "modelRole": "object-decomposition",
                    "model": segmentation_model,
                    "modelId": segmentation["modelId"],
                    "backend": segmentation_backend,
                    "segmentationBackend": segmentation_backend,
                    "segmentationModel": segmentation_model,
                    "regionSchema": "underpaint.segmentation-region.v1",
                    "regionSetId": region_set_id,
                    "regionSetLabel": region_set_label,
                    "groupId": group_id,
                    "groupLabel": group_label,
                    "regionIndex": index,
                    "regionCount": len(selected_masks),
                    "requestedRegionCount": requested_mask_count,
                    "minRegionAreaPct": min_area_pct,
                    "decompositionDepth": decomposition_depth,
                    "maskRole": "extracted-object",
                    "samPredictedIou": item["score"],
                    "predictedIou": item["score"],
                    "maskPrior": item.get("prior", "sam-grid"),
                    "className": item.get("className", ""),
                    "boxPrompt": item.get("boxPrompt", []),
                    "detectorModel": item.get("detectorModel", ""),
                    "mergedDetectionCount": item.get("mergedDetectionCount", 1),
                    "maskUtility": item["utility"],
                    "samPointIndex": item["pointIndex"],
                    "samOptionIndex": item["optionIndex"],
                    "overlapWithSelected": item["overlapWithSelected"],
                    "areaPixels": item["area"],
                    "areaPx": item["area"],
                    "rawAreaPixels": item["rawArea"],
                    "areaRatio": area_ratio,
                    "bounds": list(item["bounds"]),
                    "bbox": list(item["bounds"]),
                    "rawBounds": list(item["rawBounds"]),
                    "edgeFeatherPx": edge_feather,
                    "minComponentAreaPixels": item["minComponentArea"],
                    "labelStatus": "pending",
                    "helperStatus": "pending",
                },
            }
        )

    elapsed_ms = int((time.monotonic() - started) * 1000)
    debug_event(
        "object-decomposition-complete",
        {
            "requestId": request_id,
            "candidateCount": len(candidates),
            "rawMaskCount": len(raw_masks),
            "preselectionRawMaskCount": preselection_raw_mask_count,
            "rejectionCounts": rejection_counts,
            "elapsedMsec": elapsed_ms,
            "model": segmentation_model,
            "backend": segmentation_backend,
        },
    )
    write_json(
        response_path,
        response(
            request_id,
            "succeeded",
            f"Generated {len(candidates)} object mask layer(s).",
            candidates=candidates,
            diagnostics={
                "elapsedMsec": elapsed_ms,
                "inputWidth": source.width,
                "inputHeight": source.height,
                "requestedRegionCount": requested_mask_count,
                "regionCount": len(candidates),
                "rawMaskCount": len(raw_masks),
                "preselectionRawMaskCount": preselection_raw_mask_count,
                "rawMaskPreselectionLimit": raw_mask_preselection_limit,
                "rejectedMaskCount": sum(rejection_counts.values()),
                "rejectionCounts": rejection_counts,
                "personPrior": person_prior_report,
                "objectPrior": object_prior_report,
                "samGridFallback": sam_grid_report,
                "decompositionDepth": decomposition_depth,
                "regionSetId": region_set_id,
                "regionSetLabel": region_set_label,
                "model": segmentation_model,
                "modelId": segmentation["modelId"],
                "backend": segmentation_backend,
                "segmentationBackend": segmentation_backend,
                "gridSize": grid_size,
                **debug_diagnostics(),
            },
            provenance={
                "backend": "python-worker",
                "schema": SCHEMA,
                "model": segmentation_model,
                "modelId": segmentation["modelId"],
                "segmentationBackend": segmentation_backend,
            },
        ),
    )
    return 0


def foreground_mask_with_sam(
    source: Image.Image,
    model_id: str,
    decomposition_depth: str,
) -> tuple[Any, dict[str, Any]]:
    import numpy as np
    import torch
    from transformers import SamModel, SamProcessor

    allow_cpu = os.environ.get("UNDERPAINT_AI_ALLOW_CPU") == "1"
    if torch.cuda.is_available():
        device = "cuda"
    elif allow_cpu:
        device = "cpu"
    else:
        raise RuntimeError(
            "CUDA is not available for SAM background removal. Reboot or repair "
            "the NVIDIA driver, or set UNDERPAINT_AI_ALLOW_CPU=1 for a very slow CPU test."
        )

    rgb = source.convert("RGB")
    source_alpha_mask = np.asarray(source.getchannel("A")) > 0
    total_pixels = max(1, source.width * source.height)
    grid_size = decomposition_grid_size(decomposition_depth)
    point_batch: list[list[list[int]]] = []
    for gy in range(grid_size):
        y = round((gy + 0.5) * source.height / grid_size)
        for gx in range(grid_size):
            x = round((gx + 0.5) * source.width / grid_size)
            point_batch.append([[int(x), int(y)]])

    model = None
    try:
        processor = SamProcessor.from_pretrained(model_id)
        model = SamModel.from_pretrained(model_id).to(device)
        model.eval()
        inputs = processor(rgb, input_points=[point_batch], return_tensors="pt")
        inputs = {key: value.to(device) for key, value in inputs.items()}
        with torch.no_grad():
            outputs = model(**inputs)
        processed_masks = processor.image_processor.post_process_masks(
            outputs.pred_masks.cpu(),
            inputs["original_sizes"].cpu(),
            inputs["reshaped_input_sizes"].cpu(),
        )[0]
        iou_scores = outputs.iou_scores.detach().cpu()[0]
    finally:
        try:
            if model is not None:
                del model
            if torch.cuda.is_available():
                torch.cuda.empty_cache()
        except Exception:
            pass

    raw_masks: list[dict[str, Any]] = []
    min_area = int(total_pixels * 0.01)
    for point_index in range(processed_masks.shape[0]):
        for option_index in range(processed_masks.shape[1]):
            score = float(iou_scores[point_index][option_index].item())
            mask = processed_masks[point_index][option_index].numpy().astype(bool)
            raw_area = int(np.count_nonzero(mask))
            if raw_area < min_area:
                continue
            bounds = mask_bounds(mask)
            if bounds is None:
                continue
            if is_background_like_mask(mask, bounds, source.width, source.height):
                continue
            raw_masks.append(
                {
                    "mask": mask,
                    "area": raw_area,
                    "score": score,
                    "utility": object_mask_utility(
                        raw_area, bounds, source.width, source.height, score
                    ),
                }
            )

    raw_masks.sort(key=lambda item: (item["utility"], item["score"], item["area"]), reverse=True)
    selected_masks: list[Any] = []
    selected_union = np.zeros(source_alpha_mask.shape, dtype=bool)
    for candidate in raw_masks:
        raw_area = int(candidate["area"])
        min_component_area = max(96, int(total_pixels * 0.0004), int(raw_area * 0.006))
        mask = clean_object_mask(candidate["mask"], source_alpha_mask, min_component_area)
        area = int(np.count_nonzero(mask))
        if area < min_area:
            continue
        bounds = mask_bounds(mask)
        if bounds is None:
            continue
        if is_background_like_mask(mask, bounds, source.width, source.height):
            continue
        overlap = float(np.logical_and(mask, selected_union).sum()) / float(area)
        if overlap >= 0.65:
            continue
        selected_masks.append(mask)
        selected_union = np.logical_or(selected_union, mask)
        if len(selected_masks) >= 12:
            break

    if not selected_masks:
        raise RuntimeError("SAM did not find a usable foreground mask.")

    return selected_union, {
        "backend": "sam-foreground-union",
        "model": model_id,
        "device": device,
        "gridSize": grid_size,
        "rawMaskCount": len(raw_masks),
        "selectedMaskCount": len(selected_masks),
    }


def write_background_removal_response(
    request: dict[str, Any],
    response_path: Path,
    request_id: str,
    job_dir: Path,
    started: float,
) -> int:
    debug_event("background-removal-start", {"requestId": request_id})
    source_path = asset_path(request, "source-image")
    if source_path is None:
        return fail(response_path, request_id, "source-image is required.")

    try:
        source = Image.open(source_path).convert("RGBA")
    except Exception as exc:  # noqa: BLE001
        return fail(response_path, request_id, f"Could not load source image: {exc}", 2)

    parameters = request.get("parameters", {})
    model_id = str(
        parameters.get("model")
        or os.environ.get("UNDERPAINT_SAM_MODEL")
        or "facebook/sam-vit-base"
    )
    raw_depth = str(parameters.get("decompositionDepth", "clean")).strip().lower()
    decomposition_depth = raw_depth if raw_depth in {
        "clean",
        "balanced",
        "detailed",
        "exhaustive",
    } else "clean"

    backend_report: dict[str, Any]
    try:
        try:
            from rembg import remove

            cutout = remove(source).convert("RGBA")
            matte = cutout.getchannel("A")
            backend_report = {"backend": "rembg", "model": "rembg-default"}
        except Exception as rembg_exc:  # noqa: BLE001
            debug_event("background-removal-rembg-fallback", {"message": str(rembg_exc)})
            mask, backend_report = foreground_mask_with_sam(
                source, model_id, decomposition_depth
            )
            matte = soft_object_mask(mask, max(1.0, min(source.size) / 512.0))
            cutout = source_with_alpha(source, matte)
    except Exception as exc:  # noqa: BLE001
        return fail(
            response_path,
            request_id,
            f"Background removal failed: {exc}",
            2,
        )

    image_path = job_dir / "background-removed.png"
    mask_path = job_dir / "foreground-matte.png"
    cutout.save(image_path)
    matte.save(mask_path)
    emit_progress(
        {
            "type": "candidate",
            "id": "background-removed",
            "candidate": 1,
            "label": "Background Removed",
            "imagePath": str(image_path),
        }
    )

    elapsed_ms = int((time.monotonic() - started) * 1000)
    candidate = {
        "id": "background-removed",
        "label": "Background Removed",
        "imagePath": str(image_path),
        "maskPath": str(mask_path),
        "metadata": {
            "operation": "background-removal",
            "modelRole": "background-removal",
            "model": backend_report.get("model", model_id),
            "maskRole": "foreground-matte",
            "groupId": "background-removal",
            "groupLabel": "Background Removal",
        },
    }
    write_json(
        response_path,
        response(
            request_id,
            "succeeded",
            "Removed background and generated a foreground cutout.",
            candidates=[candidate],
            diagnostics={
                "elapsedMsec": elapsed_ms,
                "inputWidth": source.width,
                "inputHeight": source.height,
                **backend_report,
                **debug_diagnostics(),
            },
            provenance={
                "backend": "python-worker",
                "schema": SCHEMA,
                "model": backend_report.get("model", model_id),
            },
        ),
    )
    return 0


def write_scene_separation_response(
    request: dict[str, Any],
    response_path: Path,
    request_id: str,
    job_dir: Path,
    started: float,
) -> int:
    debug_event("scene-separation-start", {"requestId": request_id})
    source_path = asset_path(request, "source-image")
    if source_path is None:
        return fail(response_path, request_id, "source-image is required.")

    try:
        source = Image.open(source_path).convert("RGBA")
    except Exception as exc:  # noqa: BLE001
        return fail(response_path, request_id, f"Could not load source image: {exc}", 2)

    parameters = request.get("parameters", {})
    requested_region_count = int(parameters.get("maxMasks", parameters.get("maxRegions", 5)))
    region_count = max(2, min(requested_region_count, 32))
    min_region_area_pct = max(1, min(int(parameters.get("minRegionAreaPct", 3)), 20))
    raw_depth = str(parameters.get("decompositionDepth", "balanced")).strip().lower()
    decomposition_depth = raw_depth if raw_depth in {
        "clean",
        "balanced",
        "detailed",
        "exhaustive",
    } else "balanced"
    group_repeated_regions = bool(parameters.get("groupRepeatedRegions", True))
    region_set_id = f"region-set-{decomposition_depth}"
    region_set_label = f"Region Set - {decomposition_depth_label(decomposition_depth)}"
    luma = source.convert("L")
    candidates: list[dict[str, Any]] = []

    for index in range(region_count):
        low = (index * 256) // region_count
        high = 256 if index == region_count - 1 else ((index + 1) * 256) // region_count
        mask = luma.point(
            lambda value, low=low, high=high: 255 if low <= value < high else 0
        )
        layer = Image.new("RGBA", source.size, (0, 0, 0, 0))
        layer.paste(source, (0, 0), mask)

        candidate_id = f"region-{index + 1}"
        label = region_label(index, region_count)
        group_id, group_label = placeholder_region_group(
            label, index, group_repeated_regions
        )
        image_path = job_dir / f"{candidate_id}.png"
        mask_path = job_dir / f"{candidate_id}-mask.png"
        layer.save(image_path)
        mask.save(mask_path)
        emit_progress(
            {
                "type": "candidate",
                "id": candidate_id,
                "candidate": index + 1,
                "label": label,
                "groupLabel": group_label,
                "imagePath": str(image_path),
            }
        )
        candidates.append(
            {
                "id": candidate_id,
                "label": label,
                "imagePath": str(image_path),
                "maskPath": str(mask_path),
                "metadata": {
                    "operation": "scene-separation",
                    "placeholder": True,
                    "modelRole": "color-separation",
                    "model": "placeholder-luma-regions",
                    "regionSetId": region_set_id,
                    "regionSetLabel": region_set_label,
                    "groupId": group_id,
                    "groupLabel": group_label,
                    "regionIndex": index,
                    "regionCount": region_count,
                    "requestedRegionCount": requested_region_count,
                    "minRegionAreaPct": min_region_area_pct,
                    "decompositionDepth": decomposition_depth,
                    "groupRepeatedRegions": group_repeated_regions,
                    "labelStatus": "placeholder",
                    "helperStatus": "pending",
                    "maskRole": "extracted-region",
                    "promptPhrase": label.lower(),
                },
            }
        )

    elapsed_ms = int((time.monotonic() - started) * 1000)
    debug_event(
        "scene-separation-complete",
        {
            "requestId": request_id,
            "candidateCount": len(candidates),
            "elapsedMsec": elapsed_ms,
            "inputWidth": source.width,
            "inputHeight": source.height,
        },
    )
    write_json(
        response_path,
        response(
            request_id,
            "succeeded",
            f"Generated {len(candidates)} color separation layer(s).",
            candidates=candidates,
            diagnostics={
                "elapsedMsec": elapsed_ms,
                "inputWidth": source.width,
                "inputHeight": source.height,
                "requestedRegionCount": requested_region_count,
                "regionCount": region_count,
                "decompositionDepth": decomposition_depth,
                "groupRepeatedRegions": group_repeated_regions,
                "regionSetId": region_set_id,
                "regionSetLabel": region_set_label,
                **debug_diagnostics(),
            },
            provenance={
                "backend": "python-worker",
                "schema": SCHEMA,
                "model": "placeholder-luma-regions",
            },
        ),
    )
    return 0


def load_mask(path: Path, size: tuple[int, int]) -> Image.Image:
    raw_mask = Image.open(path)
    mask = raw_mask.convert("RGBA")
    if mask.size != size:
        mask = mask.resize(size, Image.Resampling.LANCZOS)

    alpha = mask.getchannel("A")
    luma = mask.convert("L")
    has_source_alpha = "A" in raw_mask.getbands()
    if has_source_alpha and (
        alpha.getextrema() != (255, 255) or luma.getextrema() == (0, 0)
    ):
        mask = alpha
    else:
        mask = luma

    # Diffusers expects white pixels to be repainted.
    return mask


def round_up(value: int, multiple: int) -> int:
    return ((value + multiple - 1) // multiple) * multiple


def target_size_for_edge(size: tuple[int, int], target_edge: int) -> tuple[int, int]:
    width, height = size
    longest = max(width, height)
    if target_edge <= 0 or longest == target_edge:
        return size
    scale = target_edge / longest
    return (max(8, round(width * scale)), max(8, round(height * scale)))


def resize_for_render(
    source: Image.Image, mask: Image.Image, target_edge: int
) -> tuple[Image.Image, Image.Image, tuple[int, int]]:
    render_size = target_size_for_edge(source.size, target_edge)
    if render_size == source.size:
        return source, mask, render_size
    return (
        source.resize(render_size, Image.Resampling.LANCZOS),
        mask.resize(render_size, Image.Resampling.LANCZOS),
        render_size,
    )


def apply_alpha_mask(
    image: Image.Image, mask: Image.Image, edge_feather_px: int
) -> Image.Image:
    alpha = mask.convert("L")
    if edge_feather_px > 0:
        alpha = alpha.filter(ImageFilter.GaussianBlur(radius=edge_feather_px))
    rgba = image.convert("RGBA")
    rgba.putalpha(alpha)
    return rgba


def resize_to_max_edge(image: Image.Image, max_edge: int) -> Image.Image:
    if max_edge <= 0:
        return image
    width, height = image.size
    longest = max(width, height)
    if longest <= max_edge:
        return image
    scale = max_edge / longest
    return image.resize(
        (max(1, round(width * scale)), max(1, round(height * scale))),
        Image.Resampling.LANCZOS,
    )


def decode_latents_to_image(pipe: Any, torch: Any, latents: Any) -> Image.Image:
    latents = latents[:1].detach()
    needs_upcasting = pipe.vae.dtype == torch.float16 and pipe.vae.config.force_upcast
    if needs_upcasting:
        pipe.upcast_vae()
        latents = latents.to(next(iter(pipe.vae.post_quant_conv.parameters())).dtype)
    elif latents.dtype != pipe.vae.dtype and torch.backends.mps.is_available():
        pipe.vae = pipe.vae.to(latents.dtype)

    has_latents_mean = (
        hasattr(pipe.vae.config, "latents_mean")
        and pipe.vae.config.latents_mean is not None
    )
    has_latents_std = (
        hasattr(pipe.vae.config, "latents_std")
        and pipe.vae.config.latents_std is not None
    )
    if has_latents_mean and has_latents_std:
        latents_mean = (
            torch.tensor(pipe.vae.config.latents_mean)
            .view(1, 4, 1, 1)
            .to(latents.device, latents.dtype)
        )
        latents_std = (
            torch.tensor(pipe.vae.config.latents_std)
            .view(1, 4, 1, 1)
            .to(latents.device, latents.dtype)
        )
        latents = latents * latents_std / pipe.vae.config.scaling_factor + latents_mean
    else:
        latents = latents / pipe.vae.config.scaling_factor

    with torch.no_grad():
        image = pipe.vae.decode(latents, return_dict=False)[0]
    if needs_upcasting:
        pipe.vae.to(dtype=torch.float16)
    return pipe.image_processor.postprocess(image, output_type="pil")[0]


def pad_source_to_multiple(
    image: Image.Image, multiple: int
) -> tuple[Image.Image, tuple[int, int]]:
    original_size = image.size
    target_size = (
        round_up(original_size[0], multiple),
        round_up(original_size[1], multiple),
    )
    if target_size == original_size:
        return image, original_size

    padded = Image.new(image.mode, target_size)
    padded.paste(image, (0, 0))

    width, height = original_size
    target_width, target_height = target_size
    if target_width > width:
        right_edge = image.crop((width - 1, 0, width, height))
        padded.paste(
            right_edge.resize((target_width - width, height), Image.Resampling.NEAREST),
            (width, 0),
        )
    if target_height > height:
        bottom_edge = padded.crop((0, height - 1, target_width, height))
        padded.paste(
            bottom_edge.resize(
                (target_width, target_height - height), Image.Resampling.NEAREST
            ),
            (0, height),
        )
    return padded, original_size


def pad_mask_to_size(mask: Image.Image, size: tuple[int, int]) -> Image.Image:
    if mask.size == size:
        return mask
    padded = Image.new("L", size, 0)
    padded.paste(mask, (0, 0))
    return padded


def detail_predict_device(worker_device: str) -> str | int:
    configured = os.environ.get("UNDERPAINT_DETAIL_DETECTOR_DEVICE", "cpu").strip()
    if configured:
        return 0 if configured == "cuda" else configured
    return 0 if worker_device == "cuda" else "cpu"


def mask_for_box(
    size: tuple[int, int],
    box: tuple[float, float, float, float],
    padding: int,
) -> Image.Image:
    width, height = size
    left, top, right, bottom = box
    left = max(0, math.floor(left - padding))
    top = max(0, math.floor(top - padding))
    right = min(width, math.ceil(right + padding))
    bottom = min(height, math.ceil(bottom + padding))
    mask = Image.new("L", size, 0)
    if right <= left or bottom <= top:
        return mask
    ImageDraw.Draw(mask).rectangle((left, top, right, bottom), fill=255)
    return mask


def expand_box_to_min_edge(
    box: tuple[int, int, int, int],
    image_size: tuple[int, int],
    min_edge: int,
) -> tuple[int, int, int, int]:
    left, top, right, bottom = box
    width, height = image_size
    box_width = right - left
    box_height = bottom - top
    target_width = max(box_width, min_edge)
    target_height = max(box_height, min_edge)
    center_x = (left + right) / 2.0
    center_y = (top + bottom) / 2.0
    left = math.floor(center_x - target_width / 2.0)
    top = math.floor(center_y - target_height / 2.0)
    right = left + target_width
    bottom = top + target_height

    if left < 0:
        right -= left
        left = 0
    if top < 0:
        bottom -= top
        top = 0
    if right > width:
        left -= right - width
        right = width
    if bottom > height:
        top -= bottom - height
        bottom = height
    return (max(0, left), max(0, top), min(width, right), min(height, bottom))


def detail_crop_region(
    region_mask: Image.Image,
    image_size: tuple[int, int],
    min_crop_edge: int,
) -> tuple[int, int, int, int] | None:
    bbox = region_mask.getbbox()
    if bbox is None:
        return None
    return expand_box_to_min_edge(bbox, image_size, min_crop_edge)


def detail_render_size(
    crop_size: tuple[int, int],
    target_edge: int,
) -> tuple[int, int]:
    width, height = crop_size
    longest = max(width, height)
    if longest <= 0:
        return (8, 8)
    scale = target_edge / longest
    return (
        max(8, round_up(round(width * scale), 8)),
        max(8, round_up(round(height * scale), 8)),
    )


DETAIL_EXPECTED_CLASSES = {
    "face": {"face"},
    "body": {"person"},
    "hands": {"hand", "hands"},
}


def detail_detector_class_name(detector: dict[str, Any], class_id: Any) -> str:
    try:
        names = getattr(detector["model"], "names", {})
        name = names.get(int(class_id), "") if isinstance(names, dict) else ""
        return str(name).strip().lower()
    except Exception:
        return ""


def detail_detection_rejection_reason(
    region: str,
    class_name: str,
    box: list[float],
    image_size: tuple[int, int],
) -> str | None:
    expected_classes = DETAIL_EXPECTED_CLASSES.get(region, set())
    if expected_classes and class_name and class_name not in expected_classes:
        return f"class:{class_name or 'unknown'}"

    left, top, right, bottom = box
    width = max(0.0, right - left)
    height = max(0.0, bottom - top)
    image_width, image_height = image_size
    image_area = max(1.0, float(image_width * image_height))
    area_ratio = (width * height) / image_area
    shortest_image_edge = max(1.0, float(min(image_width, image_height)))
    min_side = max(8.0, shortest_image_edge * 0.012)
    if width < min_side or height < min_side:
        return "too-small"

    aspect = width / height if height else 0.0
    if region == "face":
        if aspect < 0.35 or aspect > 2.25:
            return "face-aspect"
        if area_ratio < 0.00025:
            return "face-too-small"
        if area_ratio > 0.75:
            return "face-too-large"
    elif region == "body":
        if aspect < 0.15 or aspect > 3.25:
            return "body-aspect"
        if area_ratio < 0.0005:
            return "body-too-small"
    elif region == "hands":
        if aspect < 0.25 or aspect > 4.0:
            return "hand-aspect"
        if area_ratio < 0.00012:
            return "hand-too-small"
        if area_ratio > 0.25:
            return "hand-too-large"
    return None


def detected_detail_regions(
    detectors: list[dict[str, Any]],
    image: Image.Image,
    detail_pass: dict[str, Any],
    worker_device: str,
) -> tuple[list[dict[str, Any]], int, int, dict[str, int], list[dict[str, Any]]]:
    regions: list[dict[str, Any]] = []
    raw_detection_count = 0
    rejection_counts: dict[str, int] = {}
    rejected_samples: list[dict[str, Any]] = []
    predict_device = detail_predict_device(worker_device)
    for detector in detectors:
        results = detector["model"].predict(
            source=image,
            conf=detail_pass["detectionConfidence"],
            verbose=False,
            device=predict_device,
        )
        for result in results:
            boxes = getattr(result, "boxes", None)
            if boxes is None or boxes.xyxy is None:
                continue
            xyxy = boxes.xyxy.detach().cpu().tolist()
            confidences = boxes.conf.detach().cpu().tolist()
            classes = (
                boxes.cls.detach().cpu().tolist()
                if getattr(boxes, "cls", None) is not None
                else [None] * len(xyxy)
            )
            for box, confidence, class_id in zip(xyxy, confidences, classes):
                raw_detection_count += 1
                box = [float(value) for value in box]
                class_name = detail_detector_class_name(detector, class_id)
                rejection_reason = detail_detection_rejection_reason(
                    detector["region"], class_name, box, image.size
                )
                if rejection_reason:
                    rejection_counts[rejection_reason] = (
                        rejection_counts.get(rejection_reason, 0) + 1
                    )
                    if len(rejected_samples) < 12:
                        rejected_samples.append(
                            {
                                "region": detector["region"],
                                "className": class_name,
                                "confidence": float(confidence),
                                "box": [round(float(value), 2) for value in box],
                                "reason": rejection_reason,
                            }
                        )
                    continue
                mask = mask_for_box(image.size, tuple(box), detail_pass["maskPaddingPx"])
                if mask.getbbox() is None:
                    rejection_counts["empty-mask"] = rejection_counts.get("empty-mask", 0) + 1
                    continue
                regions.append(
                    {
                        "region": detector["region"],
                        "className": class_name,
                        "confidence": float(confidence),
                        "box": [round(float(value), 2) for value in box],
                        "mask": mask,
                    }
                )
    regions.sort(key=lambda region: region["confidence"], reverse=True)
    return (
        regions[: detail_pass["maxRegions"]],
        len(regions),
        raw_detection_count,
        rejection_counts,
        rejected_samples,
    )


def run_detail_pass(
    pipe: Any,
    torch: Any,
    schedulers: tuple[Any, Any, Any],
    scheduler_config: Any,
    detectors: list[dict[str, Any]],
    detail_pass: dict[str, Any],
    detail_report: dict[str, Any],
    image: Image.Image,
    prompt: str,
    negative_prompt: str | None,
    cfg: float,
    seed: int,
    device: str,
    job_dir: Path,
    candidate_index: int,
    precomputed_detection: tuple[
        list[dict[str, Any]], int, int, dict[str, int], list[dict[str, Any]]
    ] | None = None,
) -> tuple[Image.Image, dict[str, Any]]:
    if detail_report.get("status") != "ready" or not detectors:
        debug_event(
            "detail-pass-skipped",
            {
                "candidate": candidate_index,
                "status": detail_report.get("status"),
                "detectors": len(detectors),
            },
        )
        return image, detail_report

    report = dict(detail_report)
    debug_event(
        "detail-pass-start",
        {
            "candidate": candidate_index,
            "detectors": [
                {"region": detector["region"], "path": detector["path"]}
                for detector in detectors
            ],
            "steps": detail_pass["steps"],
            "denoise": detail_pass["denoise"],
            "scheduler": detail_pass["scheduler"],
            "detailRenderEdge": detail_pass["detailRenderEdge"],
            "minCropEdge": detail_pass["minCropEdge"],
        },
    )
    emit_progress(
        {
            "type": "detail",
            "status": "detecting",
            "candidate": candidate_index,
        }
    )
    if precomputed_detection is None:
        precomputed_detection = detected_detail_regions(
            detectors, image, detail_pass, device
        )
    (
        regions,
        total_detected_regions,
        raw_detected_regions,
        rejection_counts,
        rejected_samples,
    ) = precomputed_detection
    report["detectedRegions"] = total_detected_regions
    report["rawDetectedRegions"] = raw_detected_regions
    report["rejectedDetections"] = sum(rejection_counts.values())
    report["rejectionCounts"] = rejection_counts
    if rejected_samples:
        report["rejectedDetectionSamples"] = rejected_samples
    report["selectedRegions"] = len(regions)
    report["maxRegions"] = detail_pass["maxRegions"]
    report["truncatedRegions"] = max(0, total_detected_regions - len(regions))
    report["appliedRegions"] = 0
    report["fallbackRegions"] = 0
    report["detections"] = [
        {
            "region": region["region"],
            "confidence": region["confidence"],
            "box": region["box"],
        }
        for region in regions
    ]
    if not regions:
        report["status"] = "no-detections"
        debug_event(
            "detail-pass-no-detections",
            {
                "candidate": candidate_index,
                "fallbackToEditMask": False,
                "rawDetectedRegions": raw_detected_regions,
                "rejectedDetections": sum(rejection_counts.values()),
                "rejectionCounts": rejection_counts,
            },
        )
        emit_progress(
            {
                "type": "detail",
                "status": "no-detections",
                "candidate": candidate_index,
            }
        )
        return image, report

    if pipe is None:
        report["status"] = "pipeline-unavailable"
        report["message"] = "Detail regions were detected, but no inpaint pipeline is loaded."
        debug_event(
            "detail-pass-pipeline-unavailable",
            {
                "candidate": candidate_index,
                "detectedRegions": total_detected_regions,
                "selectedRegions": len(regions),
            },
        )
        return image, report

    current = image.convert("RGB")
    scheduler_key, scheduler_class = apply_scheduler(
        pipe, detail_pass["scheduler"], *schedulers, scheduler_config=scheduler_config
    )
    report["scheduler"] = scheduler_key
    report["schedulerClass"] = scheduler_class

    failures: list[str] = []
    crop_reports: list[dict[str, Any]] = []
    for region_index, region in enumerate(regions, start=1):
        try:
            region_mask = region["mask"]
            crop_box = detail_crop_region(
                region_mask, current.size, detail_pass["minCropEdge"]
            )
            if crop_box is None:
                continue
            crop_width = crop_box[2] - crop_box[0]
            crop_height = crop_box[3] - crop_box[1]
            if crop_width <= 0 or crop_height <= 0:
                continue
            render_size = detail_render_size(
                (crop_width, crop_height), detail_pass["detailRenderEdge"]
            )
            source_crop = current.crop(crop_box)
            mask_crop = region_mask.crop(crop_box)
            source_render = source_crop.resize(render_size, Image.Resampling.LANCZOS)
            mask_render = mask_crop.resize(render_size, Image.Resampling.LANCZOS)
            source_padded, unpadded_size = pad_source_to_multiple(source_render, 8)
            mask_padded = pad_mask_to_size(mask_render, source_padded.size)
            crop_reports.append(
                {
                    "region": region["region"],
                    "box": list(crop_box),
                    "cropWidth": crop_width,
                    "cropHeight": crop_height,
                    "renderWidth": source_padded.width,
                    "renderHeight": source_padded.height,
                }
            )
            detail_effective_steps = effective_diffusion_steps(
                detail_pass["steps"], detail_pass["denoise"]
            )

            def on_detail_step_end(
                pipeline: Any, step_index: int, timestep: Any, callback_kwargs: dict[str, Any]
            ) -> dict[str, Any]:
                del pipeline, timestep
                step = int(step_index) + 1
                interval = max(
                    1,
                    min(
                        8,
                        detail_effective_steps // 8
                        if detail_effective_steps >= 8
                        else 1,
                    ),
                )
                if (
                    step == 1
                    or step == detail_effective_steps
                    or step % interval == 0
                ):
                    emit_progress(
                        {
                            "type": "detail",
                            "status": "refining",
                            "candidate": candidate_index,
                            "region": region["region"],
                            "regionIndex": region_index,
                            "regions": len(regions),
                            "step": step,
                            "steps": detail_effective_steps,
                            "requestedSteps": detail_pass["steps"],
                            "denoise": detail_pass["denoise"],
                            "renderWidth": source_padded.width,
                            "renderHeight": source_padded.height,
                        }
                    )
                return callback_kwargs

            detailed = pipe(
                prompt=prompt,
                negative_prompt=negative_prompt,
                image=source_padded,
                mask_image=mask_padded,
                height=source_padded.height,
                width=source_padded.width,
                guidance_scale=cfg,
                strength=detail_pass["denoise"],
                num_inference_steps=detail_pass["steps"],
                generator=generator_for_seed(torch, seed + 1000 + region_index, device),
                callback_on_step_end=on_detail_step_end,
                callback_on_step_end_tensor_inputs=["latents"],
            ).images[0]
            detailed = detailed.crop((0, 0, unpadded_size[0], unpadded_size[1]))
            detailed = detailed.resize((crop_width, crop_height), Image.Resampling.LANCZOS)
            blend_mask = region_mask.filter(
                ImageFilter.GaussianBlur(
                    radius=max(1, min(detail_pass["maskPaddingPx"] // 4, 16))
                )
            ).crop(crop_box)
            current_crop = current.crop(crop_box)
            blended_crop = Image.composite(
                detailed.convert("RGB"), current_crop, blend_mask
            )
            current.paste(blended_crop, (crop_box[0], crop_box[1]))
            region_mask.save(
                job_dir / f"detail-mask-c{candidate_index}-r{region_index}.png"
            )
            source_crop.save(
                job_dir / f"detail-source-c{candidate_index}-r{region_index}.png"
            )
            detailed.save(
                job_dir / f"detail-render-c{candidate_index}-r{region_index}.png"
            )
            report["appliedRegions"] += 1
            emit_progress(
                {
                    "type": "detail",
                    "status": "applied-region",
                    "candidate": candidate_index,
                    "region": region["region"],
                    "regionIndex": region_index,
                    "regions": len(regions),
                }
            )
        except Exception as exc:  # noqa: BLE001
            failures.append(f"{region['region']}: {exc}")
            debug_event(
                "detail-pass-region-error",
                {
                    "candidate": candidate_index,
                    "region": region["region"],
                    "regionIndex": region_index,
                    "message": str(exc),
                    "traceback": traceback.format_exc(),
                },
            )

    report["status"] = "applied" if report["appliedRegions"] else "failed"
    report["crops"] = crop_reports
    if failures:
        report["failures"] = failures
    debug_event(
        "detail-pass-complete",
        {
            "candidate": candidate_index,
            "status": report["status"],
            "detectedRegions": report.get("detectedRegions", 0),
            "appliedRegions": report.get("appliedRegions", 0),
            "fallbackRegions": report.get("fallbackRegions", 0),
            "failures": failures,
        },
    )
    return current, report


def run_refiner_pass(
    pipe: Any,
    torch: Any,
    refiner: dict[str, Any],
    image: Image.Image,
    prompt: str,
    negative_prompt: str | None,
    cfg: float,
    seed: int,
    device: str,
    candidate_index: int,
) -> Image.Image:
    effective_steps = effective_diffusion_steps(
        refiner["steps"], refiner["strength"]
    )
    emit_progress(
        {
            "type": "refiner",
            "status": "starting",
            "candidate": candidate_index,
            "steps": effective_steps,
            "requestedSteps": refiner["steps"],
            "strength": refiner["strength"],
        }
    )
    last_step = 0

    def on_refiner_step_end(
        pipeline: Any, step_index: int, timestep: Any, callback_kwargs: dict[str, Any]
    ) -> dict[str, Any]:
        del pipeline, timestep
        nonlocal last_step
        step = int(step_index) + 1
        interval = max(
            1,
            min(8, effective_steps // 8 if effective_steps >= 8 else 1),
        )
        if step == 1 or step == effective_steps or step - last_step >= interval:
            last_step = step
            emit_progress(
                {
                    "type": "refiner",
                    "status": "refining",
                    "candidate": candidate_index,
                    "step": step,
                    "steps": effective_steps,
                    "requestedSteps": refiner["steps"],
                    "strength": refiner["strength"],
                }
            )
        return callback_kwargs

    refined = pipe(
        prompt=prompt,
        negative_prompt=negative_prompt,
        image=image.convert("RGB"),
        strength=refiner["strength"],
        num_inference_steps=refiner["steps"],
        guidance_scale=cfg,
        generator=generator_for_seed(torch, seed + 2000, device),
        callback_on_step_end=on_refiner_step_end,
        callback_on_step_end_tensor_inputs=["latents"],
    ).images[0]
    emit_progress(
        {
            "type": "refiner",
            "status": "complete",
            "candidate": candidate_index,
            "step": effective_steps,
            "steps": effective_steps,
            "requestedSteps": refiner["steps"],
            "strength": refiner["strength"],
        }
    )
    return refined.convert("RGB")


def run_gguf_refiner_pass(
    refiner: dict[str, Any],
    image: Image.Image,
    prompt: str,
    negative_prompt: str | None,
    cfg: float,
    seed: int,
    job_dir: Path,
    candidate_index: int,
) -> Image.Image:
    runner = str(refiner.get("runner", "")).strip()
    if not runner:
        raise RuntimeError(
            "GGUF refiner backend is selected, but "
            "UNDERPAINT_GGUF_REFINER_WORKER is not set."
        )

    emit_progress(
        {
            "type": "refiner",
            "status": "starting-gguf",
            "candidate": candidate_index,
        }
    )
    refiner_dir = job_dir / "gguf-refiner"
    refiner_dir.mkdir(parents=True, exist_ok=True)
    input_path = refiner_dir / f"candidate-{candidate_index}-input.png"
    request_path = refiner_dir / f"candidate-{candidate_index}-request.json"
    response_path = refiner_dir / f"candidate-{candidate_index}-response.json"
    image.convert("RGB").save(input_path)

    request = {
        "schema": "underpaint.gguf-refiner.v1",
        "candidate": candidate_index,
        "inputImagePath": str(input_path),
        "outputImagePath": str(refiner_dir / f"candidate-{candidate_index}-output.png"),
        "model": refiner["model"],
        "prompt": prompt,
        "negativePrompt": negative_prompt or "",
        "cfg": cfg,
        "strength": refiner["strength"],
        "steps": refiner["steps"],
        "scheduler": refiner["scheduler"],
        "seed": seed + 2000,
    }
    write_json(request_path, request)

    command = [*shlex.split(runner), str(request_path), str(response_path), str(refiner_dir)]
    debug_event(
        "gguf-refiner-runner-start",
        {
            "candidate": candidate_index,
            "runner": runner,
            "requestPath": str(request_path),
            "responsePath": str(response_path),
            "model": refiner["model"],
        },
    )
    completed = subprocess.run(
        command,
        cwd=str(job_dir),
        text=True,
        capture_output=True,
        timeout=max(60, int(refiner["steps"]) * 20),
        check=False,
    )
    debug_event(
        "gguf-refiner-runner-complete",
        {
            "candidate": candidate_index,
            "returnCode": completed.returncode,
            "stdout": completed.stdout[-4000:],
            "stderr": completed.stderr[-4000:],
        },
    )
    if completed.returncode != 0:
        raise RuntimeError(
            "GGUF refiner worker failed "
            f"with exit code {completed.returncode}: {completed.stderr.strip()}"
        )
    if not response_path.is_file():
        raise RuntimeError("GGUF refiner worker did not write a response JSON file.")
    payload = json.loads(response_path.read_text(encoding="utf-8"))
    if payload.get("status", "succeeded") != "succeeded":
        raise RuntimeError(str(payload.get("message", "GGUF refiner worker failed.")))
    output_path = Path(
        payload.get("imagePath")
        or payload.get("outputImagePath")
        or request["outputImagePath"]
    )
    if not output_path.is_file():
        raise RuntimeError(f"GGUF refiner output image was not found: {output_path}")
    emit_progress(
        {
            "type": "refiner",
            "status": "complete-gguf",
            "candidate": candidate_index,
            "step": refiner["steps"],
            "steps": refiner["steps"],
        }
    )
    return Image.open(output_path).convert("RGB")


def average_color(image: Image.Image, alpha: Image.Image) -> tuple[int, int, int]:
    alpha_bbox = alpha.getbbox()
    if alpha_bbox is None:
        return (0, 0, 0)

    small_rgb = image.crop(alpha_bbox).resize((1, 1), Image.Resampling.BOX)
    return small_rgb.getpixel((0, 0))


def stretched_color_field(source: Image.Image, alpha: Image.Image) -> Image.Image:
    source_rgb = source.convert("RGB")
    alpha = alpha.convert("L").point(lambda pixel: 255 if pixel > 8 else 0)
    content_bbox = alpha.getbbox()
    if content_bbox is None:
        return source_rgb

    width, height = source_rgb.size
    base = Image.new("RGB", source_rgb.size, average_color(source_rgb, alpha))
    base.paste(source_rgb, (0, 0), alpha)

    content = base.crop(content_bbox)
    color_field = content.resize(source_rgb.size, Image.Resampling.BICUBIC)
    blur_radius = max(16, round(max(width, height) * 0.09))
    blurred = color_field.filter(ImageFilter.GaussianBlur(radius=blur_radius))
    return Image.blend(color_field, blurred, 0.92)


def paste_masked(fill: Image.Image, patch: Image.Image, xy: tuple[int, int], mask: Image.Image) -> None:
    if patch.width <= 0 or patch.height <= 0:
        return
    fill.paste(patch, xy, mask)


def stretched_edge_slice_field(source: Image.Image, alpha: Image.Image, mask: Image.Image) -> Image.Image:
    """Prefill outpaint regions by stretching 25% slices from each content edge."""
    source_rgb = source.convert("RGB")
    alpha = alpha.convert("L").point(lambda pixel: 255 if pixel > 8 else 0)
    content_bbox = alpha.getbbox()
    if content_bbox is None:
        return source_rgb

    width, height = source_rgb.size
    content_left, content_top, content_right, content_bottom = content_bbox
    content_width = content_right - content_left
    content_height = content_bottom - content_top
    if content_width <= 0 or content_height <= 0:
        return source_rgb

    slice_width = max(1, content_width // 4)
    slice_height = max(1, content_height // 4)
    fill = source_rgb.copy()
    mask = mask.convert("L").point(lambda pixel: 255 if pixel > 8 else 0)

    if content_left > 0:
        left_strip = source_rgb.crop(
            (content_left, content_top, content_left + slice_width, content_bottom)
        )
        left_patch = left_strip.resize(
            (content_left, height), Image.Resampling.BICUBIC
        )
        paste_masked(
            fill,
            left_patch,
            (0, 0),
            mask.crop((0, 0, content_left, height)),
        )

    if content_right < width:
        right_strip = source_rgb.crop(
            (content_right - slice_width, content_top, content_right, content_bottom)
        )
        right_width = width - content_right
        right_patch = right_strip.resize(
            (right_width, height), Image.Resampling.BICUBIC
        )
        paste_masked(
            fill,
            right_patch,
            (content_right, 0),
            mask.crop((content_right, 0, width, height)),
        )

    if content_top > 0:
        top_strip = source_rgb.crop(
            (content_left, content_top, content_right, content_top + slice_height)
        )
        top_patch = top_strip.resize((width, content_top), Image.Resampling.BICUBIC)
        paste_masked(
            fill,
            top_patch,
            (0, 0),
            mask.crop((0, 0, width, content_top)),
        )

    if content_bottom < height:
        bottom_strip = source_rgb.crop(
            (content_left, content_bottom - slice_height, content_right, content_bottom)
        )
        bottom_height = height - content_bottom
        bottom_patch = bottom_strip.resize(
            (width, bottom_height), Image.Resampling.BICUBIC
        )
        paste_masked(
            fill,
            bottom_patch,
            (0, content_bottom),
            mask.crop((0, content_bottom, width, height)),
        )

    return fill


def mask_component_bounds(mask: Image.Image) -> list[tuple[int, int, int, int]]:
    import numpy as np

    pixels = np.array(mask.convert("L")) > 8
    height, width = pixels.shape
    visited = np.zeros(pixels.shape, dtype=bool)
    bounds: list[tuple[int, int, int, int]] = []
    ys, xs = np.where(pixels)
    for start_y, start_x in zip(ys, xs):
        if visited[start_y, start_x]:
            continue
        stack = [(int(start_x), int(start_y))]
        visited[start_y, start_x] = True
        left = right = int(start_x)
        top = bottom = int(start_y)
        while stack:
            x, y = stack.pop()
            left = min(left, x)
            right = max(right, x)
            top = min(top, y)
            bottom = max(bottom, y)
            for next_x, next_y in (
                (x - 1, y),
                (x + 1, y),
                (x, y - 1),
                (x, y + 1),
            ):
                if (
                    0 <= next_x < width
                    and 0 <= next_y < height
                    and pixels[next_y, next_x]
                    and not visited[next_y, next_x]
                ):
                    visited[next_y, next_x] = True
                    stack.append((next_x, next_y))
        bounds.append((left, top, right + 1, bottom + 1))
    return bounds


def stretched_mask_component_field(source: Image.Image, mask: Image.Image) -> Image.Image:
    """Prefill internal object holes from the nearest 25% edge slices."""
    source_rgb = source.convert("RGB")
    width, height = source_rgb.size
    mask = mask.convert("L").point(lambda pixel: 255 if pixel > 8 else 0)
    fill = source_rgb.copy()
    for left, top, right, bottom in mask_component_bounds(mask):
        hole_width = right - left
        hole_height = bottom - top
        if hole_width <= 0 or hole_height <= 0:
            continue
        component_mask = mask.crop((left, top, right, bottom))
        slice_width = max(1, min(max(8, hole_width // 4), width))
        slice_height = max(1, min(max(8, hole_height // 4), height))
        filled_component = False

        if left > 0:
            strip_left = max(0, left - min(slice_width, left))
            strip = source_rgb.crop((strip_left, top, left, bottom))
            if strip.width > 0 and strip.height > 0:
                patch = strip.resize((hole_width, hole_height), Image.Resampling.BICUBIC)
                paste_masked(fill, patch, (left, top), component_mask)
                filled_component = True

        if right < width:
            strip_right = min(width, right + min(slice_width, width - right))
            strip = source_rgb.crop((right, top, strip_right, bottom))
            if strip.width > 0 and strip.height > 0:
                patch = strip.resize((hole_width, hole_height), Image.Resampling.BICUBIC)
                paste_masked(fill, patch, (left, top), component_mask)
                filled_component = True

        if top > 0:
            strip_top = max(0, top - min(slice_height, top))
            strip = source_rgb.crop((left, strip_top, right, top))
            if strip.width > 0 and strip.height > 0:
                patch = strip.resize((hole_width, hole_height), Image.Resampling.BICUBIC)
                paste_masked(fill, patch, (left, top), component_mask)
                filled_component = True

        if bottom < height:
            strip_bottom = min(height, bottom + min(slice_height, height - bottom))
            strip = source_rgb.crop((left, bottom, right, strip_bottom))
            if strip.width > 0 and strip.height > 0:
                patch = strip.resize((hole_width, hole_height), Image.Resampling.BICUBIC)
                paste_masked(fill, patch, (left, top), component_mask)
                filled_component = True

        if not filled_component:
            blurred = source_rgb.filter(ImageFilter.GaussianBlur(radius=32))
            fill.paste(blurred.crop((left, top, right, bottom)), (left, top), component_mask)
    return fill


def blurred_visible_context_field(
    source: Image.Image, mask: Image.Image, radius: int = 56
) -> Image.Image:
    """Prefill object-removal holes from blurred visible pixels.

    Internal object removal is not the same as outpainting. Stretching edge
    strips across a large person-shaped hole creates strong bands that the
    diffusion model then tries to preserve. A normalized alpha blur gives the
    model a softer color/light primer while still asking it to rebuild content
    inside the explicit mask.
    """

    import numpy as np

    rgba = source.convert("RGBA")
    source_rgb = rgba.convert("RGB")
    alpha = rgba.getchannel("A")
    binary_mask = mask.convert("L").point(lambda pixel: 255 if pixel > 8 else 0)
    if binary_mask.getbbox() is None:
        return source_rgb

    rgb_array = np.asarray(source_rgb, dtype=np.float32)
    alpha_array = np.asarray(alpha, dtype=np.float32) / 255.0
    visible = alpha_array > 0.03
    if not visible.any():
        return source_rgb
    average_visible_rgb = rgb_array[visible].mean(axis=0)

    premultiplied = np.clip(rgb_array * alpha_array[..., None], 0, 255).astype(
        np.uint8
    )
    blurred_rgb = np.asarray(
        Image.fromarray(premultiplied, "RGB").filter(
            ImageFilter.GaussianBlur(radius=radius)
        ),
        dtype=np.float32,
    )
    blurred_alpha = np.asarray(
        Image.fromarray(np.clip(alpha_array * 255, 0, 255).astype(np.uint8), "L").filter(
            ImageFilter.GaussianBlur(radius=radius)
        ),
        dtype=np.float32,
    ) / 255.0
    normalized = blurred_rgb / np.maximum(blurred_alpha[..., None], 0.015)
    normalized = np.clip(normalized, 0, 255)
    low_support = blurred_alpha < 0.015
    if low_support.any():
        normalized[low_support] = average_visible_rgb

    result = rgb_array.copy()
    mask_array = np.asarray(binary_mask, dtype=np.uint8) > 8
    result[mask_array] = normalized[mask_array]
    filled = Image.fromarray(np.clip(result, 0, 255).astype(np.uint8), "RGB")
    soft_mask = binary_mask.filter(ImageFilter.GaussianBlur(radius=12))
    softened = filled.filter(ImageFilter.GaussianBlur(radius=6))
    filled.paste(softened, (0, 0), soft_mask)
    return filled


def prefill_outpaint_source(
    source: Image.Image,
    mask: Image.Image,
    prefill_noise: float = 0.42,
    prefill_style: str = "",
) -> Image.Image:
    """Give outpaint models a plausible continuation under the editable mask."""
    source_rgb = source.convert("RGB")
    source_alpha = source.getchannel("A") if "A" in source.getbands() else None
    mask = mask.convert("L").point(lambda pixel: 255 if pixel > 8 else 0)
    bbox = mask.getbbox()
    if bbox is None:
        return source_rgb
    if prefill_style.lower() == "object-context-plate" and source_alpha is not None:
        fill = blurred_visible_context_field(source, mask)
        if prefill_noise > 0.0:
            noise = Image.effect_noise(source_rgb.size, 96).convert("L")
            noise_rgb = Image.merge("RGB", (noise, noise, noise))
            noisy_fill = Image.blend(fill, noise_rgb, max(0.0, min(prefill_noise, 0.85)))
            fill.paste(noisy_fill, (0, 0), mask)
        return fill

    width, height = source_rgb.size
    left, top, right, bottom = bbox
    fill = source_rgb.copy()
    if source_alpha is not None:
        alpha_bbox = source_alpha.convert("L").point(
            lambda pixel: 255 if pixel > 8 else 0
        ).getbbox()
        if alpha_bbox and alpha_bbox != (0, 0, width, height):
            return stretched_edge_slice_field(source, source_alpha, mask)

    did_edge_fill = False
    if left == 0 and right < width:
        fill_width = right
        edge = source_rgb.crop((right, 0, right + 1, height))
        fill.paste(
            edge.resize((fill_width, height), Image.Resampling.NEAREST),
            (0, 0),
            mask.crop((0, 0, fill_width, height)),
        )
        did_edge_fill = True

    if right == width and left > 0:
        fill_width = width - left
        edge = source_rgb.crop((left - 1, 0, left, height))
        fill.paste(
            edge.resize((fill_width, height), Image.Resampling.NEAREST),
            (left, 0),
            mask.crop((left, 0, width, height)),
        )
        did_edge_fill = True

    if top == 0 and bottom < height:
        fill_height = bottom
        edge = fill.crop((0, bottom, width, bottom + 1))
        fill.paste(
            edge.resize((width, fill_height), Image.Resampling.NEAREST),
            (0, 0),
            mask.crop((0, 0, width, fill_height)),
        )
        did_edge_fill = True

    if bottom == height and top > 0:
        fill_height = height - top
        edge = fill.crop((0, top - 1, width, top))
        fill.paste(
            edge.resize((width, fill_height), Image.Resampling.NEAREST),
            (0, top),
            mask.crop((0, top, width, height)),
        )
        did_edge_fill = True

    if not did_edge_fill and source_alpha is not None:
        fill = stretched_mask_component_field(source, mask)
        did_edge_fill = True

    if not did_edge_fill and source_alpha is None:
        fill.paste(source_rgb.filter(ImageFilter.GaussianBlur(radius=32)), (0, 0), mask)

    if prefill_noise > 0.0:
        noise = Image.effect_noise(source_rgb.size, 96).convert("L")
        noise_rgb = Image.merge("RGB", (noise, noise, noise))
        noisy_fill = Image.blend(fill, noise_rgb, max(0.0, min(prefill_noise, 0.85)))
        fill.paste(noisy_fill, (0, 0), mask)

    soft_mask = mask.filter(ImageFilter.GaussianBlur(radius=12))
    softened = fill.filter(ImageFilter.GaussianBlur(radius=8))
    fill.paste(softened, (0, 0), soft_mask)
    return fill


def generator_for_seed(torch: Any, seed: int, device: str) -> Any:
    if seed < 0:
        return None
    return torch.Generator(device=device).manual_seed(seed)


def main(argv: list[str]) -> int:
    if len(argv) != 4:
        print(
            "Usage: underpaint-diffusers-worker.py "
            "<request.json> <response.json> <job-directory>",
            file=sys.stderr,
        )
        return 2

    request_path = Path(argv[1])
    response_path = Path(argv[2])
    job_dir = Path(argv[3])
    job_dir.mkdir(parents=True, exist_ok=True)
    configure_debug_logging(job_dir)

    started = time.monotonic()
    request: dict[str, Any] = {}
    try:
        request = json.loads(request_path.read_text(encoding="utf-8"))
    except Exception as exc:  # noqa: BLE001
        return fail(response_path, "", f"Invalid request JSON: {exc}", 2)

    request_id = str(request.get("id") or "")
    if not request_id:
        return fail(response_path, "", "Request id is missing.", 2)
    if request.get("schema") != SCHEMA:
        return fail(response_path, request_id, "Unsupported AI job schema.", 2)
    operation = request.get("operation")
    if operation not in {
        "scene-separation",
        "object-decomposition",
        "background-removal",
        "inpaint",
        "generative-fill",
        "outpaint",
    }:
        return fail(
            response_path,
            request_id,
            f"Unsupported operation: {operation}",
            2,
        )
    debug_event(
        "request-validated",
        {
            "requestId": request_id,
            "operation": operation,
            "responsePath": str(response_path),
            "jobDir": str(job_dir),
            "inputRoles": [
                asset.get("role")
                for asset in request.get("inputs", [])
                if isinstance(asset, dict)
            ],
        },
    )
    if operation == "scene-separation":
        return write_scene_separation_response(
            request, response_path, request_id, job_dir, started
        )
    if operation == "object-decomposition":
        return write_object_decomposition_response(
            request, response_path, request_id, job_dir, started
        )
    if operation == "background-removal":
        return write_background_removal_response(
            request, response_path, request_id, job_dir, started
        )

    source_path = asset_path(request, "source-image")
    mask_path = asset_path(request, "mask")
    if source_path is None or mask_path is None:
        return fail(response_path, request_id, "source-image and mask are required.")

    try:
        source_rgba = Image.open(source_path).convert("RGBA")
        source = source_rgba.convert("RGB")
        output_size = source.size
        mask = load_mask(mask_path, source.size)
        output_mask = mask.copy()
        parameters = request.get("parameters", {})
        prefill_style = str(parameters.get("prefillStyle", "")).lower()
        should_prefill = operation == "outpaint" or prefill_style in {
            "edge-slices-25",
            "context-plate",
            "object-context-plate",
        }
        if should_prefill:
            prefill_style = str(
                parameters.get("prefillStyle", "edge-slices-25")
            )
            prefill_noise = float(
                parameters.get("prefillNoise", 0.42 if operation == "outpaint" else 0.0)
            )
            debug_event(
                "masked-prefill",
                {
                    "operation": operation,
                    "style": prefill_style,
                    "prefillNoise": prefill_noise,
                    "hasSourceAlpha": "A" in source_rgba.getbands(),
                },
            )
            source = prefill_outpaint_source(
                source_rgba, mask, prefill_noise, prefill_style
            )
            if DEBUG_ENABLED:
                prefill_path = job_dir / "prefill-source.png"
                source.save(prefill_path)
                debug_event("masked-prefill-saved", {"path": str(prefill_path)})
        preferences = request.get("preferences", {})
        requested_render_edge = int(
            preferences.get("targetRenderEdge")
            or preferences.get("maxRenderEdge", 1024)
        )
        target_render_edge = max(64, min(requested_render_edge, 2048))
        source, mask, render_size = resize_for_render(
            source, mask, target_render_edge
        )
        source, unpadded_render_size = pad_source_to_multiple(source, 8)
        mask = pad_mask_to_size(mask, source.size)
        mask_min, mask_max = mask.getextrema()
        if mask_max == 0:
            return fail(
                response_path,
                request_id,
                "The inpaint mask contains no editable pixels.",
                2,
            )
        mask_bbox = mask.getbbox()
        output_mask_bbox = output_mask.getbbox()
        debug_event(
            "assets-loaded",
            {
                "sourcePath": str(source_path),
                "maskPath": str(mask_path),
                "outputWidth": output_size[0],
                "outputHeight": output_size[1],
                "renderWidth": unpadded_render_size[0],
                "renderHeight": unpadded_render_size[1],
                "paddedWidth": source.width,
                "paddedHeight": source.height,
                "maskMin": int(mask_min),
                "maskMax": int(mask_max),
                "maskBox": list(mask_bbox) if mask_bbox else None,
                "outputMaskBox": list(output_mask_bbox) if output_mask_bbox else None,
            },
        )
    except Exception as exc:  # noqa: BLE001
        return fail(response_path, request_id, f"Could not load source assets: {exc}")

    try:
        import torch
        from diffusers import (
            AutoPipelineForInpainting,
            DDIMScheduler,
            DPMSolverMultistepScheduler,
            EulerAncestralDiscreteScheduler,
            EulerDiscreteScheduler,
            StableDiffusionXLImg2ImgPipeline,
        )
    except Exception as exc:  # noqa: BLE001
        return fail(
            response_path,
            request_id,
            f"Diffusers dependencies are not installed: {exc}",
            2,
        )

    allow_cpu = os.environ.get("UNDERPAINT_AI_ALLOW_CPU") == "1"
    if torch.cuda.is_available():
        device = "cuda"
        dtype = torch.float16
    elif allow_cpu:
        device = "cpu"
        dtype = torch.float32
    else:
        return fail(
            response_path,
            request_id,
            "CUDA is not available. Reboot or repair the NVIDIA driver, or set "
            "UNDERPAINT_AI_ALLOW_CPU=1 for a very slow CPU test.",
        )

    parameters = request.get("parameters", {})
    preferences = request.get("preferences", {})
    default_prompt = (
        "seamlessly extend this image outward, continuing the existing subject, "
        "background, colors, lighting, texture, perspective, and level of detail; "
        "the central subject must remain unchanged"
        if operation == "outpaint"
        else (
            "reconstruct the selected area with concrete subject detail, matching "
            "the surrounding perspective, lighting, texture, color, and camera feel"
        )
    )
    prompt = parameters.get("prompt") or default_prompt
    negative_prompt = parameters.get("negativePrompt") or None
    if operation == "outpaint":
        prompt = outpaint_positive_prompt(prompt)
        negative_prompt = outpaint_negative_prompt(negative_prompt)
    candidate_count = max(1, min(int(parameters.get("candidateCount", 1)), 4))
    cfg = float(parameters.get("cfg", 5.0))
    strength = float(parameters.get("denoise", 0.75))
    steps = int(parameters.get("steps") or 20)
    scheduler_name = (
        parameters.get("scheduler")
        or parameters.get("sampler")
        or preferences.get("scheduler")
        or os.environ.get("UNDERPAINT_SCHEDULER")
        or "dpmpp-3m-karras"
    )
    refiner = normalized_refiner(parameters)
    refiner_report = refiner_diagnostics(refiner)
    detail_pass = normalized_detail_pass(parameters)
    if operation == "outpaint":
        detail_pass["fallbackToEditMask"] = False
    detail_pass_report = detail_pass_diagnostics(detail_pass)
    edge_feather_px = max(0, min(int(parameters.get("edgeFeatherPx", 24)), 256))
    preview_max_edge = max(64, min(int(parameters.get("previewMaxEdge", 256)), 512))
    preview_every_steps = int(
        parameters.get("previewEverySteps", max(4, steps // 5 if steps >= 5 else 1))
    )
    preview_every_steps = max(1, min(preview_every_steps, max(1, steps)))
    if strength > 0.0 and int(steps * strength) < 1:
        steps = max(steps, math.ceil(1.0 / strength))
    base_effective_steps = effective_diffusion_steps(steps, strength)
    preview_every_steps = max(1, min(preview_every_steps, base_effective_steps))
    requested_seed = int(parameters.get("seed", -1))
    max_base_seed = (2**31 - 1) - candidate_count
    base_seed = (
        min(requested_seed, max_base_seed)
        if requested_seed >= 0
        else secrets.randbelow(max_base_seed + 1)
    )
    model_config = normalized_generation_model(parameters)
    model_id = model_config["model"]
    if model_config["format"] == "single_file_sdxl" and detail_pass.get("enabled"):
        detail_pass["enabled"] = False
        detail_pass_report["status"] = "unsupported-model-format"
        detail_pass_report["message"] = (
            "Detail pass is disabled for single-file SDXL masked-img2img models."
        )
    debug_event(
        "generation-config",
        {
            "requestId": request_id,
            "operation": operation,
            "model": model_id,
            "modelConfig": model_config,
            "prompt": prompt,
            "negativePrompt": negative_prompt,
            "candidateCount": candidate_count,
            "cfg": cfg,
            "denoise": strength,
            "steps": steps,
            "effectiveSteps": base_effective_steps,
            "scheduler": scheduler_name,
            "seed": base_seed,
            "requestedSeed": requested_seed,
            "refiner": refiner,
            "detailPass": detail_pass,
            "edgeFeatherPx": edge_feather_px,
            "previewEverySteps": preview_every_steps,
            "previewMaxEdge": preview_max_edge,
            "targetRenderEdge": target_render_edge,
            "device": device,
            "dtype": str(dtype),
        },
    )

    try:
        load_kwargs: dict[str, Any] = {
            "torch_dtype": dtype,
            "use_safetensors": True,
        }
        if dtype is torch.float16:
            load_kwargs["variant"] = "fp16"
        debug_event(
            "model-load-start",
            {"model": model_id, "modelConfig": model_config, "loadKwargs": load_kwargs},
        )
        if model_config["format"] == "single_file_sdxl":
            pipe = load_single_file_pipeline(
                StableDiffusionXLImg2ImgPipeline, model_id, load_kwargs
            )
        else:
            pipe = load_pipeline(AutoPipelineForInpainting, model_id, load_kwargs)
        debug_event(
            "model-load-complete",
            {
                "model": model_id,
                "modelFormat": model_config["format"],
                "modelAdapter": model_config["adapter"],
                "pipelineClass": pipe.__class__.__name__,
                "schedulerClass": pipe.scheduler.__class__.__name__,
            },
        )
        scheduler_config = pipe.scheduler.config
        scheduler_classes = (
            EulerDiscreteScheduler,
            EulerAncestralDiscreteScheduler,
            DPMSolverMultistepScheduler,
            DDIMScheduler,
        )
        scheduler_key, scheduler_class = apply_scheduler(
            pipe,
            scheduler_name,
            *scheduler_classes,
            scheduler_config=scheduler_config,
        )
        prepare_pipeline_for_device(pipe, device, preferences, "base-inpaint")
        if preferences.get("vaeTiling", True):
            if hasattr(pipe, "vae") and hasattr(pipe.vae, "enable_tiling"):
                pipe.vae.enable_tiling()
            elif hasattr(pipe, "enable_vae_tiling"):
                pipe.enable_vae_tiling()
        if hasattr(pipe, "vae") and hasattr(pipe.vae, "enable_slicing"):
            pipe.vae.enable_slicing()
        elif hasattr(pipe, "enable_vae_slicing"):
            pipe.enable_vae_slicing()

        base_candidates: list[dict[str, Any]] = []
        candidates: list[dict[str, Any]] = []
        detail_pass_runs: list[dict[str, Any]] = []
        peak_vram_mb = None
        if device == "cuda":
            torch.cuda.reset_peak_memory_stats()

        for index in range(candidate_count):
            seed = base_seed + index
            last_preview_step = 0
            debug_event(
                "candidate-start",
                {
                    "candidate": index + 1,
                    "seed": seed,
                    "steps": steps,
                    "effectiveSteps": base_effective_steps,
                    "denoise": strength,
                    "scheduler": scheduler_name,
                },
            )
            scheduler_key, scheduler_class = apply_scheduler(
                pipe,
                scheduler_name,
                *scheduler_classes,
                scheduler_config=scheduler_config,
            )

            def on_step_end(pipeline: Any, step_index: int, timestep: Any, callback_kwargs: dict[str, Any]) -> dict[str, Any]:
                nonlocal last_preview_step
                step = step_index + 1
                if (
                    step != base_effective_steps
                    and step - last_preview_step < preview_every_steps
                ):
                    return {}
                last_preview_step = step
                try:
                    preview = decode_latents_to_image(
                        pipeline, torch, callback_kwargs["latents"]
                    )
                    preview = preview.crop(
                        (0, 0, unpadded_render_size[0], unpadded_render_size[1])
                    )
                    if unpadded_render_size != output_size:
                        preview = preview.resize(output_size, Image.Resampling.LANCZOS)
                    preview = apply_alpha_mask(preview, output_mask, edge_feather_px)
                    preview = resize_to_max_edge(preview, preview_max_edge)
                    preview_id = f"preview-c{index + 1}-s{step}"
                    preview_path = job_dir / f"{preview_id}.png"
                    preview.save(preview_path)
                    emit_progress(
                        {
                            "type": "preview",
                            "id": preview_id,
                            "candidate": index + 1,
                            "step": step,
                            "steps": base_effective_steps,
                            "requestedSteps": steps,
                            "denoise": strength,
                            "seed": seed,
                            "imagePath": str(preview_path),
                        }
                    )
                except Exception as exc:  # noqa: BLE001
                    emit_progress(
                        {
                            "type": "preview-error",
                            "candidate": index + 1,
                            "step": step,
                            "message": str(exc),
                        }
                    )
                return {}

            generated = generate_base_candidate(
                pipe,
                torch,
                model_config,
                source,
                mask,
                prompt,
                negative_prompt,
                cfg,
                strength,
                steps,
                seed,
                device,
                on_step_end,
            )
            generated = generated.crop(
                (0, 0, unpadded_render_size[0], unpadded_render_size[1])
            )
            debug_event(
                "candidate-base-complete",
                {
                    "candidate": index + 1,
                    "seed": seed,
                    "scheduler": scheduler_key,
                    "schedulerClass": scheduler_class,
                    "steps": steps,
                    "effectiveSteps": base_effective_steps,
                    "denoise": strength,
                    "width": generated.width,
                    "height": generated.height,
                },
            )
            base_candidates.append(
                {
                    "index": index + 1,
                    "seed": seed,
                    "image": generated.convert("RGB"),
                    "scheduler": scheduler_key,
                    "schedulerClass": scheduler_class,
                    "steps": steps,
                    "effectiveSteps": base_effective_steps,
                    "denoise": strength,
                }
            )

        def apply_refiner_stage(stage_label: str) -> None:
            nonlocal pipe, refiner_report
            if not refiner["enabled"]:
                return
            if refiner["backend"] == "diffusers":
                unload_pipeline(pipe, torch, stage_label)
                pipe = None
                debug_event(
                    "refiner-load-start",
                    {
                        "model": refiner["model"],
                        "loadKwargs": load_kwargs,
                        "placement": refiner["placement"],
                    },
                )
                refiner_pipe = load_pipeline(
                    StableDiffusionXLImg2ImgPipeline, refiner["model"], load_kwargs
                )
                refiner_scheduler_config = refiner_pipe.scheduler.config
                refiner_scheduler_key, refiner_scheduler_class = apply_scheduler(
                    refiner_pipe,
                    refiner["scheduler"],
                    *scheduler_classes,
                    scheduler_config=refiner_scheduler_config,
                )
                prepare_pipeline_for_device(
                    refiner_pipe,
                    device,
                    preferences,
                    "refiner",
                    prefer_cpu_offload=True,
                )
                if preferences.get("vaeTiling", True):
                    if hasattr(refiner_pipe, "vae") and hasattr(
                        refiner_pipe.vae, "enable_tiling"
                    ):
                        refiner_pipe.vae.enable_tiling()
                    elif hasattr(refiner_pipe, "enable_vae_tiling"):
                        refiner_pipe.enable_vae_tiling()
                if hasattr(refiner_pipe, "vae") and hasattr(
                    refiner_pipe.vae, "enable_slicing"
                ):
                    refiner_pipe.vae.enable_slicing()
                elif hasattr(refiner_pipe, "enable_vae_slicing"):
                    refiner_pipe.enable_vae_slicing()
                debug_event(
                    "refiner-load-complete",
                    {
                        "model": refiner["model"],
                        "pipelineClass": refiner_pipe.__class__.__name__,
                        "scheduler": refiner_scheduler_key,
                        "schedulerClass": refiner_scheduler_class,
                        "placement": refiner["placement"],
                    },
                )
                for candidate in base_candidates:
                    candidate["image"] = run_refiner_pass(
                        refiner_pipe,
                        torch,
                        refiner,
                        candidate["image"],
                        prompt,
                        negative_prompt,
                        cfg,
                        int(candidate["seed"]),
                        device,
                        int(candidate["index"]),
                    )
                    candidate["refiner"] = {
                        **refiner,
                        "status": "applied",
                        "scheduler": refiner_scheduler_key,
                        "schedulerClass": refiner_scheduler_class,
                    }
                    refiner_report["appliedCandidates"] += 1
                refiner_report["status"] = "applied"
                refiner_report["scheduler"] = refiner_scheduler_key
                refiner_report["schedulerClass"] = refiner_scheduler_class
                unload_pipeline(refiner_pipe, torch, "refiner")
                return
            if refiner["backend"] == "gguf":
                unload_pipeline(pipe, torch, stage_label)
                pipe = None
                debug_event(
                    "gguf-refiner-start",
                    {
                        "model": refiner["model"],
                        "runner": refiner.get("runner", ""),
                        "candidateCount": len(base_candidates),
                        "placement": refiner["placement"],
                    },
                )
                for candidate in base_candidates:
                    candidate["image"] = run_gguf_refiner_pass(
                        refiner,
                        candidate["image"],
                        prompt,
                        negative_prompt,
                        cfg,
                        int(candidate["seed"]),
                        job_dir,
                        int(candidate["index"]),
                    )
                    candidate["refiner"] = {
                        **refiner,
                        "status": "applied",
                    }
                    refiner_report["appliedCandidates"] += 1
                refiner_report["status"] = "applied"
                debug_event(
                    "gguf-refiner-complete",
                    {
                        "appliedCandidates": refiner_report["appliedCandidates"],
                        "placement": refiner["placement"],
                    },
                )

        if refiner["enabled"] and refiner["placement"] != "after-detail":
            apply_refiner_stage("base-inpaint")

        detail_precomputed_detections: dict[
            int,
            tuple[
                list[dict[str, Any]],
                int,
                int,
                dict[str, int],
                list[dict[str, Any]],
            ],
        ] = {}
        detail_has_regions = False
        if detail_pass.get("enabled"):
            detail_detectors, detail_pass_report = load_detail_detectors(detail_pass)
            if detail_pass_report.get("status") == "ready" and detail_detectors:
                for candidate in base_candidates:
                    candidate_index = int(candidate["index"])
                    detection = detected_detail_regions(
                        detail_detectors, candidate["image"], detail_pass, device
                    )
                    detail_precomputed_detections[candidate_index] = detection
                    if detection[0]:
                        detail_has_regions = True
                if not detail_has_regions:
                    debug_event(
                        "detail-pass-no-candidate-detections",
                        {
                            "candidateCount": len(base_candidates),
                            "detectors": [
                                {"region": detector["region"], "path": detector["path"]}
                                for detector in detail_detectors
                            ],
                        },
                    )
            if detail_has_regions and pipe is None:
                debug_event(
                    "detail-inpaint-reload-start",
                    {"model": model_id, "loadKwargs": load_kwargs},
                )
                pipe = load_pipeline(AutoPipelineForInpainting, model_id, load_kwargs)
                scheduler_config = pipe.scheduler.config
                prepare_pipeline_for_device(pipe, device, preferences, "detail-inpaint")
                if preferences.get("vaeTiling", True):
                    if hasattr(pipe, "vae") and hasattr(pipe.vae, "enable_tiling"):
                        pipe.vae.enable_tiling()
                    elif hasattr(pipe, "enable_vae_tiling"):
                        pipe.enable_vae_tiling()
                if hasattr(pipe, "vae") and hasattr(pipe.vae, "enable_slicing"):
                    pipe.vae.enable_slicing()
                elif hasattr(pipe, "enable_vae_slicing"):
                    pipe.enable_vae_slicing()
                debug_event(
                    "detail-inpaint-reload-complete",
                    {"model": model_id, "pipelineClass": pipe.__class__.__name__},
                )
        else:
            detail_detectors = []

        for candidate in base_candidates:
            candidate_index = int(candidate["index"])
            seed = int(candidate["seed"])
            generated = candidate["image"]
            candidate_detail_report = detail_pass_report
            if detail_pass.get("enabled"):
                generated, candidate_detail_report = run_detail_pass(
                    pipe,
                    torch,
                    scheduler_classes,
                    scheduler_config,
                    detail_detectors,
                    detail_pass,
                    detail_pass_report,
                    generated,
                    prompt,
                    negative_prompt,
                    cfg,
                    seed,
                    device,
                    job_dir,
                    candidate_index,
                    detail_precomputed_detections.get(candidate_index),
                )
            detail_pass_runs.append(candidate_detail_report)
            candidate["image"] = generated.convert("RGB")
            candidate["detailPass"] = candidate_detail_report

        if refiner["enabled"] and refiner["placement"] == "after-detail":
            apply_refiner_stage("detail-inpaint")

        for candidate in base_candidates:
            candidate_index = int(candidate["index"])
            seed = int(candidate["seed"])
            generated = candidate["image"]
            candidate_detail_report = candidate.get("detailPass", detail_pass_report)
            if generated.size != output_size:
                generated = generated.resize(output_size, Image.Resampling.LANCZOS)
            generated = apply_alpha_mask(generated, output_mask, edge_feather_px)

            candidate_id = f"candidate-{candidate_index}"
            image_path = job_dir / f"{candidate_id}.png"
            generated.save(image_path)
            debug_event(
                "candidate-complete",
                {
                    "candidate": candidate_index,
                    "seed": seed,
                    "imagePath": str(image_path),
                    "refiner": candidate.get("refiner", refiner_report),
                    "detailPass": candidate_detail_report,
                },
            )
            emit_progress(
                {
                    "type": "candidate",
                    "id": candidate_id,
                    "candidate": candidate_index,
                    "seed": seed,
                    "imagePath": str(image_path),
                }
            )
            candidates.append(
                {
                    "id": candidate_id,
                    "label": f"Candidate {candidate_index}",
                    "imagePath": str(image_path),
                    "maskPath": str(mask_path),
                    "metadata": {
                        "seed": seed,
                        "modelRole": operation,
                        "model": model_id,
                        "modelFormat": model_config["format"],
                        "modelAdapter": model_config["adapter"],
                        "scheduler": candidate["scheduler"],
                        "schedulerClass": candidate["schedulerClass"],
                        "steps": candidate["steps"],
                        "effectiveSteps": candidate["effectiveSteps"],
                        "denoise": candidate["denoise"],
                        "refiner": candidate.get("refiner", refiner_report),
                        "detailPass": candidate_detail_report,
                        "renderWidth": unpadded_render_size[0],
                        "renderHeight": unpadded_render_size[1],
                        "edgeFeatherPx": edge_feather_px,
                    },
                }
            )

        if device == "cuda":
            peak_vram_mb = int(torch.cuda.max_memory_allocated() / 1024 / 1024)

    except Exception as exc:  # noqa: BLE001
        debug_event(
            "generation-error",
            {"message": str(exc), "traceback": traceback.format_exc()},
        )
        if is_cuda_oom(exc):
            return fail(
                response_path,
                request_id,
                cuda_oom_message(
                    exc,
                    refiner=refiner,
                    detail_pass=detail_pass,
                    target_render_edge=target_render_edge,
                    candidate_count=candidate_count,
                ),
            )
        return fail(response_path, request_id, f"Diffusers generation failed: {exc}")

    elapsed_ms = int((time.monotonic() - started) * 1000)
    detail_pass_summary = dict(detail_pass_report)
    if detail_pass_runs:
        detail_pass_summary["candidates"] = detail_pass_runs
        detail_pass_summary["detectedRegions"] = sum(
            int(run.get("detectedRegions", 0)) for run in detail_pass_runs
        )
        detail_pass_summary["rawDetectedRegions"] = sum(
            int(run.get("rawDetectedRegions", 0)) for run in detail_pass_runs
        )
        detail_pass_summary["rejectedDetections"] = sum(
            int(run.get("rejectedDetections", 0)) for run in detail_pass_runs
        )
        detail_pass_summary["appliedRegions"] = sum(
            int(run.get("appliedRegions", 0)) for run in detail_pass_runs
        )
        detail_pass_summary["fallbackRegions"] = sum(
            int(run.get("fallbackRegions", 0)) for run in detail_pass_runs
        )
        statuses = {str(run.get("status", "")) for run in detail_pass_runs}
        if detail_pass_summary["appliedRegions"] > 0:
            detail_pass_summary["status"] = "applied"
        elif "no-detections" in statuses:
            detail_pass_summary["status"] = "no-detections"
        elif "failed" in statuses:
            detail_pass_summary["status"] = "failed"
    diagnostics: dict[str, Any] = {
        "elapsedMsec": elapsed_ms,
        "device": device,
        "inputWidth": output_size[0],
        "inputHeight": output_size[1],
        "renderWidth": unpadded_render_size[0],
        "renderHeight": unpadded_render_size[1],
        "paddedWidth": source.width,
        "paddedHeight": source.height,
        "targetRenderEdge": target_render_edge,
        "scheduler": scheduler_key,
        "schedulerClass": scheduler_class,
        "denoise": strength,
        "steps": steps,
        "effectiveSteps": base_effective_steps,
        "modelFormat": model_config["format"],
        "modelAdapter": model_config["adapter"],
        "refiner": refiner_report,
        "detailPass": detail_pass_summary,
        "edgeFeatherPx": edge_feather_px,
        "maskMin": int(mask_min),
        "maskMax": int(mask_max),
    }
    if peak_vram_mb is not None:
        diagnostics["peakVramMb"] = peak_vram_mb
    diagnostics.update(debug_diagnostics())

    debug_event(
        "job-complete",
        {
            "requestId": request_id,
            "candidateCount": len(candidates),
            "elapsedMsec": elapsed_ms,
            "diagnostics": diagnostics,
        },
    )

    write_json(
        response_path,
        response(
            request_id,
            "succeeded",
            (
                f"Generated {len(candidates)} candidate(s). "
                f"Detail pass applied to {detail_pass_summary['appliedRegions']} region(s)."
                if detail_pass_summary.get("status") == "applied"
                else f"Generated {len(candidates)} candidate(s). "
                "Detail pass requested, but no regions were detected."
                if detail_pass_summary.get("status") == "no-detections"
                else f"Generated {len(candidates)} candidate(s). "
                "Detail pass requested, but detector models are missing."
                if detail_pass_summary.get("status") == "models-missing"
                else f"Generated {len(candidates)} candidate(s). "
                "Detail pass requested, but detector dependency is unavailable."
                if detail_pass_summary.get("status") == "dependency-unavailable"
                else f"Generated {len(candidates)} candidate(s)."
            ),
            candidates=candidates,
            diagnostics=diagnostics,
            provenance={
                "backend": "diffusers",
                "schema": SCHEMA,
                "model": model_id,
                "modelFormat": model_config["format"],
                "modelAdapter": model_config["adapter"],
                "scheduler": scheduler_key,
            },
        ),
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main(sys.argv))
