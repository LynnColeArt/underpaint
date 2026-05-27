#!/usr/bin/env bash
set -euo pipefail

script_dir="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"

# One-command local AI launcher. Model-specific launchers still exist for
# comparison, but this is the default path for day-to-day Underpaint testing.
exec "$script_dir/run-underpaint-realvisxl-inpaint.sh" "$@"
