# Feature Request: Pluggable Segmentation Backends

## Summary

Underpaint should replace the current single-SAM decomposition path with a
pluggable segmentation backend system. The goal is to support sharper masks,
faster previews, text-directed object selection, and future model upgrades
without changing the C++/Qt editor or turning the workflow into a node graph.

This is a follow-up feature request. It should build on the current
`object-decomposition` workflow, semantic layer grouping, repaired-base pass,
and `underpaint.ai-job.v1` contract.

For the higher-quality iterative decomposition path, see
`docs/semantic-peel-feature-request.md`.

## Motivation

Object decomposition is the title feature of Underpaint. It needs masks that
are precise enough to let users move a flower petal, remove foreground leaves,
lift a person from a scene, or rebuild the background behind overlapping
objects. The current SAM path is useful, but it is only the baseline.

No single segmentation model is likely to be enough for the product. Underpaint
needs different segmentation modes for different moments:

- high-quality object and part extraction
- fast previews while users tune decomposition depth
- text-directed segmentation such as "find the green leaf"
- background removal and matting
- low-power fallback on machines that cannot run the best model locally
- future video or frame-sequence object tracking

The product magic should live in the Underpaint layer pipeline: model choice,
mask cleanup, semantic grouping, user review, and repaired underpainting.

## Product Goal

Underpaint should provide a small set of artist-facing decomposition presets
that map to a backend registry.

Expected user-facing behavior:

- Users can choose a decomposition quality preset instead of a raw model name.
- Users can choose segmentation depth, from a few major objects to many
  fine-grained masks.
- Users can optionally direct segmentation with text, clicks, boxes, or
  existing selections.
- Imported results arrive as named, grouped, inspectable layers and masks.
- Source snapshots, debug masks, and context plates do not clutter the visible
  canvas.
- Mask edges are cleaned enough to be useful for moving and repairing objects.
- Missing or unsupported backends fail clearly before destructive work begins.

## Backend Candidates

### High-Quality Default

Primary candidates:

- SAM 2 or SAM 2.1 for the modern promptable segmentation baseline.
- HQ-SAM or HQ-SAM 2 for sharper boundaries and fine detail.

This lane should optimize for restoration usefulness: thin structures, petals,
hair, branches, decorative edges, and partial occlusions matter more than raw
demo speed.

### Text-Directed Segmentation

Primary candidates:

- Grounded SAM 2 with Grounding DINO, Florence-2, or a similar grounding model.
- Florence-style region captioning as a companion path for naming and grouping.

This lane should power workflows like:

- "separate the purple leaves"
- "find the person in red"
- "extract the car"
- "segment the foreground branches"

Text-directed segmentation should still return normal Underpaint masks and
layers. It should not become a separate product mode.

### Fast Preview

Primary candidates:

- EfficientSAM
- FastSAM
- MobileSAM or other small promptable segmenters after benchmarking

This lane should be used for previews, thumbnails, quick depth estimates, and
low-power machines. It should not be trusted as the final high-quality mask
path unless local tests prove the masks are good enough.

### Experimental Multimodal

Research candidates:

- SEEM or similar multimodal segmentation systems.

This lane is interesting for future text, click, scribble, and reference-image
segmentation, but it should not block the practical Underpaint workflow.

## Normalized Backend Contract

Each segmentation backend should return the same normalized region shape.

Example result:

```json
{
  "id": "region-12",
  "backend": "hq-sam2",
  "backendVersion": "beta",
  "promptType": "automatic",
  "semanticName": "green leaf",
  "depthRole": "foreground",
  "confidence": 0.86,
  "bbox": {"x": 286, "y": 412, "width": 118, "height": 96},
  "maskPath": "/tmp/underpaint-ai-job/region-12-mask.png",
  "cutoutPath": "/tmp/underpaint-ai-job/region-12.png",
  "areaPx": 8124,
  "overlapWithSelected": 0.12,
  "quality": {
    "edgeConfidence": 0.72,
    "thinStructureScore": 0.58,
    "mattingAvailable": false
  }
}
```

Required fields for the first pass:

- `id`
- `backend`
- `bbox`
- `maskPath`
- `cutoutPath`
- `areaPx`

Optional fields should be preserved as provenance and used by later grouping,
matting, and repair passes.

## Mask Cleanup Layer

Every backend should pass through an Underpaint-owned cleanup stage before
import.

Cleanup responsibilities:

- reject masks that are too small, too large, or mostly duplicate another mask
- reject broad context masks when the user asked for movable objects
- fill pinholes when appropriate
- remove isolated specks
- smooth or preserve edges depending on preset
- estimate a usable alpha edge for soft subjects when a matting backend exists
- compute overlap and containment relationships between regions
- keep raw masks available for debugging, but hidden from the default canvas

This is a product layer, not a model-specific detail. The segmentation model
gets Underpaint close; the cleanup layer decides what is actually useful.

## Preset Direction

Suggested initial presets:

- `Fast Preview`: quick approximate masks for UI iteration.
- `Balanced`: default object decomposition for general photo work.
- `Fine Detail`: slower, sharper masks for petals, hair, branches, lace, and
  restoration work.
- `Text Target`: segment objects matching a user phrase.
- `Background Removal`: subject/background matte with a dedicated matting path.

