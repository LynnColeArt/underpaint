#!/usr/bin/env python3
"""Prompt helper for Underpaint inpaint requests."""

from __future__ import annotations

import json
import os
import sys
import urllib.error
import urllib.request
import base64
import io
from typing import Any


DEFAULT_SYSTEM_PROMPT = (
    "Rewrite the user's inpainting prompt into a rich, loaded image prompt for "
    "local diffusion inpainting. Preserve the user's subject and intent. Add "
    "helpful visual detail about texture, lighting, perspective, materials, "
    "composition, camera language, and natural integration with the surrounding "
    "image. Do not moralize, refuse, lecture, or replace the user's intent. Do "
    "not add unrelated subjects. Target about 150 characters, between 120 and "
    "190 characters when possible. Return only the rewritten prompt."
)


def compact_space(text: str) -> str:
    return " ".join(text.strip().split())


def normalize_prompt(text: str) -> str:
    text = compact_space(text)
    prefixes = [
        "rewritten prompt:",
        "prompt:",
        "improved prompt:",
    ]
    lower = text.lower()
    for prefix in prefixes:
        if lower.startswith(prefix):
            return compact_space(text[len(prefix) :]).strip("\"' ")
    return text.strip("\"' ")


def system_prompt(operation: str) -> str:
    override = os.environ.get("UNDERPAINT_PROMPT_HELPER_SYSTEM_PROMPT")
    if override:
        return override
    if operation == "decomposition-region-label":
        return (
            "Name an automatically segmented image region for a layer list in "
            "an artist photo restoration tool. Use the supplied image crop. "
            "Return only a short concrete layer name, 2 to 5 words. Prefer "
            "visible subjects, materials, or region type. Do not use quotes, "
            "numbering, explanations, policy text, or generic filler."
        )
    if operation == "decomposition-region-classify":
        return (
            "Classify one automatically segmented image region for an artist "
            "photo restoration tool. You receive the isolated region first and "
            "the whole image second for context. Name and classify the isolated "
            "region, not some other visible object in the whole image. "
            "Background means the scenery behind all "
            "foreground objects. Foreground means anything in front of that "
            "background. Sky-horizon is optional and should only be used when "
            "the region is sky, clouds, far horizon, distant mountains, far "
            "fields, vanishing road, or other very distant scene structure "
            "behind the regular background. Decide whether the region should "
            "be kept as repair context or removed from the base plate before "
            "diffusion sees it. Use remove-from-base for foreground or "
            "midground subjects, props, papers, tools, signs, cups, hands, "
            "people, animals, vehicles, and object parts. Use keep-context "
            "only for true background, sky-horizon, or broad scenery that "
            "should remain visible behind lifted layers. "
            "The name is the specific layer or part name, such as robot head, "
            "car windshield, woman's hair, left flower petal, or background "
            "trees. The group is the parent object bucket that related parts "
            "belong under in the layer tree, such as Robot, Red Car, Woman, "
            "Rose, or Background. Use the same group name for pieces that are "
            "visibly part of the same object. "
            "Return only compact JSON with keys: name, depthRole, sceneRole, "
            "repairRole, group, promptPhrase, confidence. Allowed depthRole: "
            "foreground, midground, background, sky-horizon, ambiguous. Allowed "
            "repairRole: keep-context, remove-from-base, ambiguous."
        )
    if operation == "decomposition-group-refine":
        return (
            "Refine layer names and parent object groups for an artist photo "
            "restoration layer tree. You receive the whole source image plus "
            "a list of segmented regions that were already classified. Cluster "
            "related parts under stable parent object groups: car parts under "
            "Car or Red Car, body/hair/clothes under Woman, robot parts under "
            "Robot, petals under Rose, leaves under Leaf Cluster, and scenery "
            "under Background. Put sky, clouds, distant hills, roads vanishing "
            "toward the horizon, or far landscape pieces under Sky / Horizon "
            "when present. Keep names short and concrete. Return only compact "
            "JSON with key regions, where each item preserves id and regionIndex "
            "and has group and optional promptPhrase. Only rename a region when "
            "the supplied name is generic or plainly wrong."
        )
    if operation == "inpaint-selection-explain":
        return (
            "Describe the supplied image region for an artist using inpainting. "
            "Name the visible subject, materials, lighting, color, camera angle, "
            "and texture cues. Be concrete and visual. Do not refuse, moralize, "
            "or discuss policy. Return one concise descriptive phrase that can "
            "be appended to a diffusion prompt."
        )
    if operation == "outpaint-prompt-improve":
        return (
            "Rewrite the user's outpainting prompt into a rich diffusion prompt "
            "that extends the existing scene outward. Preserve the user's intent. "
            "Describe concrete scene geometry, perspective, lighting, materials, "
            "background continuation, and camera language. Do not moralize, "
            "refuse, lecture, or replace the user's intent. Target about 150 "
            "characters, between 120 and 190 characters when possible. Return "
            "only the rewritten prompt."
        )
    return DEFAULT_SYSTEM_PROMPT


