#!/usr/bin/env bash
set -euo pipefail

script_dir="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
repo_root="$(cd -- "$script_dir/../.." && pwd)"
UNDERPAINT_HOME="${UNDERPAINT_HOME:-$HOME/.underpaint}"
LOG_DIR="$UNDERPAINT_HOME/logs"
PID_FILE="$UNDERPAINT_HOME/runtime/prompt-helper.pid"
LOG_FILE="$LOG_DIR/prompt-helper.log"

mkdir -p "$LOG_DIR" "$(dirname "$PID_FILE")"

if [[ -f "$PID_FILE" ]]; then
	old_pid="$(cat "$PID_FILE" 2>/dev/null || true)"
	if [[ -n "$old_pid" ]] && kill -0 "$old_pid" 2>/dev/null; then
		echo "Underpaint prompt helper is already running: $old_pid"
		exit 0
	fi
fi

cd "$repo_root"
setsid "$script_dir/run-underpaint-prompt-helper-server.sh" >"$LOG_FILE" 2>&1 < /dev/null &
pid="$!"
echo "$pid" > "$PID_FILE"
echo "Started Underpaint prompt helper: $pid"
echo "Log: $LOG_FILE"
