# Underpaint Agent Notes

This repo is the Underpaint workspace. Preserve collaboration/session behavior
while moving the public product surface toward a local-first AI-assisted photo
restoration workspace.

## Current Direction

- Keep the main app C++/Qt.
- Keep AI work out of process behind the `underpaint.ai-job.v1` JSON contract.
- Treat AI outputs as normal inspectable layers, masks, maps, regions, and
  candidates.
- Keep inpaint and outpaint explicit user actions.
- Do not reintroduce SD 1.5 as the default inpaint/outpaint target.
  The current baseline is XL-class inpainting, tested on crops with conservative
  RTX 4070 behavior.

## Current AI Worker State

- The dependency-free compiled stub is still the default worker:
  `build-qt5-client-baseline/bin/underpaint-ai-worker-stub`.
- The real Diffusers worker lives at:
  `tools/ai/underpaint-diffusers-worker.py`.
- Diffusers is the first runnable image backend. GGUF diffusion support is an
  explicit experimental backend lane, not a Diffusers drop-in. The refiner
  settings can pass `backend: "gguf"`, but the worker expects
  `UNDERPAINT_GGUF_REFINER_WORKER` to point at a future external GGUF image
  adapter before that path can render.
- Fooocus SDXL inpaint patching is also an experimental adapter lane. The
  assets download to `~/.underpaint/models/inpaint/fooocus/`, but the patch is
  not a Diffusers drop-in; it needs either a Comfy-style model patcher worker or
  a native equivalent before it can render through the app.
- RealVisXL V4.0 Inpaint is the first photoreal true-Diffusers inpaint
  candidate. Launch it with `tools/ai/run-underpaint-realvisxl-inpaint.sh`.
  A cached 512x512 smoke with 24 steps completed in about 13 seconds and used
  about 5.2 GB peak allocated VRAM with model CPU offload.
- The face/body detail pass should crop detected regions, upscale the crop to
  `detailRenderEdge`, inpaint that crop, then resize/blend it back. It should
  not run only at the full-candidate resolution, because small faces do not get
  enough pixels that way.
- The first model registry is `tools/ai/model-registry.json`. The editor and
  worker can be pointed at an alternate registry with `UNDERPAINT_MODEL_REGISTRY`.
- `tools/ai/run-diffusers-worker.sh` sets
  `PYTORCH_CUDA_ALLOC_CONF=expandable_segments:True` by default. In
  `safe4070Mode`, the Diffusers refiner uses model CPU offload automatically;
  set `UNDERPAINT_AI_CPU_OFFLOAD=1` to force lower-VRAM offload more broadly.
- The local Python venv is:
  `.venv`.
- The venv has been set up with:
  - `torch 2.12.0+cu130`
  - `diffusers 0.37.1`
  - `transformers 4.57.6`
  - `accelerate 1.13.0`
  - `numpy 1.26.4`
  - `pillow 12.2.0`
  - `safetensors 0.7.0`
  - `ultralytics 8.x`
  - `rembg 2.0.69`
  - `onnxruntime 1.26.0`
- Background removal is wired through `Power Tools > Remove Background...`. The worker
  prefers `rembg`/U2Net via CPU ONNX Runtime and downloads `~/.u2net/u2net.onnx`
  on first use. If `rembg` or ONNX Runtime are unavailable, it falls back to a
  SAM foreground-union dev path and reports that backend in diagnostics.
- Repeatable install command:

```bash
uv venv --python /usr/bin/python3 .venv
uv pip install --python .venv/bin/python -r tools/ai/requirements-diffusers.txt
```

## CUDA State

CUDA is currently visible after reboot. Verified state:

- loaded kernel module: `580.159.04`
- installed user-space/DKMS driver: `580.159.04`
- GPU: `NVIDIA GeForce RTX 4070`
- VRAM: about 12 GB total
- venv PyTorch: `torch.cuda.is_available() == True`

