# Underpaint Architecture

Underpaint is a C++/Qt desktop application with an out-of-process AI runtime. The app owns the artwork. The runtime owns model execution.

This architecture keeps the editor responsive, preserves Drawpile's collaboration/session machinery, and leaves room for Python, ONNX Runtime, TensorRT, cloud workers, and future native providers.

The initial editor-to-worker contract is documented in
`docs/ai-job-contract.md` and represented in C++ by
`src/desktop/ai/aijob.h`.

## System Boundary

```text
Underpaint C++/Qt app
  canvas, layers, masks, selections
  project files and provenance
  undo/history/session events
  collaboration and permissions
  AI preferences and model manager UI

AI runtime provider
  model loading and unloading
  GPU memory management
  inference execution
  model-specific preprocessing
  crash isolation
```

The app should never depend on Python inside the UI process. Early local providers can be Python worker processes. Optimized providers can later use ONNX Runtime, TensorRT, CUDA, or cloud APIs behind the same job boundary.

## Core Operation Model

All AI features should flow through a shared operation shape:

```text
source artifact(s)
+ target region/mask
+ operation settings
+ model/provider selection
= candidate artifact group
```

Examples:

- Generative Fill: source visible image + active selection + prompt/settings = candidate patch layers.
- Outpaint: expanded canvas + transparent/edge mask + prompt/settings = candidate outpaint layers.
- Scene Repair: separated-object mask + source image + context = candidate background repair layers.
- Depth Map: source visible image = grayscale guide layer.
- Background Removal: source layer = cutout layer + alpha matte.

## AI Job Request

The first runtime protocol can be simple JSON plus file paths. It does not need to be final, but it should look like the final domain model.

Minimum request fields:

```yaml
job_id: uuid
operation: generative_fill
source:
  project_id: string
  image_path: path
  mask_path: path
  region_bounds: [x, y, width, height]
  source_layer_ids: [int]
settings:
  prompt: string
  negative_prompt: string
  seed: int | null
  cfg: float
  denoise: float
  steps: int
  variation_count: int
  context_padding: int
  output_size: [width, height]
provider:
  backend: local_python
  model_id: string
provenance:
  requested_by: user_or_agent_id
  created_at: timestamp
```

Minimum response fields:

```yaml
job_id: uuid
status: complete
artifacts:
  - type: rgba_image
    path: path
    seed: int
    label: Candidate 1
  - type: rgba_image
    path: path
    seed: int
    label: Candidate 2
metadata:
  model_id: string
  backend: local_python
  elapsed_ms: int
  warnings: [string]
```

For early implementation, the worker can return placeholder images. The important first step is the app-to-runtime-to-layer loop.

## Candidate Layer Groups

Generated outputs should land in a structured layer group rather than replacing pixels.

Example:

```text
Generative Fill - continue lake behind rock
  Candidate 1 [visible]
  Candidate 2
  Candidate 3
  Mask used
  Source snapshot
```

Rules:

- The original source remains untouched.
- One candidate is visible by default.
- Other candidates stay hidden until selected.
- The mask used for generation is retained.
- Provenance is stored with the group or generated layers.
- Reruns should either append candidates or create a new candidate group.

## Provenance

Every AI artifact should know how it was made.

Track:

- operation type
- source layer ids
- region bounds
- mask id or mask artifact
- model id
- provider id
- seed
- CFG
- denoise
- steps or quality preset
- prompt and negative prompt
- guide layers used
- local/cloud execution
- user or agent that requested the job

The UI does not need to show all of this by default, but it should be stored for rerun, audit, and collaboration.

## Model Manager

The model manager has two responsibilities:

1. User-facing capability management.
2. Runtime-facing model scheduling metadata.

It should track:

- model id
- display name
- capabilities
- provider/backend
- source URL
- license
- commercial-use status
- installed state
- disk size
- expected VRAM/RAM
- supported precision
- preferred input size
- maximum tested input size
- output artifact type
- dependencies
- safety/privacy notes

Users should mostly see capabilities:

- Separate Image
- Generative Fill
- Outpaint
- Background Remove
- Detail Enhance
- Create Depth Map
- Create Pose Map

The runtime should see model scheduling facts:

- can this model run locally?
- can it run in FP16?
- should it stay warm?
- must another model unload first?
- can it process a crop?
- does it support tiling?

## 4070 Runtime Policy

The RTX 4070-class target should shape defaults.

Rules:

- Prefer region crops over whole-canvas diffusion.
- Use context padding rather than full-image generation.
- Generate variants sequentially by default.
- Show the first completed candidate while later candidates continue.
- Keep small preprocessors warm only when cheap.
- Keep one heavy XL-class diffusion family resident at a time.
- Unload heavy models after inactivity.
- Cache derived guide layers such as depth, pose, and segmentation maps.
- Use VAE tiling/slicing for large jobs.
- Expose a VRAM budget in AI Preferences.

The scheduler should be conservative. It is better to run a job sequentially and finish than to oversubscribe VRAM and crash.

## Providers

Provider types:

- `local_python`: first implementation path for model experimentation.
- `local_onnx`: optimized preprocessor path for segmentation, depth, pose, matting, and upscaling where available.
- `local_tensorrt`: optimized path for stable models after conversion is proven.
- `cloud`: paid/hosted rendering or storage.
- `agent`: future session participant that can request or propose jobs.

All providers should produce the same artifact shape from the app's perspective.

## Collaboration And Agents

Drawpile's session model should remain intact. AI workers and agents can eventually appear as scoped participants.

Agent rules:

- agents create proposed artifacts
- agents do not silently overwrite originals
- agent permissions are visible
- agent actions are logged
- users can accept, hide, delete, or rerun agent output

This keeps MCP or other agent control systems aligned with the same domain operations used by the UI.

## First Implementation Slice

The first architecture slice should prove:

1. Export active selection and mask to runtime input files.
2. Submit a job request to a local worker.
3. Receive one or more image artifacts.
4. Create a candidate layer group.
5. Store basic provenance.
6. Keep the app responsive if the worker fails.

The worker can be a placeholder at first. The milestone is the boundary and artifact loop, not model quality.
