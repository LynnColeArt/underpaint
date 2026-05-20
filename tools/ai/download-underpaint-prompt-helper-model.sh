#!/usr/bin/env bash
set -euo pipefail

UNDERPAINT_HOME="${UNDERPAINT_HOME:-$HOME/.underpaint}"
REPO="${UNDERPAINT_PROMPT_HELPER_REPO:?Set UNDERPAINT_PROMPT_HELPER_REPO, for example owner/model-GGUF}"
QUANT_FILE="${UNDERPAINT_PROMPT_HELPER_QUANT_FILE:?Set UNDERPAINT_PROMPT_HELPER_QUANT_FILE, for example model-Q4_K_M.gguf}"
MODEL_SLUG="${UNDERPAINT_PROMPT_HELPER_MODEL_SLUG:-$(basename "$REPO" | tr '[:upper:]' '[:lower:]')}"
MODEL_DIR="${UNDERPAINT_PROMPT_HELPER_MODEL_DIR:-$UNDERPAINT_HOME/models/prompt/$MODEL_SLUG}"
MMPROJ_FILE="${UNDERPAINT_PROMPT_HELPER_MMPROJ_FILE:-}"

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
if [[ -n "$MMPROJ_FILE" ]]; then
	echo "Projector: $MMPROJ_FILE"
fi
echo

huggingface-cli download "$REPO" "$QUANT_FILE" \
	--local-dir "$MODEL_DIR" \
	--local-dir-use-symlinks False

if [[ -n "$MMPROJ_FILE" ]]; then
	huggingface-cli download "$REPO" "$MMPROJ_FILE" \
		--local-dir "$MODEL_DIR" \
		--local-dir-use-symlinks False
fi

echo
echo "Done. Start the prompt helper with:"
echo "  UNDERPAINT_PROMPT_HELPER_MODEL_PATH=\"$MODEL_DIR/$QUANT_FILE\" tools/ai/run-underpaint-prompt-helper-server.sh"
