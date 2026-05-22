#!/usr/bin/env bash
set -euo pipefail

UNDERPAINT_HOME="${UNDERPAINT_HOME:-$HOME/.underpaint}"
REPO="${UNDERPAINT_REFINER_GGUF_REPO:-gpustack/stable-diffusion-xl-refiner-1.0-GGUF}"
QUANT_FILE="${UNDERPAINT_REFINER_GGUF_FILE:-stable-diffusion-xl-refiner-1.0-Q4_1.gguf}"
MODEL_DIR="${UNDERPAINT_REFINER_GGUF_DIR:-$UNDERPAINT_HOME/models/refiner/stable-diffusion-xl-refiner-1.0-GGUF}"

mkdir -p "$MODEL_DIR" "$UNDERPAINT_HOME/cache" "$UNDERPAINT_HOME/logs"

if ! command -v huggingface-cli >/dev/null 2>&1; then
	echo "huggingface-cli is required. Install huggingface_hub or set the model path manually." >&2
	exit 1
fi

echo "Downloading Underpaint SDXL refiner GGUF into:"
echo "  $MODEL_DIR"
echo "Repo:"
echo "  $REPO"
echo "File:"
echo "  $QUANT_FILE"

huggingface-cli download "$REPO" "$QUANT_FILE" \
	--local-dir "$MODEL_DIR" \
	--local-dir-use-symlinks False

echo
echo "Refiner GGUF ready:"
echo "  $MODEL_DIR/$QUANT_FILE"
echo
echo "To use it with a future GGUF image runner:"
echo "  UNDERPAINT_REFINER_BACKEND=gguf \\"
echo "  UNDERPAINT_REFINER_GGUF_MODEL=\"$MODEL_DIR/$QUANT_FILE\" \\"
echo "  UNDERPAINT_GGUF_REFINER_WORKER=/path/to/underpaint-gguf-refiner-worker \\"
echo "  tools/ai/run-underpaint-juggernaut.sh"