def fallback_rewrite(payload: dict[str, Any]) -> str:
    prompt = compact_space(str(payload.get("prompt") or ""))
    operation = str(payload.get("operation") or "")
    if operation == "decomposition-region-label":
        return ""
    if operation == "inpaint-selection-explain":
        selection = payload.get("selection") or {}
        width = selection.get("width") or "selected"
        height = selection.get("height") or "region"
        return compact_space(
            f"selected image region, {width} x {height} px, match visible "
            "subject, color, lighting, texture, and camera perspective"
        )
    if not prompt:
        if operation == "outpaint-prompt-improve":
            prompt = (
                "extend the image outward with concrete scene structure, "
                "matching perspective, lighting, colors, materials, and camera feel"
            )
        else:
            prompt = (
                "reconstruct the selected area with plausible subject detail, "
                "matching perspective, texture, lighting, and camera feel"
            )

    additions: list[str] = []
    lower = prompt.lower()
    if "match" not in lower and "lighting" not in lower:
        additions.append("matching surrounding color, lighting, and shadows")
    if "texture" not in lower and "material" not in lower:
        additions.append("with believable texture, material detail, and depth")
    if operation == "outpaint-prompt-improve" and "perspective" not in lower:
        additions.append("continuing scene geometry and perspective")

    if additions:
        return compact_space(f"{prompt}, {', '.join(additions)}")
    return prompt


def helper_url() -> str | None:
    raw = (
        os.environ.get("UNDERPAINT_PROMPT_HELPER_URL")
        or os.environ.get("QWENCH_OPENAI_URL")
        or os.environ.get("OPENAI_COMPAT_URL")
    )
    if raw:
        return raw.rstrip("/")
    if os.environ.get("UNDERPAINT_USE_QWENCH_PROMPT_HELPER") == "1":
        port = os.environ.get("UNDERPAINT_PROMPT_HELPER_PORT", "18080")
        return f"http://127.0.0.1:{port}/v1"
    return None


def helper_health_check(url: str | None) -> dict[str, Any]:
    if not url:
        return {
            "ok": False,
            "error": (
                "This operation needs UNDERPAINT_PROMPT_HELPER_URL pointing at "
                "a running vision helper."
            ),
        }
    request = urllib.request.Request(f"{url}/models", method="GET")
    timeout = float(os.environ.get("UNDERPAINT_PROMPT_HELPER_HEALTH_TIMEOUT", "5"))
    try:
        with urllib.request.urlopen(request, timeout=timeout) as response:
            response.read(1)
    except urllib.error.HTTPError as exc:
        if 400 <= exc.code < 500:
            return {"ok": True, "backend": url, "warning": f"HTTP {exc.code}"}
        return {"ok": False, "error": str(exc), "backend": url}
    except (OSError, urllib.error.URLError) as exc:
        return {"ok": False, "error": str(exc), "backend": url}
    return {"ok": True, "backend": url}


