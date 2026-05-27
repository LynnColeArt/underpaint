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
- `object-decomposition`
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

## Background Removal

The desktop `Power Tools > Remove Background...` action exports the visible canvas as a
`source-image`, submits a `background-removal` job, and imports a grouped
foreground cutout. The group contains:

- `Background Removed`, an RGBA foreground cutout.
- `Foreground Matte`, a hidden grayscale alpha matte artifact.
- `Source Snapshot`, a hidden copy of the input image.

The Python worker prefers `rembg` with the CPU ONNX Runtime backend for this
quick local slice. The first run downloads `u2net.onnx` into `~/.u2net`. If
`rembg` or ONNX Runtime are unavailable, the worker falls back to a SAM
foreground-union path and reports that backend in diagnostics. That fallback is
useful for development, but the intended community-quality path is a dedicated
matting/background-removal model such as BiRefNet.

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
    "uiEntryPoint": "Power Tools/Inpaint"
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

## Color Separation

The desktop `Power Tools > Color Separation...` action is a utility path for
luminance/color-band layer separation. It exports the visible canvas as a
`source-image`, submits the existing `scene-separation` job, and imports every
returned candidate as a normal transparent layer inside a `Color Separation`
group.

The worker behavior is intentionally separate from the future object/part
decomposition feature. Both the compiled stub and Python worker split the image
into deterministic luminance regions
such as `Shadows`, `Darks`, `Midtones`, `Lights`, and `Highlights`. This validates
the editor contract, layer grouping, progress UI, and import behavior before a
real segmentation backend is chosen, but it should not be presented as the
flower/petal/leaf style decomposition workflow.

Future model-backed workers should keep the same response shape:

- `imagePath` should point to an RGBA layer image in document coordinates.
- `maskPath` should point to the corresponding grayscale region mask.
- `label` should be human-readable enough for the layer list.
- `metadata.modelRole` should use `color-separation` for this utility path.
- Progress events may use `candidate` events as each region becomes available.

## Object Decomposition

The desktop `Power Tools > Object Decomposition...` action is the first model-backed
path for the actual Underpaint decomposition workflow. It exports the visible
canvas as a `source-image`, submits an `object-decomposition` job, and imports
returned object/part masks as transparent movable layers.

The Python worker now treats decomposition as detector-first. By default it runs
installed YOLO-family priors before any SAM-family grid pass: the ADetailer
`person_yolov8n-seg.pt` model finds people, while
`~/.underpaint/models/detection/yolo11n-seg.pt` proposes common COCO-style
objects such as vehicles, animals, furniture, signs, and props. These detector
masks are ranked, cleaned, deduplicated, and imported as normal movable layers.

The SAM-family backend is still available as a second-pass grid discoverer
rather than the primary scene discoverer. `parameters.segmentationBackend`
selects that grid backend: `sam` defaults to `facebook/sam-vit-base`, while
`sam-hq` loads HQ-SAM from `~/.underpaint/models/segmentation/sam-hq-vit-base`
when downloaded. Backends can also be overridden with
`UNDERPAINT_SEGMENTATION_BACKEND`, `UNDERPAINT_SAM_HQ_MODEL`, or
`UNDERPAINT_SAM_MODEL`. The UI keeps the grid pass on by default because
detectors alone miss unusual scene objects like robots, notes, signs, and
whiteboard scraps.

The person prior is controlled with `parameters.personPriorEnabled` (default
`true`). It uses the installed ADetailer model under
`~/.underpaint/models/detail/adetailer` to find person boxes, merges overlapping
same-person detections, then asks SAM Base to turn those boxes into person
masks. Diagnostics report its status under `diagnostics.personPrior`.

The general object prior is controlled with `parameters.objectPriorEnabled`
(default `true`). It uses the installed YOLO11n segmentation detector under
`~/.underpaint/models/detection/yolo11n-seg.pt`. Diagnostics report its status
under `diagnostics.objectPrior`. When enabled with the person prior, person
detections are skipped in the general detector so the person-specific path can
keep bodies together.

The SAM-family grid pass is controlled with
`parameters.samGridFallbackEnabled` (default `true`). Diagnostics report its
status under `diagnostics.samGridFallback`. The pass runs after detector
proposals so unusual subjects, robots, notes, signs, whiteboard scraps, props,
and other background objects can still become editable layers. Candidates
record their source path with `metadata.maskPrior`, such as
`person-yolo-sam-box`, `yolo-mask-correction`, `object-yolo-seg`, or `sam-grid`.