Current Diffusers smoke result after the CUDA driver refresh: a 512x512
inpainting request with one low-step candidate succeeded on the RTX 4070,
returned a 512x512 PNG, and reported about 7.3 GB peak VRAM.

The desktop Inpaint action now runs the worker on a background Qt
thread and shows a modal indeterminate progress dialog while the model loads and
generation runs. Real percentage progress needs a future worker progress
channel; the current JSON response arrives only when the job finishes.

Inpaint now requires an active selection. The previous fallback treated
"no selection" as "repaint the whole canvas," which produced slow,
hard-to-understand full-canvas jobs. A real desktop test from the app after the
CUDA reboot ran the Diffusers backend on CUDA with
`diffusers/stable-diffusion-xl-1.0-inpainting-0.1`, generated three 2000x2000
candidates in about 92 seconds, and reported about 8 GB peak VRAM. The result
was nearly blank because the request was a full-canvas repaint with an empty
prompt, not because the stub worker was used.

The Inpaint completion dialog now includes the resolved worker path,
backend, model, device, elapsed time, and peak VRAM when the worker reports
those diagnostics.

When Inpaint is run without an active selection, the app switches to
the rectangular selection tool and asks the user to select a repaint area before
running Inpaint again. This avoids leaving the user in the Annotation
tool, whose settings panel looks like a text box and makes the workflow feel
broken.

The Diffusers worker pads source and mask crops to dimensions divisible by 8
before calling SDXL, then crops generated candidates back to the original
selection size. This fixes normal arbitrary selection sizes such as 557x560.
Verified odd-size smoke: 557x560 input padded to 560x560, CUDA generation
succeeded, final candidate exported as 557x560.

Inpaint now has a prompt/settings dialog before launching the worker.
It exposes prompt, negative prompt, candidate count, seed, CFG, denoise, and
step count. After generation, the app opens a candidate chooser with thumbnails;
clicking a candidate previews it by toggling the imported candidate layers'
local visibility, and "Use Candidate" leaves the selected layer visible.

Selection masks must be exported from the selection alpha channel, not from RGB
luminance. Rectangular selection masks can be black RGB with fully
opaque alpha; treating luminance as the inpaint mask makes Diffusers repaint
nothing and returns candidates that look like the original crop. The editor now
writes an explicit grayscale alpha mask, and the worker defensively treats
opaque black RGBA masks as alpha masks. Worker diagnostics include mask min/max.

Previous caveat, if this regresses: CUDA was blocked by this mismatch:

- loaded kernel module: `580.126.09`
- installed user-space/DKMS driver: `580.159.04`

When this happens, `nvidia-smi` reports `Driver/library version mismatch`, and
PyTorch reports `torch.cuda.is_available() == False`. A reboot should load the
matching module.

## Launch Notes

For normal local AI testing, use the one-command launcher:

```bash
tools/ai/run-underpaint.sh
```

It starts the prompt helper if needed, waits until `/v1/models` is reachable,
sets `UNDERPAINT_PROMPT_HELPER_URL`, points the app at the Diffusers worker, and
launches Underpaint. Set `UNDERPAINT_START_PROMPT_HELPER=0` only when you
intentionally want to provide your own helper endpoint.

Model-specific launchers use the same helper startup behavior:

```bash
tools/ai/run-underpaint-diffusers.sh
tools/ai/run-underpaint-realvisxl-inpaint.sh
tools/ai/run-underpaint-juggernaut.sh
tools/ai/run-underpaint-juggernaut-x.sh
```

## Verification Commands

```bash
.venv/bin/python -m py_compile tools/ai/underpaint-diffusers-worker.py
cmake --build build-qt5-client-baseline --target underpaint-ai-worker-stub
cmake --build build-qt5-client-baseline
git diff --check
```

Expected current Diffusers smoke-test result after reboot: model load and GPU
generation should proceed, subject to first-run model download time.
