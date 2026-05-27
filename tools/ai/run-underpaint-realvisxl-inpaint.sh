#!/usr/bin/env bash
set -euo pipefail

script_dir="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
repo_root="$(cd -- "$script_dir/../.." && pwd)"

export UNDERPAINT_INPAINT_MODEL_ID="${UNDERPAINT_INPAINT_MODEL_ID:-realvisxl-v4-inpaint-diffusers}"
export UNDERPAINT_INPAINT_MODEL="${UNDERPAINT_INPAINT_MODEL:-OzzyGT/RealVisXL_V4.0_inpainting}"
export UNDERPAINT_INPAINT_MODEL_FORMAT="${UNDERPAINT_INPAINT_MODEL_FORMAT:-diffusers_repo}"
export UNDERPAINT_AI_WORKER="$repo_root/tools/ai/run-diffusers-worker.sh"
prompt_helper_url_was_set="${UNDERPAINT_PROMPT_HELPER_URL:-}"
export UNDERPAINT_PROMPT_HELPER_URL="${UNDERPAINT_PROMPT_HELPER_URL:-http://127.0.0.1:${UNDERPAINT_PROMPT_HELPER_PORT:-18080}/v1}"

if [[ "${UNDERPAINT_START_PROMPT_HELPER:-1}" == "1" && -z "$prompt_helper_url_was_set" ]]; then
	"$repo_root/tools/ai/start-underpaint-prompt-helper.sh"
fi

exec "$repo_root/build-qt5-client-baseline/bin/drawpile" "$@"
