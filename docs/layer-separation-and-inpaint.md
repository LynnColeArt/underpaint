# Layer Separation And Inpaint

This spec defines the first major Underpaint workflows: scene decomposition, manual inpaint, and intentional outpaint.

The core idea is simple: AI operations produce editable layers, masks, maps, and candidate groups. The user stays in the artwork rather than moving into a node graph.

## Product Goals

- Make scene separation feel like an artist-facing layer operation.
- Make inpaint approachable without hiding its editable layer and mask outputs.
- Expose advanced controls for users who need repeatability and precision.
- Make outpaint intentional, not automatic.
- Preserve the original image.
- Keep every result inspectable and reversible.

## Shared Concepts

### Source

An operation can use:

- the visible composite image
- selected layer
- selected layer group
- one or more chosen source layers
- source snapshot captured at operation start

### Target Region

The target region can come from:

- active selection
- accepted scene region
- object mask
- transparent canvas expansion
- manually painted mask
- agent-proposed mask

### Candidate Group

Generative operations return candidate layer groups:

```text
Operation Name
  Candidate 1 [visible]
  Candidate 2
  Candidate 3
  Mask used
  Source snapshot
```

The default visible candidate can be the first completed result. Users can switch candidates, discard alternates, or rerun.

### Provenance

Each candidate group stores:

- source layer ids
- source snapshot id
- mask id
- region bounds
- operation type
- prompt
- seed
- CFG
- denoise
- steps or quality preset
- context padding
- model id
- provider id
- guide layers
- requested by user/agent

## Scene Decomposition

Scene Decomposition is the Underpaint version of "magic layers." It separates meaningful image regions and prepares the canvas for restoration.

It does not care about font search, templates, or design assets. It cares about the layers of the image.

Example image:

A woman sits in a lake by a waterfall. She is clear and important. Trees, rocks, or branches encroach into the foreground. The user wants to preserve the subject and environment while controlling or removing unwanted foreground objects.

### Default Workflow

1. User clicks `Separate Image`.
2. Underpaint captures the visible image.
3. Segmentation proposes candidate masks.
4. Naming/grouping pass labels masks.
5. User sees proposed regions in a scene parts panel and layer stack.
6. User accepts, renames, merges, splits, hides, or refines regions.
7. For selected removed/separated regions, Underpaint can generate background repair candidates behind them.

### Expected Outputs

```text
Original Photo [locked]
Scene Repair
  Repair behind foreground rock - Candidate 1 [visible]
  Repair behind foreground rock - Candidate 2
  Repair behind foreground rock - Candidate 3
Separated Objects
  Woman
  Foreground rock
  Tree branches
  Waterfall
  Lake water
Analysis
  Segmentation map
```

### Current Implementation Slice

The first implementation is available as `AI > Photo Decomposition...`. It
captures the visible canvas, submits a `scene-separation` job, and imports the
returned RGBA region images as normal layers inside a `Photo Decomposition`
group.

The current worker output is deliberately provisional: it creates deterministic
luminance-based placeholder regions so the editor workflow can be tested before
SAM-like segmentation, labeling, and background repair are connected. The
important behavior is that every proposed region already enters the document as
an inspectable layer with a corresponding mask path in the AI response.

### Region Editing

Users must be able to:

- accept region
- reject region
- rename region
- merge regions
- split region
- refine edge
- feather mask
- expand/contract mask
- hide/show region
- lock accepted region
- rerun fill for region
- prompt region repair

Mask editing should be normal art-tool behavior, not a debugging mode.

### Repair Behind Object

When an object is separated or hidden, Underpaint can repair the revealed space.

Inputs:

- visible source image
- object mask
- hole mask
- context padding
- optional prompt
- AI preferences

Outputs:

- two or three repair candidates by default
- source snapshot
- mask used
- provenance

The user chooses the best candidate or reruns the region with edited mask/prompt/settings.

## Manual Inpaint

Manual Inpaint is the familiar selection-based entry point.

### Default Workflow

1. User selects an area.
2. Inpaint panel appears or becomes active.
3. User optionally enters a prompt.
4. User clicks Generate.
5. Underpaint creates a candidate layer group.
6. User picks a candidate, reruns, edits mask, or deletes group.

### Basic Controls

- prompt
- generate
- variation count

### Advanced Controls

- seed
- random/fixed seed mode
- CFG
- denoise
- steps or quality preset
- context padding
- guide layer
- model/provider
- output mode

Advanced controls should be available without turning the feature into a node editor.

### Output Modes

- new candidate group
- append candidates to current group
- replace candidates in current group
- create single visible layer

The default should be new candidate group.

## Intentional Outpaint

Canvas resize is not outpaint.

Expanding the canvas creates empty transparent space. Underpaint should never automatically fill that space with generated content. A user accustomed to art tools expects canvas expansion to be a layout operation, not an AI operation.

### Default Workflow

1. User expands the canvas.
2. Transparent area appears.
3. User selects the empty/edge area or clicks `Outpaint`.
4. Underpaint builds an edge-context mask.
5. User optionally prompts and adjusts settings.
6. Underpaint creates outpaint candidates.

### Inputs

- source visible image
- transparent expansion mask
- edge context
- prompt
- seed
- CFG
- denoise
- context padding
- variation count

### Outputs

```text
Outpaint - left edge
  Candidate 1 [visible]
  Candidate 2
  Candidate 3
  Edge mask
  Source snapshot
```

## AI Preferences

Global defaults should live in an AI Preferences page.

Recommended defaults:

- Scene repair variations: 3
- Inpaint variations: 3
- Outpaint variations: 3
- Detail Enhance variations: 1 or 2
- seed mode: random
- CFG: model-specific default
- denoise: model-specific default
- quality preset: balanced
- context padding: medium
- output: candidate group
- cloud use: ask every time unless trusted

Per-operation panels can override these defaults.

## 4070 Behavior

For RTX 4070-class hardware:

- run XL-class inpainting on crops, not full images
- generate candidates sequentially
- show candidates as they finish
- reuse cached segmentation/depth/pose maps
- unload heavy models after job batches
- avoid automatic background jobs while the user is painting

The UI should explain when a request is large and suggest reducing crop size or variation count.

## First Buildable Slice

A useful first slice is:

1. User selects a region.
2. Inpaint panel captures prompt/settings.
3. App exports image crop and mask.
4. Placeholder worker returns three tinted/generated placeholder images.
5. App creates a candidate layer group.
6. User switches visible candidate.

This proves the UX and artifact loop before real model integration.

Status: the first placeholder artifact loop now exists. The current desktop
action exports the region source and mask, receives three source-sized
placeholder variants from the local worker stub, and imports readable variants
as candidate layers.