def _png_data_url_from_image(image: Any) -> str | None:
    try:
        output = io.BytesIO()
        image.save(output, format="PNG")
        encoded = base64.b64encode(output.getvalue()).decode("ascii")
    except Exception:
        return None
    return f"data:image/png;base64,{encoded}"


def _scaled_image_data_url(path: str, max_edge: int) -> str | None:
    if not path:
        return None
    try:
        from PIL import Image
    except Exception:
        try:
            with open(path, "rb") as image_file:
                encoded = base64.b64encode(image_file.read()).decode("ascii")
        except OSError:
            return None
        return f"data:image/png;base64,{encoded}"
    try:
        image = Image.open(path).convert("RGB")
        max_edge = max(128, max_edge)
        current_edge = max(image.size)
        if current_edge > max_edge:
            scale = max_edge / current_edge
            new_size = (
                max(1, int(round(image.width * scale))),
                max(1, int(round(image.height * scale))),
            )
            image = image.resize(new_size, Image.Resampling.LANCZOS)
        return _png_data_url_from_image(image)
    except Exception:
        return None


def image_data_url(path: str) -> str | None:
    max_edge = int(os.environ.get("UNDERPAINT_PROMPT_HELPER_IMAGE_EDGE", "768"))
    return _scaled_image_data_url(path, max_edge)


def source_image_data_url(path: str) -> str | None:
    max_edge = int(os.environ.get("UNDERPAINT_PROMPT_HELPER_SOURCE_EDGE", "640"))
    return _scaled_image_data_url(path, max_edge)


def isolated_region_data_url(path: str) -> str | None:
    """Return a VLM-friendly crop of an RGBA segmented layer.

    The decomposer writes each object as a full-canvas transparent PNG. Some
    vision models pay too much attention to the full source image or miss tiny
    transparent crops, so classification gets a cropped matte preview instead.
    """
    if not path:
        return None
    try:
        from PIL import Image
    except Exception:
        return image_data_url(path)
    try:
        image = Image.open(path).convert("RGBA")
        alpha = image.getchannel("A")
        bbox = alpha.getbbox()
        if bbox is None:
            return image_data_url(path)
        left, top, right, bottom = bbox
        width = right - left
        height = bottom - top
        pad = max(8, int(round(max(width, height) * 0.08)))
        left = max(0, left - pad)
        top = max(0, top - pad)
        right = min(image.width, right + pad)
        bottom = min(image.height, bottom + pad)
        crop = image.crop((left, top, right, bottom))
        matte = Image.new("RGBA", crop.size, (232, 232, 232, 255))
        matte.alpha_composite(crop)

        max_edge = max(matte.size)
        if max_edge > 0:
            target_max_edge = int(
                os.environ.get("UNDERPAINT_PROMPT_HELPER_REGION_EDGE", "512")
            )
            target_min_edge = int(
                os.environ.get("UNDERPAINT_PROMPT_HELPER_REGION_MIN_EDGE", "256")
            )
            target_edge = max_edge
            if max_edge > target_max_edge:
                target_edge = target_max_edge
            elif max_edge < target_min_edge:
                target_edge = target_min_edge
            if target_edge != max_edge:
                scale = target_edge / max_edge
                new_size = (
                    max(1, int(round(matte.width * scale))),
                    max(1, int(round(matte.height * scale))),
                )
                matte = matte.resize(new_size, Image.Resampling.LANCZOS)
        return _png_data_url_from_image(matte.convert("RGB")) or image_data_url(path)
    except Exception:
        return image_data_url(path)