The person prior has separate crowd-sensitive controls:
`parameters.personPriorConfidence` sets detector certainty,
`parameters.personPriorMaxRegions` caps promoted detections, and
`parameters.personPriorMinAreaPct` sets the minimum person-box size as a
percentage of the source image. This lets dense street scenes keep small distant
people without lowering the general SAM grid threshold for every object.

The object prior has matching controls:
`parameters.objectPriorConfidence`, `parameters.objectPriorMaxRegions`, and
`parameters.objectPriorMinAreaPct`. The defaults are intentionally recall-heavy
so small props survive the first pass and can be filtered by cleanup and layer
review instead of disappearing before the artist sees them.

`parameters.minRegionAreaPct` is a numeric percentage and may be fractional.
Busy scenes often need values below `1.0`; the UI default is intentionally low
so small people, props, and object parts are not discarded before ranking.
The object-decomposition default is tuned for detailed scene peel rather than
only the most obvious foreground subjects.

The operation returns the same candidate shape as color separation, plus
normalized segmentation metadata such as `metadata.segmentationBackend`,
`metadata.bbox`, `metadata.areaPx`, and worker diagnostics for rejected masks.
Worker-side selection ranks masks by usefulness rather than size, keeps a
base-remainder layer for pixels not covered by extracted objects, removes small
disconnected fragments, fills only tiny pinholes, rejects likely background
masks, and emits a softly feathered cutout alpha:

- `imagePath` points to the RGBA extracted object/part layer.
- `maskPath` points to the corresponding grayscale mask.
- `metadata.modelRole` uses `object-decomposition`.
- `metadata.maskRole` uses `extracted-object`.
- `metadata.maskPrior` identifies `person-yolo-sam-box`,
  `yolo-mask-correction`, `object-yolo-seg`, or `sam-grid`.
- `metadata.bounds`, `metadata.bbox`, `metadata.areaPixels`,
  `metadata.areaPx`, and `metadata.predictedIou` describe the mask.

Cutout PNGs preserve the source RGB and replace only the alpha channel. This
avoids dark premultiplied-looking fringes around feathered masks.

After a successful `object-decomposition` job, the editor applies default
semantic metadata to each candidate. When a local vision helper is configured,
the editor requires it to be reachable, sends the full source image plus each
isolated region to the helper, and stores returned metadata:

- `metadata.semanticName`
- `metadata.depthRole`, using `foreground`, `midground`, `background`,
  optional `sky-horizon`, or `ambiguous`
- `metadata.sceneRole`, such as `subject`, `prop`, `structure`, or `scenery`
- `metadata.repairRole`, using `keep-context` or `remove-from-base`
- `metadata.promptPhrase`
- `metadata.semanticConfidence`

For decomposition results, `metadata.semanticName` is the specific part or
layer name, while `metadata.groupLabel` is the parent object group used in the
layer tree. The helper should group related pieces together, for example car
panels under `Red Car`, body and clothing parts under `Woman`, and robot parts
under `Robot`. When the image contains sky, clouds, far hills, vanishing roads,
or a visible horizon, the helper may classify those distant pieces as
`depthRole: "sky-horizon"` and group them under `Sky / Horizon`; this role is
optional and should not be invented for images without that distance cue.

Automatic base repair is currently disabled while the layer packet is being
tested. The editor imports object layers, masks, groups, and semantic metadata
without launching a follow-up diffusion generation. In this mode, the
`base-remainder` layer is pinned into a dedicated `Base Remainder` group and
stays visible underneath the extracted object layers so the imported stack
reconstructs the original source. Viewed by itself, this layer is expected to
look like a punched plate with transparent holes where extracted objects were
removed. The source snapshot and mask debug layers stay hidden. The
repaired-base scaffold still exists behind the feature flag: it builds a
semantic repair source plate from
`base-remainder` plus `keep-context` candidates, turns
`remove-from-base` candidates into the repair mask, then runs an `inpaint`
request with `prefillStyle: object-context-plate`.

Users can still select an imported object layer or group and run
`Power Tools > Underpaint Behind Active Layer...` for manual reruns or targeted
background repair candidates.

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

Vision-backed helper operations, including decomposition region classification,
need a running OpenAI-compatible vision endpoint. When a helper endpoint is
configured, object decomposition fails early if that endpoint is absent, and it
fails before import if the helper classifies zero extracted regions. This is
intentional: layer naming and grouping should not silently fall back to
misleading semantics.

To download the current 4B prompt-helper candidate:

```bash
tools/ai/download-underpaint-qwen35-4b.sh
```

