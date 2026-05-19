#!/usr/bin/env bash
set -euo pipefail

UNDERPAINT_HOME="${UNDERPAINT_HOME:-$HOME/.underpaint}"
MODEL_DIR="${UNDERPAINT_PROMPT_HELPER_MODEL_DIR:-$UNDERPAINT_HOME/models/prompt/qwen3.5-4b-gguf}"
REPO="${UNDERPAINT_PROMPT_HELPER_REPO:-unsloth/Qwen3.5-4B-GGUF}"
QUANT_FILE="${UNDERPAINT_PROMPT_HELPER_QUANT_FILE:-Qwen3.5-4B-UD-Q4_K_XL.gguf}"
MMPROJ_FILE="${UNDERPAINT_PROMPT_HELPER_MMPROJ_FILE:-mmproj-F16.gguf}"

mkdir -p "$MODEL_DIR" "$UNDERPAINT_HOME/cache" "$UNDERPAINT_HOME/logs"

if ! command -v huggingface-cli >/dev/null 2>&1; then
	echo "huggingface-cli is required. Install huggingface_hub or set the model path manually." >&2
	exit 2
fi

echo "Downloading Underpaint prompt helper model into:"
echo "  $MODEL_DIR"
echo
echo "Repo: $REPO"
echo "Model: $QUANT_FILE"
echo "Projector: $MMPROJ_FILE"
echo

huggingface-cli download "$REPO" "$QUANT_FILE" \
	--local-dir "$MODEL_DIR" \
	--local-dir-use-symlinks False

huggingface-cli download "$REPO" "$MMPROJ_FILE" \
	--local-dir "$MODEL_DIR" \
	--local-dir-use-symlinks False

echo
echo "Done. Start the prompt helper with:"
echo "  tools/ai/run-underpaint-prompt-helper-server.sh"