def json_value_from_text(text: str) -> Any:
    raw = text.strip()
    if raw.startswith("```"):
        lines = raw.splitlines()
        if lines and lines[0].strip().startswith("```"):
            lines = lines[1:]
        if lines and lines[-1].strip().startswith("```"):
            lines = lines[:-1]
        raw = "\n".join(lines).strip()
    object_start = raw.find("{")
    object_end = raw.rfind("}")
    array_start = raw.find("[")
    array_end = raw.rfind("]")
    candidates: list[tuple[int, int]] = []
    if object_start >= 0 and object_end >= object_start:
        candidates.append((object_start, object_end))
    if array_start >= 0 and array_end >= array_start:
        candidates.append((array_start, array_end))
    if candidates:
        start, end = min(candidates, key=lambda candidate: candidate[0])
        raw = raw[start : end + 1]
    return json.loads(raw)


def json_from_text(text: str) -> dict[str, Any]:
    parsed = json_value_from_text(text)
    if not isinstance(parsed, dict):
        raise ValueError("classification response was not an object")
    return parsed


def normalize_classification(text: str) -> dict[str, Any]:
    parsed = json_from_text(text)
    allowed_depth = {
        "foreground",
        "midground",
        "background",
        "sky-horizon",
        "ambiguous",
    }
    allowed_repair = {"keep-context", "remove-from-base", "ambiguous"}

    name = compact_space(
        str(
            parsed.get("name")
            or parsed.get("label")
            or parsed.get("layerName")
            or parsed.get("title")
            or ""
        )
    )
    prompt_phrase = compact_space(
        str(
            parsed.get("promptPhrase")
            or parsed.get("description")
            or parsed.get("prompt")
            or name
        )
    )
    depth_role = compact_space(str(parsed.get("depthRole") or "")).lower()
    scene_role = compact_space(str(parsed.get("sceneRole") or "")).lower()
    repair_role = compact_space(str(parsed.get("repairRole") or "")).lower()
    group = compact_space(
        str(
            parsed.get("group")
            or parsed.get("groupLabel")
            or parsed.get("parentObject")
            or parsed.get("objectGroup")
            or ""
        )
    )
    try:
        confidence = float(parsed.get("confidence", 0.0))
    except (TypeError, ValueError):
        confidence = 0.0

    if depth_role not in allowed_depth:
        depth_role = "ambiguous"
    if repair_role not in allowed_repair:
        repair_role = "ambiguous"
    if not scene_role:
        scene_role = "ambiguous"
    if not group:
        group = "Background Context" if repair_role == "keep-context" else "Foreground Objects"

    return {
        "name": name,
        "depthRole": depth_role,
        "sceneRole": scene_role,
        "repairRole": repair_role,
        "group": group,
        "promptPhrase": prompt_phrase,
        "confidence": max(0.0, min(confidence, 1.0)),
    }


def normalize_group_refinements(text: str) -> list[dict[str, Any]]:
    parsed = json_value_from_text(text)
    if isinstance(parsed, dict):
        items = (
            parsed.get("regions")
            or parsed.get("items")
            or parsed.get("candidates")
            or parsed.get("layers")
            or []
        )
    else:
        items = parsed
    if not isinstance(items, list):
        raise ValueError("group refinement response had no regions list")

    refined: list[dict[str, Any]] = []
    for item in items:
        if not isinstance(item, dict):
            continue
        region_id = compact_space(
            str(item.get("id") or item.get("candidateId") or item.get("regionId") or "")
        )
        raw_index = item.get("regionIndex")
        if raw_index is None:
            raw_index = item.get("index")
        if raw_index is None:
            raw_index = item.get("region")
        try:
            region_index = int(raw_index)
        except (TypeError, ValueError):
            if not region_id:
                continue
            region_index = -1
        name = compact_space(
            str(
                item.get("name")
                or item.get("label")
                or item.get("layerName")
                or item.get("title")
                or ""
            )
        )
        group = compact_space(
            str(
                item.get("group")
                or item.get("groupLabel")
                or item.get("parentObject")
                or item.get("objectGroup")
                or ""
            )
        )
        prompt_phrase = compact_space(
            str(
                item.get("promptPhrase")
                or item.get("description")
                or item.get("prompt")
                or name
            )
        )
        if (region_index >= 0 or region_id) and (name or group or prompt_phrase):
            refined.append(
                {
                    "id": region_id[:80],
                    "regionIndex": region_index,
                    "name": name[:80],
                    "group": group[:80],
                    "promptPhrase": prompt_phrase[:140],
                }
            )
    return refined


