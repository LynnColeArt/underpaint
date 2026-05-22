#!/usr/bin/env bash
set -euo pipefail

UNDERPAINT_HOME="${UNDERPAINT_HOME:-$HOME/.underpaint}"
REPO="${UNDERPAINT_FOOOCUS_INPAINT_REPO:-lllyasviel/fooocus_inpaint}"
MODEL_DIR="${UNDERPAINT_FOOOCUS_INPAINT_DIR:-$UNDERPAINT_HOME/models/inpaint/fooocus}"

mkdir -p "$MODEL_DIR" "$UNDERPAINT_HOME/cache" "$UNDERPAINT_HOME/logs"

if ! command -v huggingface-cli >/dev/null 2>&1; then
	echo "huggingface-cli is required. Install huggingface_hub or set the model path manually." >&2
	exit 1
fi

download_file() {
	local filename="$1"
	local target="$MODEL_DIR/$filename"
	if [[ -f "$target" ]]; then
		echo "Already present: $target"
		return
	fi
	echo "Downloading $filename into $MODEL_DIR"
	huggingface-cli download "$REPO" "$filename" \
		--local-dir "$MODEL_DIR" \
		--local-dir-use-symlinks False
}

download_file "fooocus_inpaint_head.pth"
download_file "inpaint_v26.fooocus.patch"
download_file "fooocus_lama.safetensors"

echo
echo "Fooocus inpaint assets are ready:"
echo "  $MODEL_DIR"
echo
echo "Probe the patch with:"
echo "  tools/ai/underpaint-fooocus-patch-probe.py \\"
echo "    --checkpoint ~/.underpaint/models/checkpoints/juggernaut-x-v10/Juggernaut-X-RunDiffusion-NSFW.safetensors"
