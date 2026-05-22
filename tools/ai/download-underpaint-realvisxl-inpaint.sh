#!/usr/bin/env bash
set -euo pipefail

UNDERPAINT_HOME="${UNDERPAINT_HOME:-$HOME/.underpaint}"
REPO="${UNDERPAINT_REALVISXL_INPAINT_REPO:-OzzyGT/RealVisXL_V4.0_inpainting}"
MODEL_DIR="${UNDERPAINT_REALVISXL_INPAINT_DIR:-$UNDERPAINT_HOME/models/inpaint/realvisxl-v4}"

mkdir -p "$MODEL_DIR" "$UNDERPAINT_HOME/cache" "$UNDERPAINT_HOME/logs"

if ! command -v huggingface-cli >/dev/null 2>&1; then
	echo "huggingface-cli is required. Install huggingface_hub or let Diffusers cache the model on first run." >&2
	exit 1
fi

echo "Downloading RealVisXL V4.0 Inpaint into:"
echo "  $MODEL_DIR"
echo "Repo:"
echo "  $REPO"

huggingface-cli download "$REPO" \
	--local-dir "$MODEL_DIR" \
	--local-dir-use-symlinks False

echo
echo "RealVisXL V4.0 Inpaint is ready:"
echo "  $MODEL_DIR"
echo
echo "Launch Underpaint with:"
echo "  UNDERPAINT_INPAINT_MODEL=\"$MODEL_DIR\" tools/ai/run-underpaint-realvisxl-inpaint.sh"
