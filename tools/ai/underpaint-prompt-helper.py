#!/usr/bin/env python3
"""Prompt helper for Underpaint inpaint requests."""

from __future__ import annotations

import json
import os
import sys
import urllib.error
import urllib.request
import base64
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
            return compact_space(text[len(prefix) :])
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


def image_data_url(path: str) -> str | None:
    if not path:
        return None
    try:
        with open(path, "rb") as image_file:
            encoded = base64.b64encode(image_file.read()).decode("ascii")
    except OSError:
        return None
    return f"data:image/png;base64,{encoded}"


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
    if operation == "decomposition-region-label":
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
    return normalize_prompt(text)


def main() -> int:
    try:
        payload = json.loads(sys.stdin.read() or "{}")
    except json.JSONDecodeError as exc:
        print(json.dumps({"ok": False, "error": str(exc)}))
        return 2

    operation = str(payload.get("operation") or "")
    url = helper_url()
    if url:
        try:
            prompt = call_openai_compat(url, payload)
            if prompt:
                print(json.dumps({"ok": True, "prompt": prompt, "backend": url}))
                return 0
        except (OSError, urllib.error.URLError, json.JSONDecodeError, KeyError) as exc:
            if operation in {"decomposition-region-label", "inpaint-selection-explain"}:
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

    if operation in {"decomposition-region-label", "inpaint-selection-explain"}:
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