The default download uses `unsloth/Qwen3.5-4B-GGUF`. The MTP variant was tried
first, but the current local llama.cpp build failed to load it because of a
missing MTP tensor, so the non-MTP GGUF is the safer prompt-helper target.

For day-to-day testing, use the unified launcher:

```bash
tools/ai/run-underpaint.sh
```

It starts the prompt helper if needed, waits for the OpenAI-compatible
`/v1/models` endpoint to become reachable, exports
`UNDERPAINT_PROMPT_HELPER_URL`, configures the Diffusers worker, and launches the
app. The model-specific launchers do the same helper wiring:

```bash
tools/ai/run-underpaint-realvisxl-inpaint.sh
tools/ai/run-underpaint-juggernaut.sh
tools/ai/run-underpaint-juggernaut-x.sh
```

The helper launcher defaults to `~/.qwench/runtime/bin/llama-server`, a Qwen GGUF
under `~/.underpaint/models/prompt`, CPU execution via `--gpu-layers 0`, and the
Qwen multimodal projector when present. It falls back to existing Qwench models
when the Underpaint model directory has not been populated yet. Override these
with `UNDERPAINT_PROMPT_HELPER_MODEL_PATH`, `UNDERPAINT_PROMPT_HELPER_MMPROJ`,
`UNDERPAINT_PROMPT_HELPER_PORT`, or `UNDERPAINT_PROMPT_HELPER_GPU_LAYERS`. Set
`UNDERPAINT_START_PROMPT_HELPER=0` only when an external helper endpoint is
already running and `UNDERPAINT_PROMPT_HELPER_URL` is set manually.

Decomposition sends the helper a cropped region preview plus a downscaled
whole-image reference. The defaults are intentionally small enough to avoid
llama-server `request exceeds the available context size` failures:
`UNDERPAINT_PROMPT_HELPER_REGION_EDGE=512`,
`UNDERPAINT_PROMPT_HELPER_REGION_MIN_EDGE=256`, and
`UNDERPAINT_PROMPT_HELPER_SOURCE_EDGE=640`. The launcher starts llama-server
with `UNDERPAINT_PROMPT_HELPER_CTX=8192` unless overridden.

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
tools/ai/run-underpaint.sh
```

The default model is:

```text
diffusers/stable-diffusion-xl-1.0-inpainting-0.1
```

The worker uses `UNDERPAINT_INPAINT_MODEL` when a different Diffusers
inpainting model should be tested. It requires CUDA by default. A CPU-only
smoke path exists behind `UNDERPAINT_AI_ALLOW_CPU=1`, but that is expected to be
very slow and is not representative of the RTX 4070 target.

The current worker keeps the editor contract backend-neutral while using
Diffusers as the first runnable implementation. Model parameters may carry a
backend value such as `diffusers` or `gguf`; the editor should pass that through
as scheduling metadata rather than assuming all model identifiers are Diffusers
repositories.

The first registry file lives at:

```text
tools/ai/model-registry.json
```

Set `UNDERPAINT_MODEL_REGISTRY` to point the editor and worker at a different
registry during experiments. Registry entries describe capability, backend,
format, model locator, license notes, install state, and runtime policy. Job
parameters can pass `modelId`; workers resolve that id into backend/model facts
before loading or delegating to a backend adapter.

The refiner settings support two backend names:

- `diffusers`: runs through `StableDiffusionXLImg2ImgPipeline`.
- `gguf`: experimental adapter lane for a future SDXL GGUF image runtime.

`parameters.refiner.placement` controls where the optional refiner runs relative
to the targeted face/body detailer:

- `before-detail`: base generation, global refiner, then detected detail crops.
- `after-detail`: base generation, detected detail crops, then one final global
  refiner pass.

GGUF diffusion checkpoints are not passed to Diffusers pipelines directly. When
the GGUF refiner backend is selected, the worker expects an external adapter
configured by `UNDERPAINT_GGUF_REFINER_WORKER`. That adapter receives a small
JSON request, response path, and working directory, then returns an output image
path. Until a GGUF image runtime is wired in, selecting this backend fails
cleanly instead of trying to load a `.gguf` file as a Diffusers pipeline.

In `safe4070Mode`, the Diffusers refiner uses model CPU offload by default to
avoid keeping both XL-class base/refiner memory pressure entirely on the GPU.
This makes the refiner slower but more likely to complete on 12 GB cards. Set
`UNDERPAINT_AI_CPU_OFFLOAD=1` to force model CPU offload more broadly during
local testing.

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
