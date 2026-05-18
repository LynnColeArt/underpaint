# Underpaint Thesis

Underpaint is a GPLv3, local-first AI photo restoration and image reconstruction workspace. It starts from Drawpile because Drawpile already has the hard editor substrate: canvas state, layers, masks, selections, transform paths, project import/export, session history, networking, chat, hosting, and reconnect behavior.

The project goal is not to make another prompt box with image previews. Underpaint should make AI results feel like real art-tool material: layers, masks, maps, regions, annotations, candidates, and provenance that the user can inspect, edit, compare, hide, blend, rerun, or delete.

## Product North Star

Good art tools are intuitive, familiar, and satisfying in a predictable way. Great art tools also give the user miles of depth when they need controlled, specific work.

Underpaint should feel approachable like Photoshop's AI features, but deeper. The user should not need a node workflow to perform advanced AI-assisted restoration. Advanced controls should modify the current operation, not force the user to assemble the operation.

The target is:

```text
Photoshop-level accessibility
+ ComfyUI-level power
+ layers and masks instead of node graphs
```

## Core Principles

### Original Is Sacred

The imported photo stays as an untouched base layer. Every AI operation creates derived material above it.

Examples:

- extracted object layers
- background repair layers
- generated fill candidates
- outpaint candidates
- alpha mattes
- segmentation masks
- depth maps
- normal maps
- pose maps
- restoration annotations

### AI Outputs Are Inspectable

AI should not silently mutate the working image. It should propose material artifacts.

Background removal should produce a cutout and matte. Segmentation should produce named masks or regions. Depth estimation should produce a visible grayscale guide layer. Inpainting should produce candidate patch layers with the mask and source context preserved.

### Layers Are the Workflow

Underpaint should use familiar layer and masking metaphors to express complex AI pipelines.

Internally, an operation may look like this:

```text
source layers -> target mask -> guide map -> model -> candidates -> output group
```

The user should see:

```text
Source: visible image
Mask: active selection
Guidance: depth layer
Output: candidate group
```

### Separation Is Iterative

Layer separation will not be perfect. That is expected.

Users must be able to:

- merge regions
- split regions
- rename regions
- hide or show regions
- refine edges
- feather, expand, or contract masks
- rerun a fill for one region
- add a prompt for one region
- lock accepted regions

Human-corrected masks should be treated as better inputs than raw model output.

### Regions Become Objects

Underpaint should let parts of a photo become named, editable restoration objects.

Examples:

- person
- face
- hair
- hand
- water
- waterfall
- foreground rock
- tree branch
- damaged corner
- scratch cluster
- handwriting
- photo border

Each region can have attached masks, generated repairs, alternate candidates, annotations, and provenance.

## Scene Decomposition

The scene separation feature is the first major expression of the thesis.

Example: a photo of a woman sitting in a lake by a waterfall.

The user clicks a button to separate the image. Underpaint runs a segmentation pass and proposes classifiable visual regions: woman, water, waterfall, rocks, trees, foreground objects, and other meaningful scene parts.

For each separated region, Underpaint can generate background repair candidates behind it. If a foreground branch or rock is undesirable, the app should extract the object, preserve the mask, and produce two or three plausible fills for the newly revealed area. The user chooses the best candidate, edits the mask, reruns with a prompt, or discards the result.

Scene decomposition should create a reconstruction workspace, not destroy the original image.

## Generative Region Operations

Generative fill, outpaint, and scene-repair fills should share one operation model:

```text
source image/layers
+ target mask/region
+ optional prompt
+ generation parameters
+ context policy
+ variant count
= candidate layer group
```

Entry points:

- Scene separation repair: model-created mask.
- Manual generative fill: user-created selection or mask.
- Outpaint: user-created canvas expansion and edge/transparent-area mask.

Canvas resize is not outpaint. Expanding the canvas creates empty space. Outpaint is a deliberate operation the user starts.

## Candidate Outputs

Generative operations should default to multiple candidates, usually two or three. The selected candidate becomes the visible layer, but other candidates stay available unless the user discards them.

Example layer group:

```text
Generative Fill - continue lake water behind rock
  Candidate 1 [visible]
  Candidate 2
  Candidate 3
  Mask used
  Source snapshot
```

The user can rerun the operation with a prompt, fixed seed, different denoise value, or edited mask.

## Advanced Controls Without Nodes

Simple mode:

- select area
- type optional prompt
- generate
- choose a variation

Advanced mode:

- seed
- CFG
- denoise
- steps or quality preset
- context padding
- variation count
- guide layer selection
- model/provider selection
- output mode

Recipe/provenance view:

- shows what happened
- allows rerun with changed settings
- stores operation settings
- can become a preset

This gives the user depth without moving them into a separate node canvas.

## Collaboration And Agents

Drawpile's collaboration features should remain intact. They are not launch clutter if they are treated as strategic infrastructure.

Underpaint can use this substrate for:

- shared restoration sessions
- client review workflows
- cloud rendering workers
- local AI workers
- agent participants
- provenance and replay
- studio/private servers

Agents should be participants, not opaque plugins. An agent may create suggested layers, masks, maps, and annotations, but should not silently overwrite originals.

The long-term MCP idea is an agent-facing control port over domain operations:

- list layers
- get selection
- export region
- create mask
- create layer
- apply image to layer
- add annotation
- submit AI job
- propose edit

## Runtime Boundary

Underpaint should remain a C++/Qt app. AI should run out of process behind a stable job API.

Python is acceptable for early local model experimentation. It should not be embedded into the main UI process.

The runtime boundary should allow:

- local Python provider
- ONNX Runtime provider
- TensorRT provider
- cloud provider
- future C++ native providers

The app owns canvas state, project files, user interaction, layer history, and artifact management.

The runtime owns model loading, inference, memory pressure, and crash isolation.

## Model Manager

The model manager is a core subsystem, not just a download page.

It should know:

- installed models
- capabilities
- license status
- commercial-use status
- backend
- precision support
- expected VRAM and RAM footprint
- supported image sizes
- tiling limits
- which models can stay resident together
- which workflows depend on which model packs

Users should think in capabilities, not model names:

- background removal
- segmentation
- generative fill
- outpaint
- detail upscale
- face restoration
- depth map
- normal map
- pose map

## Hardware Target

The default local target is RTX 4070-class hardware.

This implies:

- model swapping is normal
- one heavy model family resident at a time
- region operations should crop and pad context
- variants should generate sequentially if needed
- depth/pose/segmentation maps should be cached
- VAE tiling/offload should be available for diffusion workflows
- ONNX/TensorRT should be used where it gives real wins

## Licensing Shape

Underpaint Community should be a real GPLv3 app, open to the public and useful without paid services.

Premium value can live in hosted services:

- cloud rendering
- cloud storage
- managed model packs
- team collaboration
- support

Model weights have their own licenses. The app should track model licenses explicitly and prefer license-clean defaults for the community edition.

