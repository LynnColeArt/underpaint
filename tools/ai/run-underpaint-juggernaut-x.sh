#!/usr/bin/env bash
set -euo pipefail

script_dir="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
repo_root="$(cd -- "$script_dir/../.." && pwd)"

export UNDERPAINT_INPAINT_MODEL_ID="${UNDERPAINT_INPAINT_MODEL_ID:-juggernaut-x-v10-single-file}"
export UNDERPAINT_INPAINT_MODEL_FORMAT="${UNDERPAINT_INPAINT_MODEL_FORMAT:-single_file_sdxl}"
export UNDERPAINT_INPAINT_MODEL="${UNDERPAINT_INPAINT_MODEL:-$HOME/.underpaint/models/checkpoints/juggernaut-x-v10/Juggernaut-X-RunDiffusion-NSFW.safetensors}"
export UNDERPAINT_AI_CPU_OFFLOAD="${UNDERPAINT_AI_CPU_OFFLOAD:-1}"
export UNDERPAINT_AI_WORKER="$repo_root/tools/ai/run-diffusers-worker.sh"
prompt_helper_url_was_set="${UNDERPAINT_PROMPT_HELPER_URL:-}"
export UNDERPAINT_PROMPT_HELPER_URL="${UNDERPAINT_PROMPT_HELPER_URL:-http://127.0.0.1:${UNDERPAINT_PROMPT_HELPER_PORT:-18080}/v1}"

if [[ ! -f "$UNDERPAINT_INPAINT_MODEL" ]]; then
	echo "Juggernaut X checkpoint not found:" >&2
	echo "  $UNDERPAINT_INPAINT_MODEL" >&2
	echo "Download it from RunDiffusion/Juggernaut-X-v10 first." >&2
	exit 1
fi

if [[ "${UNDERPAINT_START_PROMPT_HELPER:-1}" == "1" && -z "$prompt_helper_url_was_set" ]]; then
	"$repo_root/tools/ai/start-underpaint-prompt-helper.sh"
fi

exec "$repo_root/build-qt5-client-baseline/bin/drawpile" "$@"