Presets should map to backend ids and settings in the model manager. Users can
see the backend if they want to, but they should not need to understand model
names to use the feature.

## UI Direction

The decomposition dialog should stay approachable:

- decomposition preset
- segmentation depth
- optional text target
- optional repair-under-objects toggle
- maximum masks
- edge cleanup strength
- preview before import

Advanced controls can be hidden behind an expander:

- backend
- confidence threshold
- overlap rejection threshold
- minimum and maximum area
- morphology cleanup amount
- matting/refinement backend
- keep debug masks

The layer panel should group related regions by semantic object when the helper
is available, while preserving the individual movable pieces.

## Runtime Requirements

- Keep segmentation out of process behind `underpaint.ai-job.v1`.
- Do not load all segmentation models at app startup.
- Let the model manager explain installed, missing, recommended, experimental,
  and unsupported backends.
- Log backend id, model path, runtime, memory use, mask count, rejected mask
  count, cleanup settings, and helper status during development.
- Preserve Drawpile collaboration/session behavior by importing results as
  ordinary layers, groups, masks, and metadata.
- Treat every model license independently before bundling or recommending it.

## Bakeoff Harness

Before replacing the production decomposition path, compare candidate mask
engines with `tools/ai/underpaint-mask-bakeoff.py`.

List available harness methods:

```bash
.venv/bin/python tools/ai/underpaint-mask-bakeoff.py --list-methods
```

Run the default local comparison:

```bash
.venv/bin/python tools/ai/underpaint-mask-bakeoff.py \
  /path/to/test-image.png \
  --output-dir /tmp/underpaint-mask-bakeoff-test
```

The default run compares:

- `underpaint-current`: the current detector-first worker path.
- `yolo-object`: direct YOLO11n segmentation proposals.
- `yolo-person`: direct person segmentation proposals.
- `rembg-u2net`: CPU foreground matte path when the rembg model is present.

Run the first fine-edge candidate:

```bash
tools/ai/download-underpaint-birefnet.sh
.venv/bin/python tools/ai/underpaint-mask-bakeoff.py \
  /path/to/test-image.png \
  --method birefnet \
  --output-dir /tmp/underpaint-birefnet-bakeoff
```

Optional heavier methods:

```bash
.venv/bin/python tools/ai/underpaint-mask-bakeoff.py \
  /path/to/test-image.png \
  --method underpaint-sam-grid \
  --method underpaint-sam-hq-grid \
  --allow-cpu
```

Each method writes a folder containing:

- `report.json`
- `overlay.png`
- `contact-sheet.png`
- per-mask PNGs and cutouts

The root output directory also contains `summary.json` and the scaled
`source.png` that every method received. This makes it easier to compare edge
quality, missed objects, duplicate masks, bad grouping assumptions, and runtime
without importing every experiment into the editor.

The harness also names future candidate methods such as `sam2`,
`grounded-sam2`, `sam3`, and `ben2`. Those are intentionally reported as
skipped until their runtimes are wired and their licenses are reviewed.

## Source Notes

- Meta describes SAM 2 as a unified image/video object segmentation model with
  click, box, or mask prompts.
- HQ-SAM is explicitly aimed at higher-quality zero-shot SAM segmentation and
  now has HQ-SAM 2 checkpoints.
- Grounded SAM 2 combines SAM 2 with grounding models such as Grounding DINO and
  Florence-2 for open-set and text-directed segmentation.
- SEEM supports multiple prompt types, including clicks, strokes, text, and
  referring images.
- EfficientSAM targets lighter-weight segment-anything behavior.
- FastSAM is useful to evaluate for speed, but it has important class and
  quality caveats.

References:

- https://ai.meta.com/research/sam2/
- https://github.com/SysCV/sam-hq
- https://github.com/IDEA-Research/Grounded-SAM-2
- https://github.com/UX-Decoder/Segment-Everything-Everywhere-All-At-Once
- https://yformer.github.io/efficient-sam/
- https://docs.ultralytics.com/models/fast-sam

## Acceptance Criteria

- Segmentation backends are declared through a registry or model-manager entry.
- The worker can select a backend by job settings instead of a hard-coded model.
- At least two backends can return the same normalized region result shape.
- The cleanup layer can reject duplicate, broad, tiny, and low-utility masks.
- Decomposition presets are exposed in the UI without forcing raw backend
  selection.
- The current SAM path still works as a fallback backend.
- Decomposition imports named groups, object layers, masks, hidden debug layers,
  and repaired base context without exposing helper clutter by default.
- Logs clearly report which backend ran, how many masks were produced, how many
  were rejected, and why.

## Non-Goals For The First Pass

- Perfect fully automatic decomposition.
- A node-based segmentation workflow.
- Bundling every candidate model.
- Video segmentation or tracking in the first implementation.
- Replacing manual mask editing.
- Removing the current SAM backend before a better backend is working.

## Open Questions

- Should HQ-SAM/HQ-SAM 2 become the first quality upgrade, or should SAM 2.1
  land first because it is the broader modern baseline?
- Should text-directed segmentation use Grounded SAM 2 directly, or should
  Florence-style region proposals feed the existing helper and SAM path first?
- Should matting be part of this ticket or a separate background-removal ticket?
- How much of the cleanup stage should be shared with background removal and
  detailer masks?
- Should the UI preview masks before importing layers, or should it import
  hidden candidates that the user promotes?
