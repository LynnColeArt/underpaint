#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
UNDERPAINT_HOME="${UNDERPAINT_HOME:-$HOME/.underpaint}"
MODEL_DIR="${UNDERPAINT_PROMPT_HELPER_MODEL_DIR:-$UNDERPAINT_HOME/models/prompt/qwen3.5-4b-gguf}"
LLAMA_SERVER="${QWENCH_LLAMA_SERVER:-$HOME/.qwench/runtime/bin/llama-server}"
DEFAULT_MODEL_PATH="$MODEL_DIR/Qwen3.5-4B-UD-Q4_K_XL.gguf"
if [[ ! -f "$DEFAULT_MODEL_PATH" ]]; then
	DEFAULT_MODEL_PATH="$MODEL_DIR/Qwen3.5-4B-Q4_K_M.gguf"
fi
if [[ ! -f "$DEFAULT_MODEL_PATH" ]]; then
	DEFAULT_MODEL_PATH="$HOME/.qwench/models/qwen3-4b-instruct-q4_k_m.gguf"
fi
if [[ ! -f "$DEFAULT_MODEL_PATH" ]]; then
	DEFAULT_MODEL_PATH="$HOME/.qwench/models/qwen3.5-9b-instruct-q4_k_m.gguf"
fi
MODEL_PATH="${UNDERPAINT_PROMPT_HELPER_MODEL_PATH:-$DEFAULT_MODEL_PATH}"
DEFAULT_MMPROJ_PATH="$MODEL_DIR/mmproj-F16.gguf"
if [[ ! -f "$DEFAULT_MMPROJ_PATH" ]]; then
	DEFAULT_MMPROJ_PATH="$HOME/.qwench/models/mmproj-Qwen_Qwen3.5-9B-f16.gguf"
fi
MMPROJ_PATH="${UNDERPAINT_PROMPT_HELPER_MMPROJ:-$DEFAULT_MMPROJ_PATH}"
HOST="${UNDERPAINT_PROMPT_HELPER_HOST:-127.0.0.1}"
PORT="${UNDERPAINT_PROMPT_HELPER_PORT:-18080}"
ALIAS="${UNDERPAINT_PROMPT_HELPER_MODEL:-underpaint-prompt}"
CTX_SIZE="${UNDERPAINT_PROMPT_HELPER_CTX:-4096}"
GPU_LAYERS="${UNDERPAINT_PROMPT_HELPER_GPU_LAYERS:-0}"

if [[ ! -x "$LLAMA_SERVER" ]]; then
	echo "llama-server not found or not executable: $LLAMA_SERVER" >&2
	exit 2
fi
if [[ ! -f "$MODEL_PATH" ]]; then
	echo "Prompt helper model not found: $MODEL_PATH" >&2
	echo "Run tools/ai/download-underpaint-qwen35-4b.sh or set UNDERPAINT_PROMPT_HELPER_MODEL_PATH." >&2
	exit 2
fi

args=(
	"$LLAMA_SERVER"
	--model "$MODEL_PATH"
	--alias "$ALIAS"
	--host "$HOST"
	--port "$PORT"
	--ctx-size "$CTX_SIZE"
	--gpu-layers "$GPU_LAYERS"
)

if [[ "${UNDERPAINT_PROMPT_HELPER_DISABLE_MMPROJ:-0}" != "1" && -f "$MMPROJ_PATH" ]]; then
	args+=(--mmproj "$MMPROJ_PATH")
fi

echo "Starting Underpaint prompt helper at http://$HOST:$PORT/v1"
echo "Model: $MODEL_PATH"
if [[ "${args[*]}" == *"--mmproj"* ]]; then
	echo "Projector: $MMPROJ_PATH"
fi
echo
echo "Use with:"
echo "  UNDERPAINT_PROMPT_HELPER_URL=http://$HOST:$PORT/v1 tools/ai/run-underpaint-juggernaut.sh"
echo

cd "$ROOT_DIR"
exec "${args[@]}"
