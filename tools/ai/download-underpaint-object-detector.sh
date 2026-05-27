#!/usr/bin/env bash
set -euo pipefail

script_dir="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
repo_root="$(cd -- "$script_dir/../.." && pwd)"
underpaint_home="${UNDERPAINT_HOME:-$HOME/.underpaint}"
model_dir="${UNDERPAINT_OBJECT_DETECTOR_MODEL_DIR:-$underpaint_home/models/detection}"
target="${UNDERPAINT_OBJECT_DETECTOR_MODEL:-$model_dir/yolo11n-seg.pt}"

python_bin="${PYTHON:-}"
if [[ -z "$python_bin" ]]; then
	if [[ -x "$repo_root/.venv/bin/python" ]]; then
		python_bin="$repo_root/.venv/bin/python"
	else
		python_bin="python3"
	fi
fi

mkdir -p "$(dirname "$target")"

TARGET="$target" "$python_bin" - <<'PY'
import os
import shutil
import tempfile
from pathlib import Path

from ultralytics import YOLO

target = Path(os.environ["TARGET"]).expanduser()
target.parent.mkdir(parents=True, exist_ok=True)

if target.is_file():
    print(f"Already present: {target}")
else:
    with tempfile.TemporaryDirectory(prefix="underpaint-yolo-download-") as tmp:
        old_cwd = Path.cwd()
        os.chdir(tmp)
        try:
            model = YOLO("yolo11n-seg.pt")
            source = Path(getattr(model, "ckpt_path", "yolo11n-seg.pt")).expanduser()
            if not source.is_file():
                source = Path("yolo11n-seg.pt")
            if not source.is_file():
                raise SystemExit("Ultralytics did not download yolo11n-seg.pt")
            shutil.copy2(source, target)
        finally:
            os.chdir(old_cwd)
    print(f"Downloaded: {target}")

YOLO(str(target))
print(f"Underpaint object detector is ready: {target}")
PY
