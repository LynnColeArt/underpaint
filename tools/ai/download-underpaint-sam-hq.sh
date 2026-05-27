#!/usr/bin/env bash
set -euo pipefail

UNDERPAINT_HOME="${UNDERPAINT_HOME:-$HOME/.underpaint}"
REPO="${UNDERPAINT_SAM_HQ_REPO:-syscv-community/sam-hq-vit-base}"
MODEL_DIR="${UNDERPAINT_SAM_HQ_MODEL:-$UNDERPAINT_HOME/models/segmentation/sam-hq-vit-base}"
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

echo "Downloading HQ-SAM ViT Base into:"
echo "  $MODEL_DIR"
echo "Repo:"
echo "  $REPO"

"$hf_cli" download "$REPO" \
	--local-dir "$MODEL_DIR" \
	--local-dir-use-symlinks False

echo
echo "HQ-SAM ViT Base is ready:"
echo "  $MODEL_DIR"
echo
echo "Underpaint will use this path automatically for Object Decomposition"
echo "when the backend is set to Fine Detail (HQ-SAM)."
