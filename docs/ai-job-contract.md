# AI Job Contract

This contract describes the boundary between the C++ Underpaint editor and
future local or cloud AI workers. It does not choose a backend. Python, ONNX
Runtime, TensorRT, or a hosted worker should all be able to implement this
shape.

## Goals

- Keep the editor responsive while AI work runs out of process.
- Treat AI operations as intentional user actions.
- Pass files and metadata across the boundary, not in-memory editor objects.
- Return inspectable candidate assets that can become normal layers, masks, or
  guide maps.
- Preserve enough provenance to rerun, compare, reject, or explain results.
- Let workers fail without taking the editor down.

## Schema

The current schema identifier is:

```text
underpaint.ai-job.v1
```

The C++ request and response types live in `src/desktop/ai/aijob.h`.

## Operation Keys

- `scene-separation`
- `inpaint`
- `outpaint`
- `background-removal`
- `upscale`
- `depth-guide`
- `normal-guide`
- `pose-guide`
- `style-transfer`
- `face-restore`

Workers may still accept the older `generative-fill` key as a compatibility
alias for `inpaint`, but new editor requests should emit `inpaint`.

## Request Shape

Requests contain exported editor assets and operation settings. Asset paths
should point at files in a temporary job directory owned by the editor.

```json
{
  "schema": "underpaint.ai-job.v1",
  "id": "d8b0f47e-8e0e-4f4e-b8ab-318c3e2a7a1a",
  "operation": "inpaint",
  "inputs": [
    {
      "role": "source-image",
      "path": "/tmp/underpaint-job/source.png",
      "mimeType": "image/png",
      "metadata": {
        "layerId": 42,
        "colorSpace": "srgb"
      }
    },
    {
      "role": "mask",
      "path": "/tmp/underpaint-job/mask.png",
      "mimeType": "image/png",
      "metadata": {
        "whiteMeans": "editable-region"
      }
    }
  ],
  "region": {
    "x": 128,
    "y": 64,
    "width": 512,
    "height": 384,
    "contextPadding": 128
  },
  "parameters": {
    "prompt": "repair missing background texture",
    "negativePrompt": "",
    "seed": -1,
    "cfg": 5.0,
    "denoise": 0.75,
    "candidateCount": 3
  },
  "preferences": {
    "maxRenderEdge": 1024,
    "variantMode": "sequential",
    "unloadPolicy": "idle",
    "vaeTiling": true,
    "cacheGuides": true,
    "safe4070Mode": true
  },
  "source": {
    "documentName": "scan-restoration.dpcs",
    "activeLayerId": 42,
    "selectionSource": "current-selection"
  },
  "provenance": {
    "createdBy": "underpaint",
    "uiEntryPoint": "AI/Inpaint"
  }
}
```

## Response Shape

Workers should return `succeeded`, `failed`, or `canceled`. Candidate files
should also live in the job directory until the editor imports them.

```json
{
  "schema": "underpaint.ai-job.v1",
  "id": "d8b0f47e-8e0e-4f4e-b8ab-318c3e2a7a1a",
  "status": "succeeded",
  "message": "Generated 3 candidates.",
  "candidates": [
    {
      "id": "candidate-1",
      "label": "Candidate 1",
      "imagePath": "/tmp/underpaint-job/candidate-1.png",
      "maskPath": "/tmp/underpaint-job/mask.png",
      "metadata": {
        "seed": 132117,
        "modelRole": "inpaint"
      }
    }
  ],
  "diagnostics": {
    "elapsedMsec": 42100,
    "peakVramMb": 9216
  },
  "provenance": {
    "model": "placeholder-xl-inpaint",
    "backend": "worker-stub"
  }
}
```

## Editor Responsibilities

- Export source imagery, region crops, masks, and guide maps before submitting
  the job.
- Include region coordinates in document space.
- Keep original layers untouched.
- Import each returned candidate as a removable layer, mask, guide map, or
  candidate group.
- Store request and response provenance with imported results.
- Treat worker failure as a recoverable operation error.

## Worker Responsibilities

- Read only the paths listed in the request.
- Write outputs to the assigned job directory.
- Never modify the original document or editor-owned files in place.
- Return one response JSON object.
- Include diagnostics when practical.
- Exit nonzero or return `failed` when the job cannot be completed.

