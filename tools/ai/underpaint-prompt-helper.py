#!/usr/bin/env python3
"""Prompt helper for Underpaint inpaint requests."""

from __future__ import annotations

import json
import os
import sys
import urllib.error
import urllib.request
from typing import Any


SYSTEM_PROMPT = (
    "Rewrite the user's inpainting prompt into a rich, loaded image prompt for "
    "local diffusion inpainting. Preserve the user's subject and intent. Add "
    "helpful visual detail about texture, lighting, perspective, materials, "
    "edge blending, and natural integration with the surrounding image. Do not "
    "add unrelated subjects. Target about 150 characters, between 120 and 190 "
    "characters when possible. Return only the rewritten prompt."
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


def fallback_rewrite(payload: dict[str, Any]) -> str:
    prompt = compact_space(str(payload.get("prompt") or ""))
    if not prompt:
        prompt = "restore the selected area naturally"

    additions: list[str] = []
    lower = prompt.lower()
    if "match" not in lower and "lighting" not in lower:
        additions.append("matching the surrounding color, lighting, and shadows")
    if "texture" not in lower and "material" not in lower:
        additions.append("preserving believable texture, material detail, and depth")
    if "natural" not in lower and "seamless" not in lower:
        additions.append("with clean edges and a seamless natural blend")

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


def call_openai_compat(url: str, payload: dict[str, Any]) -> str:
    prompt = compact_space(str(payload.get("prompt") or ""))
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
    body = {
        "model": os.environ.get("UNDERPAINT_PROMPT_HELPER_MODEL", "local"),
        "temperature": 0.55,
        "max_tokens": 128,
        "messages": [
            {"role": "system", "content": SYSTEM_PROMPT},
            {
                "role": "user",
                "content": (
                    "Expand this inpainting prompt into one loaded diffusion "
                    "prompt, about 150 characters. Return only the prompt.\n"
                    + json.dumps(context, ensure_ascii=False)
                ),
            },
        ],
    }
    request = urllib.request.Request(
        f"{url}/chat/completions",
        data=json.dumps(body).encode("utf-8"),
        headers={"Content-Type": "application/json"},
        method="POST",
    )
    with urllib.request.urlopen(request, timeout=12) as response:
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

    url = helper_url()
    if url:
        try:
            prompt = call_openai_compat(url, payload)
            if prompt:
                print(json.dumps({"ok": True, "prompt": prompt, "backend": url}))
                return 0
        except (OSError, urllib.error.URLError, json.JSONDecodeError, KeyError) as exc:
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
