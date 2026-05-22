#!/usr/bin/env bash
set -euo pipefail

UNDERPAINT_HOME="${UNDERPAINT_HOME:-$HOME/.underpaint}"
MODEL_DIR="${UNDERPAINT_DETAIL_MODEL_DIR:-$UNDERPAINT_HOME/models/detail/adetailer}"
BASE_URL="https://huggingface.co/Bingsu/adetailer/resolve/main"

mkdir -p "$MODEL_DIR"

download_model() {
	local name="$1"
	local target="$MODEL_DIR/$name"
	if [[ -f "$target" ]]; then
		echo "Already present: $target"
		return
	fi
	echo "Downloading $name"
	curl -L --fail --progress-bar "$BASE_URL/$name" -o "$target"
}

download_model "face_yolov8n.pt"
download_model "hand_yolov8n.pt"
download_model "person_yolov8n-seg.pt"

echo
echo "Underpaint detail models are ready in:"
echo "$MODEL_DIR"