## First Implementation Path

Status: initial editor-to-worker artifact loop exists. Inpaint exports
the visible source crop and mask, asks the local worker stub for three
placeholder candidates, and imports readable candidates as new layers inside a
candidate group.

1. Export the current selection as a source crop and mask.
2. Write a request JSON file.
3. Launch the local `underpaint-ai-worker-stub` with the request path,
   response path, and job directory.
4. Have the worker return a placeholder image candidate.
5. Import the candidate as a normal layer.
6. Attach request/response provenance to the imported layer or group.

The first native runner lives in `src/desktop/ai/aijobrunner.h`. It creates a
temporary job directory, writes `request.json`, launches the worker with
`QProcess`, reads `response.json`, and reports process or parse failures
without crashing the editor.

## Photo Decomposition

The desktop `AI > Photo Decomposition...` action is the first editor path for
layer separation. It exports the visible canvas as a `source-image`, submits a
`scene-separation` job, and imports every returned candidate as a normal
transparent layer inside a `Photo Decomposition` group.

The initial worker behavior is intentionally a placeholder. Both the compiled
stub and Python worker split the image into deterministic luminance regions
such as `Shadows`, `Darks`, `Midtones`, `Lights`, and `Highlights`. This validates
the editor contract, layer grouping, progress UI, and import behavior before a
real segmentation backend is chosen.

Future model-backed workers should keep the same response shape:

- `imagePath` should point to an RGBA layer image in document coordinates.
- `maskPath` should point to the corresponding grayscale region mask.
- `label` should be human-readable enough for the layer list.
- `metadata.modelRole` should use `photo-decomposition`.
- Progress events may use `candidate` events as each region becomes available.

## Prompt Helper

The Inpaint prompt field has an icon-only helper action that rewrites the
current prompt in place. This is intentionally a smart function, not an agent:
it receives the current prompt, negative prompt, selection size, and generation
settings, then returns one improved prompt string.

The desired output is a loaded diffusion prompt, roughly 150 characters long.
It should enrich the user's subject with texture, lighting, material, depth,
and edge-blending language rather than shortening the prompt into a caption.

Underpaint-owned local AI state should live under:

```text
~/.underpaint/
  models/
  cache/
  logs/
  runtime/
```

Qwench can remain a useful source of runtime binaries while Underpaint is young,
but model files and app-specific state should migrate into `~/.underpaint` so
the fork can stand on its own.

The helper script is `tools/ai/underpaint-prompt-helper.py`. It talks to a
llama.cpp/OpenAI-compatible endpoint when configured, and otherwise falls back
to a deterministic local rewrite so the UI remains usable without a running
model.

To download the current 4B prompt-helper candidate:

```bash
tools/ai/download-underpaint-qwen35-4b.sh
```

The default download uses `unsloth/Qwen3.5-4B-GGUF`. The MTP variant was tried
first, but the current local llama.cpp build failed to load it because of a
missing MTP tensor, so the non-MTP GGUF is the safer prompt-helper target.

To run a local llama.cpp prompt helper server using the Qwench runtime:

```bash
tools/ai/run-underpaint-prompt-helper-server.sh
```

Then launch Underpaint with the helper endpoint:

```bash
UNDERPAINT_PROMPT_HELPER_URL=http://127.0.0.1:18080/v1 \
  tools/ai/run-underpaint-juggernaut.sh
```

The launcher defaults to `~/.qwench/runtime/bin/llama-server`, a Qwen GGUF under
`~/.underpaint/models/prompt`, CPU execution via `--gpu-layers 0`, and the Qwen
multimodal projector when present. It falls back to existing Qwench models when
the Underpaint model directory has not been populated yet. Override these with
`UNDERPAINT_PROMPT_HELPER_MODEL_PATH`, `UNDERPAINT_PROMPT_HELPER_MMPROJ`,
`UNDERPAINT_PROMPT_HELPER_PORT`, or `UNDERPAINT_PROMPT_HELPER_GPU_LAYERS`.

## Real Diffusers Worker

