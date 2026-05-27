#!/usr/bin/env bash
set -euo pipefail

script_dir="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
repo_root="$(cd -- "$script_dir/../.." && pwd)"
UNDERPAINT_HOME="${UNDERPAINT_HOME:-$HOME/.underpaint}"
HOST="${UNDERPAINT_PROMPT_HELPER_HOST:-127.0.0.1}"
PORT="${UNDERPAINT_PROMPT_HELPER_PORT:-18080}"
URL="http://$HOST:$PORT/v1"
LOG_DIR="$UNDERPAINT_HOME/logs"
PID_FILE="$UNDERPAINT_HOME/runtime/prompt-helper.pid"
LOG_FILE="$LOG_DIR/prompt-helper.log"
START_TIMEOUT="${UNDERPAINT_PROMPT_HELPER_START_TIMEOUT:-120}"

mkdir -p "$LOG_DIR" "$(dirname "$PID_FILE")"

helper_ready() {
	curl -fsS --max-time 2 "$URL/models" >/dev/null 2>&1
}

wait_for_helper() {
	local pid="$1"
	local deadline=$((SECONDS + START_TIMEOUT))
	while (( SECONDS < deadline )); do
		if ! kill -0 "$pid" 2>/dev/null; then
			return 2
		fi
		if helper_ready; then
			return 0
		fi
		sleep 1
	done
	return 1
}

if [[ -f "$PID_FILE" ]]; then
	old_pid="$(cat "$PID_FILE" 2>/dev/null || true)"
	if [[ -n "$old_pid" ]] && kill -0 "$old_pid" 2>/dev/null; then
		if helper_ready; then
			echo "Underpaint prompt helper is already running: $old_pid"
			echo "URL: $URL"
			exit 0
		fi
		echo "Waiting for existing Underpaint prompt helper: $old_pid"
		if wait_for_helper "$old_pid"; then
			echo "Underpaint prompt helper is ready."
			echo "URL: $URL"
			exit 0
		fi
		echo "Underpaint prompt helper process exists but did not become ready: $old_pid" >&2
		echo "Log: $LOG_FILE" >&2
		exit 1
	fi
	rm -f "$PID_FILE"
fi

if helper_ready; then
	echo "Underpaint prompt helper is already reachable."
	echo "URL: $URL"
	exit 0
fi

cd "$repo_root"
setsid "$script_dir/run-underpaint-prompt-helper-server.sh" >"$LOG_FILE" 2>&1 < /dev/null &
pid="$!"
echo "$pid" > "$PID_FILE"
echo "Started Underpaint prompt helper: $pid"
echo "Log: $LOG_FILE"
echo "URL: $URL"

ready_status=0
wait_for_helper "$pid" || ready_status=$?
if [[ "$ready_status" == "0" ]]; then
	echo "Underpaint prompt helper is ready."
	exit 0
fi
case "$ready_status" in
2)
	echo "Underpaint prompt helper exited before it became ready." >&2
	echo "Log: $LOG_FILE" >&2
	exit 1
	;;
*)
	echo "Timed out waiting for Underpaint prompt helper to become ready." >&2
	echo "Log: $LOG_FILE" >&2
	exit 1
	;;
esac
