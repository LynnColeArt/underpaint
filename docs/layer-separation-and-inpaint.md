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

### Current Utility Slice

The current implementation is now named `Power Tools > Color Separation...`. It captures
the visible canvas, submits the existing `scene-separation` job, and imports
deterministic luminance/color bands as normal layers inside a `Color Separation`
region-set group. The region set also includes hidden source and mask artifacts
so later actions can inspect or reuse the exact material that produced the
editable region layers. The dialog exposes maximum regions, minimum region
area, and grouping controls for this utility.

Color Separation is useful as a utility and as plumbing validation, but it is
not the titular decomposition feature. For a flower photo, useful decomposition
means separable parts like the rose bloom, individual petals, purple foreground
leaves, green leaves, stem segments, and background foliage clusters. That
requires SAM-like object/part masks, not color or luminance bands.

### Current Object Slice

`Power Tools > Object Decomposition...` is the first real attempt at the intended
decomposition feature. It submits an `object-decomposition` job to the AI
worker. The Python worker uses a SAM checkpoint, defaulting to
`facebook/sam-vit-base`, samples a point grid according to the selected depth,
deduplicates overlapping masks, ranks masks by usefulness rather than size,
keeps a base-remainder layer for pixels not covered by extracted objects,
rejects background-like masks, cleans small mask fragments and pinholes, then
imports the resulting object/part masks as movable transparent layers with
lightly feathered alpha edges.

The worker preserves source RGB while swapping alpha into the cutout PNGs. This
keeps feathered edges from picking up dark transparent-background fringes.
After import, extracted-object layers are queued for the same local helper
naming pass used by color separation; the base-remainder layer keeps its fixed
name.

This first slice still requires manual background repair: after import, select
an object layer or group and run `Power Tools > Underpaint Behind Active Layer...`.
Automatic repaired-base generation should be the next layer on top of this SAM
mask path.

## Underpainting

Underpainting is the title workflow that turns scene decomposition into a
restoration operation. It means lifting visible objects into editable regions
and reconstructing plausible image content underneath them.

The intended flow is:

1. Capture the visible source image.
2. Run automatic segmentation to produce candidate masks.
3. Cluster repeating masks into region groups.
4. Create a usable region set immediately with generic names.
5. Send region crops to the local helper queue for asynchronous labeling.
6. Update labels and prompt phrases as helper results arrive.
7. Let the user select a region or group and run `Underpaint Behind`.
8. Hide or dim the original source while previewing the repaired background.
9. Import repair candidates through the same candidate-layer workflow used by
   inpaint and outpaint.

Underpainting should not block on the language helper. Segmentation and mask
creation should be usable first. Labels, object classes, and prompt phrases can
arrive later as enrichment.

### Region Sets

A decomposition result should be a region set, not a flood of top-level layers.
This matters because useful decomposition can produce 10 masks or 200 masks
depending on the image and user goal.

Region set structure:

```text
Scene Decomposition
  Region Set - Balanced
    Foreground objects
      Region 001
      Region 002
    Repeating small regions
      Region 003
      Region 004
    Tonal/background regions
      Region 005
    Masks
    Source snapshot
```

For the first implementation, groups can be provisional. SAM will eventually
provide better masks, and the helper can rename groups after inspecting region
crops. The important early behavior is that repeated small regions can live
under one group instead of exploding the layer list.

### Decomposition Depth

Precision should be user-configurable as decomposition depth:

- `Clean`: fewer regions, object-level grouping, good for normal restoration.
- `Balanced`: useful default, preserves obvious objects and larger parts.
- `Detailed`: smaller objects and parts are retained.
- `Exhaustive`: high mask count for mask harvesting, damage cleanup, foliage,
  scratches, hair, lace, jewelry, and other dense subjects.

Advanced controls should include:

- max masks
- minimum region area
- overlap behavior
- keep nested masks
- mask feather
- mask expand/contract
- group repeated regions
- helper labeling priority

### Helper Labeling Queue

The local helper is the slowest part of the system, so it must not stop the
artist from working.

The queue should:

- start with generic `Region 001` labels
- rename imported decomposition layers after the region set is already usable
- label selected and visible regions first
- prioritize large regions over tiny masks
- allow user-hovered or user-selected regions to jump the queue
- cache labels by source image hash and mask fingerprint
- discard stale helper results if the region was deleted or merged

The helper should return small structured enrichments:

```json
{
  "label": "foreground branch",
  "group": "Foreground objects",
  "promptPhrase": "thin wet foreground branch crossing the lake scene",
  "backgroundGuess": "lake water, waterfall mist, and wet rock texture behind it",
  "confidence": 0.72
}
```

This is a smart function, not an autonomous agent. It suggests labels and prompt
phrases; the user remains in control.

Current first slice: after `Power Tools > Color Separation...` imports the region set,
Underpaint starts a quiet helper pass using the candidate region image. When the
vision helper returns a short name, the visible region layer is retitled. If the
helper is unavailable or a label fails, the generic region name remains and the
workflow is not interrupted. The same asynchronous label queue should be reused
for true object/part decomposition once SAM-like masks are connected.

### Underpaint Behind

When the user runs `Underpaint Behind` on a region or group:

- the original/source image remains preserved
- the selected mask becomes the hole mask
- the extracted object can stay above the repaired background for comparison
- the original source should be hidden or dimmed during candidate preview
- cancel restores the visibility state and removes temporary candidates
- accept leaves the chosen candidate visible and keeps provenance

For grouped regions, the masks are combined and repaired as one operation unless
the user promotes an individual region out of the group.

Current first slice: `Power Tools > Underpaint Behind Active Layer...` uses the active
layer alpha as the object mask, or combines visible child layer alpha when the
active item is a group. It exports a padded context crop, runs the existing
inpaint worker, and imports repaired background candidates below the active
layer or group. This makes decomposition layers immediately usable as lifted
objects while SAM masks and asynchronous helper labels are still being
connected. Decomposition also preserves hidden mask layers in the region set so
we have a durable bridge from "visible extracted object" to "editable hole
mask."

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