The compiled worker stub remains the default because it is dependency-free. To
test a real XL-class inpainting model, point the editor at the Python worker:

```bash
uv venv --python /usr/bin/python3 .venv
uv pip install --python .venv/bin/python -r tools/ai/requirements-diffusers.txt

UNDERPAINT_AI_WORKER="$PWD/tools/ai/run-diffusers-worker.sh" \
  ./build-qt5-client-baseline/bin/drawpile
```

Or use the helper launcher:

```bash
tools/ai/run-underpaint-diffusers.sh
```

The default model is:

```text
diffusers/stable-diffusion-xl-1.0-inpainting-0.1
```

The worker uses `UNDERPAINT_INPAINT_MODEL` when a different Diffusers
inpainting model should be tested. It requires CUDA by default. A CPU-only
smoke path exists behind `UNDERPAINT_AI_ALLOW_CPU=1`, but that is expected to be
very slow and is not representative of the RTX 4070 target.

Current smoke result after the CUDA driver refresh: a 512x512 inpainting request
with one low-step candidate succeeded on the RTX 4070, returned a 512x512 PNG,
and reported about 7.3 GB peak VRAM.

The desktop Inpaint action now runs the worker on a background Qt
thread and shows a modal determinate progress dialog while the model loads and
generation runs. Workers may stream newline-delimited progress JSON to stdout
while the job is running. The first supported event types are:

```json
{"schema":"underpaint.ai-job.v1","type":"preview","candidate":1,"step":8,"steps":30,"seed":1234,"imagePath":"/tmp/job/preview-c1-s8.png"}
{"schema":"underpaint.ai-job.v1","type":"candidate","candidate":1,"seed":1234,"imagePath":"/tmp/job/candidate-1.png"}
```

The Diffusers worker uses step-end callbacks to save tiny preview images during
sampling, then emits final candidate events as each candidate completes. The
desktop runner parses those progress events before `response.json` is available,
advances the determinate progress bar, and shows the latest preview image in the
progress dialog. The event shape is intended to move into a proper canvas HUD
candidate strip later.

The desktop Inpaint action now requires an active selection. This avoids
accidental full-canvas inpainting when the user intended to repair a local
region. The completion dialog reports the resolved worker path, backend, model,
device, elapsed time, and peak VRAM when present in the response diagnostics.

A real app-launched SDXL inpainting run on the RTX 4070 succeeded with three
2000x2000 candidates, CUDA device diagnostics, and about 8 GB peak VRAM. The
output was nearly blank because the request was an empty-prompt full-canvas
repaint, not because the stub worker ran.

The editor exports a padded source crop around the selected area instead of
exporting only the exact selection bounds. This gives SDXL surrounding context
and lets rectangular selections return as transparent overlays instead of
visible rectangular patches. The Diffusers worker accepts arbitrary editor crop
sizes, but does not pass small odd-sized crops directly to SDXL. By default it
scales the source and mask to a 1024 px render edge, pads to dimensions divisible
by 8, runs SDXL, then crops and resizes each candidate back to the exported crop
size before returning it.

Returned inpaint candidates should be RGBA images. The worker uses the editable
mask as the alpha channel, applies the requested `edgeFeatherPx`, and leaves
unselected context pixels transparent so importing the candidate preserves the
original image outside the repaired area.

The editor exports inpaint masks from the selection alpha channel as explicit
grayscale images. This matters because Drawpile rectangular selection masks can
have black RGB data with fully opaque alpha; using luminance would produce an
all-black Diffusers mask and repaint nothing. The worker also treats opaque
black RGBA masks as alpha masks defensively and reports mask min/max in
diagnostics.

Inpaint is no longer a blind menu action. Before launching the worker,
the editor shows a settings dialog for prompt, negative prompt, candidate
count, seed, CFG, denoise, step count, and edge feather. After import, it shows
a thumbnail candidate chooser; selecting a thumbnail toggles local layer
visibility so the user can preview candidates and leave the chosen one visible.

If `nvidia-smi` reports a driver/library mismatch, the installed user-space
NVIDIA libraries and the loaded kernel module do not match. The model worker
can be installed and syntax-checked, but GPU generation should wait until CUDA
is visible to PyTorch.