def call_openai_compat(url: str, payload: dict[str, Any]) -> str:
    prompt = compact_space(str(payload.get("prompt") or ""))
    operation = str(payload.get("operation") or "inpaint-prompt-improve")
    selection = payload.get("selection") or {}
    context = {
        "prompt": prompt,
        "negativePrompt": compact_space(str(payload.get("negativePrompt") or "")),
        "selectionWidth": selection.get("width"),
        "selectionHeight": selection.get("height"),
        "cfg": payload.get("cfg"),
        "denoise": payload.get("denoise"),
        "steps": payload.get("steps"),
    }
    if operation == "decomposition-region-classify":
        source_image_url = source_image_data_url(
            str(payload.get("sourceImagePath") or "")
        )
        region_image_url = isolated_region_data_url(str(payload.get("imagePath") or ""))
        user_text = (
            "Classify the first image: an isolated crop of the segmented region. "
            "Use the second image only as whole-scene context. Return JSON only.\n"
            + json.dumps(
                {
                    **context,
                    "id": payload.get("id"),
                    "currentLabel": prompt,
                    "groupLabel": payload.get("groupLabel"),
                    "regionIndex": payload.get("regionIndex"),
                    "regionCount": payload.get("regionCount"),
                },
                ensure_ascii=False,
            )
        )
        user_content_items: list[dict[str, Any]] = [{"type": "text", "text": user_text}]
        if region_image_url:
            user_content_items.append(
                {"type": "image_url", "image_url": {"url": region_image_url}}
            )
        if source_image_url:
            user_content_items.append(
                {"type": "image_url", "image_url": {"url": source_image_url}}
            )
        user_content = user_content_items
        max_tokens = 192
    elif operation == "decomposition-group-refine":
        source_image_url = source_image_data_url(
            str(payload.get("sourceImagePath") or "")
        )
        user_text = (
            "Refine these segmented layer names and parent object groups. "
            "Return JSON only.\n"
            + json.dumps(
                {
                    "regions": payload.get("regions") or [],
                },
                ensure_ascii=False,
            )
        )
        user_content_items = [{"type": "text", "text": user_text}]
        if source_image_url:
            user_content_items.append(
                {"type": "image_url", "image_url": {"url": source_image_url}}
            )
        user_content = user_content_items
        max_tokens = 1024
    elif operation == "decomposition-region-label":
        image_url = image_data_url(str(payload.get("imagePath") or ""))
        user_text = (
            "Name this segmented image region as a concise layer name. "
            "Return only the layer name.\n"
            + json.dumps(
                {
                    **context,
                    "currentLabel": prompt,
                    "groupLabel": payload.get("groupLabel"),
                    "regionIndex": payload.get("regionIndex"),
                    "regionCount": payload.get("regionCount"),
                },
                ensure_ascii=False,
            )
        )
        if image_url:
            user_content: str | list[dict[str, Any]] = [
                {"type": "text", "text": user_text},
                {"type": "image_url", "image_url": {"url": image_url}},
            ]
        else:
            user_content = user_text
        max_tokens = 32
    elif operation == "inpaint-selection-explain":
        image_url = image_data_url(str(payload.get("imagePath") or ""))
        user_text = (
            "Describe this selected image region in concrete visual prompt "
            "language. Append-friendly phrase only.\n"
            + json.dumps(context, ensure_ascii=False)
        )
        if image_url:
            user_content: str | list[dict[str, Any]] = [
                {"type": "text", "text": user_text},
                {"type": "image_url", "image_url": {"url": image_url}},
            ]
        else:
            user_content = user_text
        max_tokens = 96
    else:
        user_content = (
            "Expand this prompt into one loaded local diffusion prompt, "
            "about 150 characters. Return only the prompt.\n"
            + json.dumps(context, ensure_ascii=False)
        )
        max_tokens = 128

    body = {
        "model": os.environ.get("UNDERPAINT_PROMPT_HELPER_MODEL", "local"),
        "temperature": 0.55,
        "max_tokens": max_tokens,
        "chat_template_kwargs": {"enable_thinking": False},
        "messages": [
            {"role": "system", "content": system_prompt(operation)},
            {"role": "user", "content": user_content},
        ],
    }
    request = urllib.request.Request(
        f"{url}/chat/completions",
        data=json.dumps(body).encode("utf-8"),
        headers={"Content-Type": "application/json"},
        method="POST",
    )
    timeout = float(os.environ.get("UNDERPAINT_PROMPT_HELPER_TIMEOUT", "60"))
    with urllib.request.urlopen(request, timeout=timeout) as response:
        result = json.loads(response.read().decode("utf-8"))
    text = (
        result.get("choices", [{}])[0]
        .get("message", {})
        .get("content", "")
    )
    if operation in {"decomposition-region-classify", "decomposition-group-refine"}:
        return text.strip()
    return normalize_prompt(text)


