#!/usr/bin/env bash
set -euo pipefail

UNDERPAINT_HOME="${UNDERPAINT_HOME:-$HOME/.underpaint}"
REPO="${UNDERPAINT_BIREFNET_REPO:-ZhengPeng7/BiRefNet}"
MODEL_DIR="${UNDERPAINT_BIREFNET_MODEL:-$UNDERPAINT_HOME/models/segmentation/birefnet}"
script_dir="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
repo_root="$(cd -- "$script_dir/../.." && pwd)"
hf_cli="${UNDERPAINT_HUGGINGFACE_CLI:-huggingface-cli}"

if ! command -v "$hf_cli" >/dev/null 2>&1 && [[ -x "$repo_root/.venv/bin/huggingface-cli" ]]; then
	hf_cli="$repo_root/.venv/bin/huggingface-cli"
fi

mkdir -p "$MODEL_DIR" "$UNDERPAINT_HOME/cache" "$UNDERPAINT_HOME/logs"

if ! command -v "$hf_cli" >/dev/null 2>&1; then
	echo "huggingface-cli is required. Install huggingface_hub in the Underpaint venv first." >&2
	echo "Try: .venv/bin/python -m pip install huggingface_hub" >&2
	exit 1
fi

echo "Downloading BiRefNet into:"
echo "  $MODEL_DIR"
echo "Repo:"
echo "  $REPO"

"$hf_cli" download "$REPO" \
	--local-dir "$MODEL_DIR" \
	--local-dir-use-symlinks False

echo
echo "BiRefNet is ready:"
echo "  $MODEL_DIR"
echo
echo "Run a bakeoff with:"
echo "  .venv/bin/python tools/ai/underpaint-mask-bakeoff.py /path/to/image.png --method birefnet"
