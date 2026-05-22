#!/usr/bin/env python3
"""Inspect Fooocus SDXL inpaint patch assets for Underpaint integration."""

from __future__ import annotations

import argparse
import hashlib
import json
import os
from pathlib import Path
from typing import Any

import torch
from safetensors import safe_open
from safetensors.torch import load_file as load_safetensors


DEFAULT_DIR = Path.home() / ".underpaint/models/inpaint/fooocus"
DEFAULT_HEAD = DEFAULT_DIR / "fooocus_inpaint_head.pth"
DEFAULT_PATCH = DEFAULT_DIR / "inpaint_v26.fooocus.patch"
DEFAULT_LAMA = DEFAULT_DIR / "fooocus_lama.safetensors"


def expand_path(value: str | Path) -> Path:
    return Path(os.path.expandvars(os.path.expanduser(str(value))))


def file_report(path: Path) -> dict[str, Any]:
    report: dict[str, Any] = {
        "path": str(path),
        "exists": path.is_file(),
    }
    if not path.is_file():
        return report
    report["sizeMb"] = round(path.stat().st_size / 1024 / 1024, 2)
    digest = hashlib.sha256()
    with path.open("rb") as handle:
        for chunk in iter(lambda: handle.read(1024 * 1024), b""):
            digest.update(chunk)
    report["sha256"] = digest.hexdigest()
    return report


def load_patch(path: Path) -> dict[str, Any]:
    try:
        return load_safetensors(str(path), device="cpu")
    except Exception:
        return torch.load(path, map_location="cpu", weights_only=True)


def tensor_shape(value: Any) -> Any:
    if isinstance(value, torch.Tensor):
        return list(value.shape)
    if isinstance(value, (list, tuple)):
        return [tensor_shape(item) for item in value]
    return type(value).__name__


def read_safetensors_keys(path: Path) -> list[str]:
    with safe_open(str(path), framework="pt", device="cpu") as handle:
        return list(handle.keys())


def checkpoint_key_report(checkpoint: Path, patch_keys: list[str]) -> dict[str, Any]:
    report: dict[str, Any] = {
        "path": str(checkpoint),
        "exists": checkpoint.is_file(),
    }
    if not checkpoint.is_file():
        return report
    if checkpoint.suffix != ".safetensors":
        report["error"] = "Only safetensors checkpoint key probing is supported."
        return report

    checkpoint_keys = set(read_safetensors_keys(checkpoint))
    patch_set = set(patch_keys)
    direct_matches = sorted(checkpoint_keys & patch_set)

    stripped_patch_keys = {
        key.removeprefix("model.diffusion_model."): key for key in patch_keys
    }
    stripped_matches = sorted(checkpoint_keys & set(stripped_patch_keys))
    model_prefixed_patch_keys = {f"model.{key}" for key in patch_keys}
    model_prefixed_patch_matches = sorted(checkpoint_keys & model_prefixed_patch_keys)
    prefixed_checkpoint_keys = {f"model.diffusion_model.{key}" for key in checkpoint_keys}
    prefixed_matches = sorted(prefixed_checkpoint_keys & patch_set)

    report.update(
        {
            "checkpointKeyCount": len(checkpoint_keys),
            "patchKeyCount": len(patch_keys),
            "directMatchCount": len(direct_matches),
            "strippedPatchMatchCount": len(stripped_matches),
            "modelPrefixedPatchMatchCount": len(model_prefixed_patch_matches),
            "prefixedCheckpointMatchCount": len(prefixed_matches),
            "sampleDirectMatches": direct_matches[:8],
            "sampleStrippedPatchMatches": stripped_matches[:8],
            "sampleModelPrefixedPatchMatches": model_prefixed_patch_matches[:8],
            "samplePrefixedCheckpointMatches": prefixed_matches[:8],
            "sampleCheckpointKeys": sorted(checkpoint_keys)[:8],
        }
    )
    return report


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--head", default=str(DEFAULT_HEAD))
    parser.add_argument("--patch", default=str(DEFAULT_PATCH))
    parser.add_argument("--lama", default=str(DEFAULT_LAMA))
    parser.add_argument("--checkpoint", help="Optional SDXL safetensors checkpoint to compare.")
    args = parser.parse_args()

    head_path = expand_path(args.head)
    patch_path = expand_path(args.patch)
    lama_path = expand_path(args.lama)

    report: dict[str, Any] = {
        "schema": "underpaint.fooocus-patch-probe.v1",
        "files": {
            "head": file_report(head_path),
            "patch": file_report(patch_path),
            "lama": file_report(lama_path),
        },
    }

    if not head_path.is_file() or not patch_path.is_file():
        report["status"] = "missing-files"
        print(json.dumps(report, indent=2, sort_keys=True))
        return 2

    head_state = torch.load(head_path, map_location="cpu", weights_only=True)
    patch_state = load_patch(patch_path)
    patch_keys = sorted(patch_state.keys())

    report["head"] = {
        "keyCount": len(head_state),
        "keys": sorted(head_state.keys()),
        "shapes": {key: tensor_shape(value) for key, value in sorted(head_state.items())},
    }
    report["patch"] = {
        "keyCount": len(patch_keys),
        "sampleKeys": patch_keys[:16],
        "sampleShapes": {
            key: tensor_shape(patch_state[key]) for key in patch_keys[:16]
        },
    }

    if args.checkpoint:
        report["checkpoint"] = checkpoint_key_report(expand_path(args.checkpoint), patch_keys)

    head_ok = report["head"]["shapes"].get("head") == [320, 5, 3, 3]
    report["status"] = "ok" if head_ok and patch_keys else "unexpected-shape"
    print(json.dumps(report, indent=2, sort_keys=True))
    return 0 if report["status"] == "ok" else 1


if __name__ == "__main__":
    raise SystemExit(main())
