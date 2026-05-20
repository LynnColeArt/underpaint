#!/usr/bin/env bash
set -euo pipefail

UNDERPAINT_HOME="${UNDERPAINT_HOME:-$HOME/.underpaint}"
MODEL_DIR="${UNDERPAINT_PROMPT_HELPER_MODEL_DIR:-$UNDERPAINT_HOME/models/prompt/qwen3.5-4b-abliterated-gguf}"
PROJECTOR_DIR="${UNDERPAINT_PROMPT_HELPER_PROJECTOR_DIR:-$UNDERPAINT_HOME/models/prompt/qwen3.5-4b-gguf}"
REPO="${UNDERPAINT_PROMPT_HELPER_REPO:-mradermacher/Qwen3.5-4B-abliterated-GGUF}"
QUANT_FILE="${UNDERPAINT_PROMPT_HELPER_QUANT_FILE:-Qwen3.5-4B-abliterated.Q4_K_M.gguf}"
PROJECTOR_REPO="${UNDERPAINT_PROMPT_HELPER_PROJECTOR_REPO:-unsloth/Qwen3.5-4B-GGUF}"
PROJECTOR_FILE="${UNDERPAINT_PROMPT_HELPER_MMPROJ_FILE:-mmproj-F16.gguf}"

mkdir -p "$MODEL_DIR" "$PROJECTOR_DIR" "$UNDERPAINT_HOME/cache" "$UNDERPAINT_HOME/logs"

if ! command -v huggingface-cli >/dev/null 2>&1; then
	echo "huggingface-cli is required. Install huggingface_hub or set the model path manually." >&2
	exit 2
fi

echo "Downloading Underpaint prompt helper model into:"
echo "  $MODEL_DIR"
echo
echo "Repo: $REPO"
echo "Model: $QUANT_FILE"
echo

huggingface-cli download "$REPO" "$QUANT_FILE" \
	--local-dir "$MODEL_DIR" \
	--local-dir-use-symlinks False

if [[ ! -f "$PROJECTOR_DIR/$PROJECTOR_FILE" ]]; then
	echo
	echo "Downloading compatible Qwen3.5 projector into:"
	echo "  $PROJECTOR_DIR"
	echo
	huggingface-cli download "$PROJECTOR_REPO" "$PROJECTOR_FILE" \
		--local-dir "$PROJECTOR_DIR" \
		--local-dir-use-symlinks False
fi

echo
echo "Done. Start the prompt helper with:"
echo "  tools/ai/run-underpaint-prompt-helper-server.sh"
