#!/usr/bin/env bash
set -euo pipefail

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
python_bin="${PYTHON:-}"
if [[ -z "$python_bin" ]]; then
	if [[ -x "$repo_root/.venv/bin/python" ]]; then
		python_bin="$repo_root/.venv/bin/python"
	else
		python_bin="python3"
	fi
fi
export PYTHONDONTWRITEBYTECODE=1

if ! "$python_bin" - <<'PY'
try:
    import PIL  # noqa: F401
except Exception:
    raise SystemExit(77)
PY
then
	exit 77
fi

exec "$python_bin" -m unittest discover \
	-s "$repo_root/tools/ai/tests" \
	-p 'test_*.py'
