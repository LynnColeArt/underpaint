# Feature Request: Object Turntable

## Summary

Underpaint should support an object turntable wizard that generates a 360 degree
spin from a selected layer, object group, or cutout. The result should be
imported into Drawpile's existing animation surface as editable frames rather
than treated as a separate video product.

This is not a generic "make a video" feature. It is a constrained art-tool
operation:

1. Select an object, layer, group, or region.
2. Cleanly isolate it against a neutral or transparent working plate.
3. Ask a helper to describe the object and preserve important visual traits.
4. Generate a short 360 degree turntable.
5. Import the frames into the document's animation timeline.
6. Let the artist edit, paint over, delete, reorder, or export the frames.

The feature should feel like an assisted animation wizard inside a normal art
program, not like a prompt-only video generator bolted onto the side.

## Motivation

Drawpile already supports basic animation. Underpaint can use that existing
surface instead of building a separate video editor. This makes a generated
object spin immediately useful:

- artists can inspect individual frames
- bad frames can be repainted instead of discarded wholesale
- generated views can be used as animation reference
- sprite sheets and turnarounds become easier to create
- the result stays inside the layer, mask, and timeline metaphors users already
  understand

This matches the Underpaint thesis: AI output should become normal editable art
material.

## Product Goal

The first product goal is a usable 360 degree object spin for isolated subjects.

Expected user-facing behavior:

- The user selects an object or object group.
- The user opens `Power Tools > Object Turntable`.
- The user chooses a turn amount and frame count.
- Underpaint generates key views and/or animation frames.
- The output appears as normal animation frames in the current document.
- The original object remains preserved.
- The generated frames are editable, inspectable, and undoable.

The generated spin does not need to be physically true 3D reconstruction. A
single image cannot prove the hidden back side of an object. The wizard should
present the result as a plausible creative turntable, not a guaranteed recovery
of unseen geometry.

## Workflow

Suggested first-pass workflow:

1. User selects an isolated object layer, object group, or active selection.
2. Underpaint captures an object packet:
   - source RGBA cutout
   - source alpha mask
   - bounding box
   - optional source context crop
   - layer/group name
   - document color profile and canvas scale metadata
3. The prompt helper describes the object:
   - object type
   - material
   - color
   - style
   - front-facing details to preserve
   - warnings about asymmetry or fragile details
4. The user confirms turntable settings.
5. The selected model generates frames or key views.
6. Underpaint imports generated images into animation frames.
7. The user reviews the frames and edits them like normal artwork.

## Suggested UI

Menu entry:

```text
Power Tools > Object Turntable...
```

Initial controls:

- Rotation: 90, 180, 360.
- Frame count: 8, 16, 24, 32.
- Direction: clockwise or counterclockwise.
- Background: transparent, neutral gray, source context, custom color.
- Preserve style: low, medium, high.
- Consistency: low, medium, high.
- Seed.
- Generate keyframes only.
- Import as:
  - animation frames
  - layer group
  - sprite sheet

The dialog should reuse existing Underpaint progress and cancellation behavior.
Closing the progress window during generation should cancel the active job.

## Model Strategy

The first implementation should research model families before committing to a
UI-backed renderer.

Candidate model categories:

- image-to-video models that can animate a single object in place
- multiview or novel-view synthesis models that can infer camera angles
- lightweight 3D-aware diffusion models
- small video models that can run locally or through an optional provider
- future Sana-family video/world models if they become practical for this task

Underpaint should not assume the same backend will work for every machine. The
model manager should expose turntable-capable models separately from inpaint,
outpaint, refiner, detailer, and segmentation models.

Useful model capability flags:

- `turntable`
- `image-to-video`
- `multiview`
- `transparent-input`
- `transparent-output`
- `local-4070`
- `cloud-recommended`
- `experimental`

## Runtime Contract

The job should be represented as an AI job type rather than custom UI logic.

Proposed job kind:

```json
{
  "kind": "object-turntable",
  "sourceImage": "object.png",
  "sourceMask": "object-mask.png",
  "prompt": "small ceramic robot figurine, glossy blue paint",
  "rotationDegrees": 360,
  "frameCount": 16,
  "direction": "clockwise",
  "background": "transparent",
  "seed": 12345
}
```

Expected response artifacts:

```text
turntable-frame-000.png
turntable-frame-001.png
turntable-frame-002.png
...
turntable-metadata.json
```

`turntable-metadata.json` should preserve:

- seed
- model id
- model backend
- frame count
- rotation degrees
- per-frame angle estimates
- elapsed time
- peak memory when available
- prompt and helper prompt, if used

## Animation Import

Generated frames should land in the existing animation system.

Open questions for implementation:

- whether each generated image becomes a new animation frame on the same layer
- whether each frame becomes a layer inside a turntable group
- whether the wizard should create a new document when dimensions differ
- how to preserve timing and frame delay
- how to export a sprite sheet or GIF/video after review

The first slice can choose the least invasive import path and document it.

## Quality Risks

Expected model weaknesses:

- invented back sides may not match the source object
- frame-to-frame identity may drift
- thin parts may flicker or disappear
- asymmetrical objects may rotate inconsistently
- transparent backgrounds may produce edge halos
- text, logos, faces, and hands may degrade across angles

The UI should make these failures recoverable by keeping every frame editable.

## Dependencies

This feature benefits from:

- clean object decomposition
- background removal and mask refinement
- prompt helper support
- model manager capability metadata
- robust progress and cancellation
- animation-frame import/export polish

It does not need to wait for perfect underpainting. A selected layer or manually
cleaned cutout is enough for an early prototype.

## Acceptance Criteria

- A user can select a layer or isolated object and open an object turntable
  dialog.
- The dialog can request a 360 degree spin with a chosen frame count.
- The AI worker can generate a sequence of frame images or return a clear
  unsupported-model error.
- The generated frames are imported into the document as editable animation
  material.
- The original selected object remains preserved.
- The job records model, seed, prompt, frame count, elapsed time, and output
  frame paths.
- Generation can be cancelled from the progress UI.

## Non-Goals For The First Pass

- Full video editing.
- Physically accurate 3D reconstruction.
- Rigging, bones, or mesh editing.
- Perfect sprite consistency.
- Real-time generation.
- Replacing Drawpile's existing animation surface.
