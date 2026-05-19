#!/usr/bin/env bash
set -euo pipefail

script_dir="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
repo_root="$(cd -- "$script_dir/../.." && pwd)"

export UNDERPAINT_AI_WORKER="$repo_root/tools/ai/run-diffusers-worker.sh"

exec "$repo_root/build-qt5-client-baseline/bin/drawpile" "$@"