def main() -> int:
    try:
        payload = json.loads(sys.stdin.read() or "{}")
    except json.JSONDecodeError as exc:
        print(json.dumps({"ok": False, "error": str(exc)}))
        return 2

    operation = str(payload.get("operation") or "")
    url = helper_url()
    if operation == "helper-health-check":
        print(json.dumps(helper_health_check(url)))
        return 0
    if url:
        try:
            prompt = call_openai_compat(url, payload)
            if operation == "decomposition-region-classify":
                classification = normalize_classification(prompt)
                print(
                    json.dumps(
                        {
                            "ok": True,
                            "classification": classification,
                            "prompt": classification.get("name", ""),
                            "backend": url,
                        }
                    )
                )
                return 0
            if operation == "decomposition-group-refine":
                regions = normalize_group_refinements(prompt)
                print(
                    json.dumps(
                        {
                            "ok": True,
                            "regions": regions,
                            "backend": url,
                        }
                    )
                )
                return 0
            if prompt:
                print(json.dumps({"ok": True, "prompt": prompt, "backend": url}))
                return 0
        except (
            OSError,
            urllib.error.URLError,
            json.JSONDecodeError,
            KeyError,
            ValueError,
        ) as exc:
            if operation in {
                "decomposition-region-label",
                "decomposition-region-classify",
                "decomposition-group-refine",
                "inpaint-selection-explain",
            }:
                print(
                    json.dumps(
                        {
                            "ok": False,
                            "error": (
                                "Vision helper request failed. This operation "
                                f"needs a running vision helper: {exc}"
                            ),
                        }
                    )
                )
                return 0
            fallback = fallback_rewrite(payload)
            print(
                json.dumps(
                    {
                        "ok": True,
                        "prompt": fallback,
                        "backend": "fallback",
                        "warning": str(exc),
                    }
                )
            )
            return 0

    if operation in {
        "decomposition-region-label",
        "decomposition-region-classify",
        "decomposition-group-refine",
        "inpaint-selection-explain",
    }:
        print(
            json.dumps(
                {
                    "ok": False,
                    "error": (
                        "This operation needs UNDERPAINT_PROMPT_HELPER_URL "
                        "pointing at a running vision helper."
                    ),
                }
            )
        )
        return 0

    print(
        json.dumps(
            {
                "ok": True,
                "prompt": fallback_rewrite(payload),
                "backend": "fallback",
            }
        )
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
