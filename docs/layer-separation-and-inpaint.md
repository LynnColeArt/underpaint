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
worker. The current path is detector-first: ADetailer
`person_yolov8n-seg.pt` finds whole-person candidates, YOLO11n segmentation
finds common objects, and the worker ranks, cleans, deduplicates, and imports
those masks as movable transparent layers. This is a better fit for crowded
photos than the old SAM grid-first path, because people, vehicles, animals,
props, signs, and furniture start as object-shaped regions instead of random
point-grid fragments.

The SAM-family segmentation backend is still available for fallback discovery.
The dialog lets the user choose `sam-hq` or `sam`, but the SAM grid pass is off
by default because HQ-SAM has been unreliable in recent decomp tests. Enabling
the fallback can recover unlabeled pieces, but it may also add noisy partial
masks that need cleanup. Person-prior certainty, object-prior certainty,
maximum detections, and minimum region sizes are separate controls so dense
crowds and small props can keep high recall without making every SAM fallback
mask enormous.

The worker keeps a base-remainder layer for pixels not covered by extracted
objects, rejects background-like masks, cleans small mask fragments and
pinholes, and imports the resulting object/part masks with a very light alpha
edge.

The worker preserves source RGB while swapping alpha into the cutout PNGs. This
keeps feathered edges from picking up dark transparent-background fringes.
Each imported region records normalized metadata such as
`segmentationBackend`, `bbox`, `areaPx`, `predictedIou`, `maskPrior`,
`className`, and `detectorModel`; the worker reports how many masks were
rejected by each cleanup policy.
The minimum region area can be fractional; the UI default is below `1%` because
busy street scenes, crowds, small props, and partial figures are exactly where
decomposition becomes useful.
When a vision helper is configured, Underpaint asks it to classify each extracted
region using both the full source image and the isolated slice. The helper
returns semantic metadata such as layer name, foreground/background depth role,
scene role, repair role, group, prompt phrase, and confidence. When automatic
base repair is enabled, this helper is required: Underpaint should fail loudly
instead of silently building a bad repair plate from generic fallback roles.

This slice can optionally run an automatic repaired-base pass after the SAM
masks return. Underpaint builds a semantic repair source plate from candidates
whose `repairRole` is `keep-context`, masks candidates whose `repairRole` is
`remove-from-base`, expands that remove mask slightly to cover object halos, then
runs one conservative inpaint pass with the same edge-slice prefill strategy used
by outpaint. The result imports as `Repaired Base` inside the base-remainder
group. Manual `Power Tools > Underpaint Behind Active Layer...` remains useful
for rerunning or targeting a specific layer or group.

## Underpainting

Underpainting is the title workflow that turns scene decomposition into a
restoration operation. It means lifting visible objects into editable regions
and reconstructing plausible image content underneath them.

Underpainting is outpainting turned inward. Instead of extending an image past
its borders, Underpaint reconstructs what should exist underneath the visible
composition. A mask alone is not a judgment call: SAM can supply scissors, but
the vision helper must decide how each region participates in the scene before
diffusion sees the repair problem.

The dirty background plate is an intentional intermediate artifact. Background
and keep-context pieces are flattened onto a scratch plate, while removed
foreground pieces become holes. Broken texture, half-erased shadows, and edge
artifacts are useful signals: they show the model where the hidden scene needs
repair. The goal is not a perfect synthetic image. The goal is a clean plate
that can sit behind movable layers without calling attention to itself.

Later passes should repeat this logic. After the first repaired-base candidate,
the vision helper can compare the original image, layer packet, and repaired
background to find leftover foreground contamination. Those leftovers become a
new mask, and the repair pass runs again. The same method can eventually repair
object overlaps: if one foreground object hides part of another, Underpaint can
reconstruct the hidden lower-layer piece before putting the scene back together.

SAM gives geometry. The vision helper gives judgment. Diffusion gives paint.
Underpaint's job is to make them cooperate without flattening the workflow into
uninspectable soup.

The intended flow is:

1. Capture the visible source image.
2. Run automatic segmentation to produce candidate masks.
3. Cluster repeating masks into region groups.
4. Create a usable region set immediately with generic names.
5. Send the full source image plus each region crop to the local vision helper.
6. Store semantic metadata on regions as helper results arrive.
7. Optionally run an automatic repaired-base pass from the semantic repair plate.
8. Let the user select a region or group and run `Underpaint Behind` for manual
   reruns or targeted repairs.
9. Hide or dim the original source while previewing the repaired background.
10. Import repair candidates through the same candidate-layer workflow used by
   inpaint and outpaint.

Underpainting should not hide helper failures. Mask-only decomposition can still
be useful as a future non-repair mode, but repaired-base generation depends on
semantic judgment and should fail clearly when the vision helper is unavailable
or cannot classify any extracted regions.

The higher-quality version of this workflow is described in
`docs/semantic-peel-feature-request.md`: an iterative loop where the vision
helper chooses the next scene element, a promptable segmentation backend cuts it
out, mask/matting cleanup turns it into an artist-usable layer, and the
remaining plate is repaired before the next peel.

### Region Sets

A decomposition result should be a region set, not a flood of top-level layers.
This matters because useful decomposition can produce 10 masks or 200 masks
depending on the image and user goal.

Region set structure:

```text
Scene Decomposition
  Region Set - Balanced
    Base Remainder
      Repaired Base
      Base Remainder
    Sky / Horizon
      clouds
      distant hills
      horizon road
    Red Car
      windshield
      hood
      dashboard edge
    Woman
      face
      hair
      shirt
      hand
    Robot
      robot head
      coffee cup
      compass hand
    Masks
    Source snapshot
```

For the first implementation, groups can be provisional. SAM will eventually
provide better masks, and the helper can rename groups after inspecting region
crops. The important early behavior is that repeated small regions can live
under one group instead of exploding the layer list. The helper should treat
`name` as the specific layer or part label and `group` as the parent object
bucket in the layer tree.

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
- person detector certainty
- person detector max regions
- minimum person area
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
  "name": "foreground branch",
  "depthRole": "foreground",
  "sceneRole": "prop",
  "repairRole": "remove-from-base",
  "group": "Foreground Objects",
  "promptPhrase": "thin wet foreground branch crossing the lake scene",
  "confidence": 0.72
}
```

`depthRole` describes visual depth: foreground is anything in front of the
background, midground covers intervening scene elements, and background is the
near or general scenery behind those foreground and midground elements.
`sky-horizon` is an optional deeper background role for sky, clouds, far hills,
vanishing roads, visible horizon lines, and other distant scene structure. It
should only appear when that material is actually present.
`repairRole` describes how Underpaint should construct the hidden repair source:
`keep-context` regions are composited into the repair plate, while
`remove-from-base` regions become holes for diffusion to fill.

This is a smart function, not an autonomous agent. It suggests labels, groups,
prompt phrases, and repair roles; the user remains in control.

Current first slice: after `Power Tools > Object Decomposition...` gets SAM
masks, Underpaint runs the helper before base repair. When classification
succeeds, visible layers import with semantic names/groups and base repair uses
the semantic repair source plate. A second text-only helper pass refines parent
object groups so related parts can land under groups like `Woman`, `Robot`, or
`Car` instead of broad fallback groups. If the helper is not running, not
configured, or classifies zero extracted regions, the operation fails before
importing a misleading decomposition.

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
- refiner/detailer stage settings
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
