#!/usr/bin/env python3
"""Diffusers worker for the Underpaint AI job contract."""

from __future__ import annotations

import json
import math
import os
import secrets
import sys
import time
from pathlib import Path
from typing import Any

from PIL import Image, ImageFilter


SCHEMA = "underpaint.ai-job.v1"
DEFAULT_MODEL = "diffusers/stable-diffusion-xl-1.0-inpainting-0.1"


def write_json(path: Path, payload: dict[str, Any]) -> None:
    path.write_text(json.dumps(payload, indent=2), encoding="utf-8")


def emit_progress(payload: dict[str, Any]) -> None:
    event = {"schema": SCHEMA, **payload}
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
    write_json(
        response_path,
        response(
            request_id,
            "failed",
            message,
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


def write_scene_separation_response(
    request: dict[str, Any],
    response_path: Path,
    request_id: str,
    job_dir: Path,
    started: float,
) -> int:
    source_path = asset_path(request, "source-image")
    if source_path is None:
        return fail(response_path, request_id, "source-image is required.")

    try:
        source = Image.open(source_path).convert("RGBA")
    except Exception as exc:  # noqa: BLE001
        return fail(response_path, request_id, f"Could not load source image: {exc}", 2)

    parameters = request.get("parameters", {})
    region_count = max(2, min(int(parameters.get("maxRegions", 5)), 8))
    min_region_area_pct = max(1, min(int(parameters.get("minRegionAreaPct", 3)), 20))
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
                    "modelRole": "photo-decomposition",
                    "model": "placeholder-luma-regions",
                    "regionIndex": index,
                    "regionCount": region_count,
                    "minRegionAreaPct": min_region_area_pct,
                },
            }
        )

    elapsed_ms = int((time.monotonic() - started) * 1000)
    write_json(
        response_path,
        response(
            request_id,
            "succeeded",
            f"Generated {len(candidates)} placeholder decomposition layer(s).",
            candidates=candidates,
            diagnostics={
                "elapsedMsec": elapsed_ms,
                "inputWidth": source.width,
                "inputHeight": source.height,
                "regionCount": region_count,
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
    if operation not in {"scene-separation", "inpaint", "generative-fill", "outpaint"}:
        return fail(
            response_path,
            request_id,
            f"Unsupported operation: {operation}",
            2,
        )
    if operation == "scene-separation":
        return write_scene_separation_response(
            request, response_path, request_id, job_dir, started
        )

    source_path = asset_path(request, "source-image")
    mask_path = asset_path(request, "mask")
    if source_path is None or mask_path is None:
        return fail(response_path, request_id, "source-image and mask are required.")

    try:
        source = Image.open(source_path).convert("RGB")
        output_size = source.size
        mask = load_mask(mask_path, source.size)
        output_mask = mask.copy()
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
    except Exception as exc:  # noqa: BLE001
        return fail(response_path, request_id, f"Could not load source assets: {exc}")

    try:
        import torch
        from diffusers import AutoPipelineForInpainting
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
    prompt = parameters.get("prompt") or "restore the selected region naturally"
    negative_prompt = parameters.get("negativePrompt") or None
    candidate_count = max(1, min(int(parameters.get("candidateCount", 1)), 4))
    cfg = float(parameters.get("cfg", 5.0))
    strength = float(parameters.get("denoise", 0.75))
    steps = int(parameters.get("steps") or 20)
    edge_feather_px = max(0, min(int(parameters.get("edgeFeatherPx", 24)), 256))
    preview_max_edge = max(64, min(int(parameters.get("previewMaxEdge", 256)), 512))
    preview_every_steps = int(
        parameters.get("previewEverySteps", max(4, steps // 5 if steps >= 5 else 1))
    )
    preview_every_steps = max(1, min(preview_every_steps, max(1, steps)))
    if strength > 0.0 and int(steps * strength) < 1:
        steps = max(steps, math.ceil(1.0 / strength))
    requested_seed = int(parameters.get("seed", -1))
    max_base_seed = (2**31 - 1) - candidate_count
    base_seed = (
        min(requested_seed, max_base_seed)
        if requested_seed >= 0
        else secrets.randbelow(max_base_seed + 1)
    )
    model_id = (
        os.environ.get("UNDERPAINT_INPAINT_MODEL")
        or parameters.get("model")
        or DEFAULT_MODEL
    )

    try:
        load_kwargs: dict[str, Any] = {
            "torch_dtype": dtype,
            "use_safetensors": True,
        }
        if dtype is torch.float16:
            load_kwargs["variant"] = "fp16"
        try:
            pipe = AutoPipelineForInpainting.from_pretrained(
                model_id,
                **load_kwargs,
            )
        except Exception as exc:  # noqa: BLE001
            if load_kwargs.pop("variant", None) == "fp16" and "variant=fp16" in str(
                exc
            ):
                pipe = AutoPipelineForInpainting.from_pretrained(
                    model_id,
                    **load_kwargs,
                )
            else:
                raise
        pipe.to(device)
        if preferences.get("vaeTiling", True):
            if hasattr(pipe, "vae") and hasattr(pipe.vae, "enable_tiling"):
                pipe.vae.enable_tiling()
            elif hasattr(pipe, "enable_vae_tiling"):
                pipe.enable_vae_tiling()
        if hasattr(pipe, "vae") and hasattr(pipe.vae, "enable_slicing"):
            pipe.vae.enable_slicing()
        elif hasattr(pipe, "enable_vae_slicing"):
            pipe.enable_vae_slicing()

        candidates: list[dict[str, Any]] = []
        peak_vram_mb = None
        if device == "cuda":
            torch.cuda.reset_peak_memory_stats()

        for index in range(candidate_count):
            seed = base_seed + index
            last_preview_step = 0

            def on_step_end(pipeline: Any, step_index: int, timestep: Any, callback_kwargs: dict[str, Any]) -> dict[str, Any]:
                nonlocal last_preview_step
                step = step_index + 1
                if step != steps and step - last_preview_step < preview_every_steps:
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
                            "steps": steps,
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

            generated = pipe(
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
                callback_on_step_end=on_step_end,
                callback_on_step_end_tensor_inputs=["latents"],
            ).images[0]
            generated = generated.crop(
                (0, 0, unpadded_render_size[0], unpadded_render_size[1])
            )
            if unpadded_render_size != output_size:
                generated = generated.resize(output_size, Image.Resampling.LANCZOS)
            generated = apply_alpha_mask(generated, output_mask, edge_feather_px)

            candidate_id = f"candidate-{index + 1}"
            image_path = job_dir / f"{candidate_id}.png"
            generated.save(image_path)
            emit_progress(
                {
                    "type": "candidate",
                    "id": candidate_id,
                    "candidate": index + 1,
                    "seed": seed,
                    "imagePath": str(image_path),
                }
            )
            candidates.append(
                {
                    "id": candidate_id,
                    "label": f"Candidate {index + 1}",
                    "imagePath": str(image_path),
                    "maskPath": str(mask_path),
                    "metadata": {
                        "seed": seed,
                        "modelRole": "inpaint",
                        "model": model_id,
                        "renderWidth": unpadded_render_size[0],
                        "renderHeight": unpadded_render_size[1],
                        "edgeFeatherPx": edge_feather_px,
                    },
                }
            )

        if device == "cuda":
            peak_vram_mb = int(torch.cuda.max_memory_allocated() / 1024 / 1024)

    except Exception as exc:  # noqa: BLE001
        return fail(response_path, request_id, f"Diffusers generation failed: {exc}")

    elapsed_ms = int((time.monotonic() - started) * 1000)
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
        "edgeFeatherPx": edge_feather_px,
        "maskMin": int(mask_min),
        "maskMax": int(mask_max),
    }
    if peak_vram_mb is not None:
        diagnostics["peakVramMb"] = peak_vram_mb

    write_json(
        response_path,
        response(
            request_id,
            "succeeded",
            f"Generated {len(candidates)} candidate(s).",
            candidates=candidates,
            diagnostics=diagnostics,
            provenance={
                "backend": "diffusers",
                "schema": SCHEMA,
                "model": model_id,
            },
        ),
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main(sys.argv))
