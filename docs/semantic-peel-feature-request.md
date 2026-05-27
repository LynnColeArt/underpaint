# Feature Request: Semantic Peel

## Summary

Underpaint should support a staged decomposition workflow called Semantic Peel.
Instead of asking one automatic segmentation pass to find every useful layer at
once, Underpaint should repeatedly identify, segment, cut out, classify, and
repair one visible scene element at a time.

This turns decomposition into an active loop:

1. Look at the source image and the current remaining scene.
2. Choose the next visible object or part worth lifting.
3. Prompt a text- or box-directed segmentation backend for that target.
4. Refine the mask edge into a useful alpha cutout.
5. Save the cutout as an inspectable layer and mask.
6. Mark that area as claimed.
7. Repair the hole on the remaining plate.
8. Repeat until the useful scene elements have been peeled away.

The goal is not to gamify the UI with points or scores. The goal is to make the
AI pipeline behave like a careful restoration assistant that removes the scene
piece by piece instead of flooding the layer stack with noisy masks.

## Motivation

The current object-decomposition path has made useful progress, but crowded
images still expose the weakness of a single broad segmentation pass:

- unrelated objects can be mashed into the same slice
- clean people may be mixed with background fragments
- non-human objects can arrive as partial or imprecise regions
- layer names can drift when the helper is asked to classify too many ambiguous
  slices after the fact
- the repaired base can inherit bad holes from bad masks

Semantic Peel changes the question from "what are all possible masks in this
image?" to "what should be removed next?"

That is a better fit for Underpaint. Artists often think in operations:
"lift the red canopy," "remove the purple leaf," "separate the woman," "take
the cup out," "now repair what is behind it." The AI stack should follow that
same rhythm.

## Product Goal

Semantic Peel should become the high-quality decomposition lane for photo
restoration and object rearrangement.

Expected user-facing behavior:

- The user can run an automatic peel pass for the whole image.
- The user can also direct the next peel with text, a click, a box, or an
  existing selection.
- Each accepted peel creates normal Underpaint layers and masks.
- Related parts are grouped under useful object groups.
- The original image stays preserved and can be hidden while reviewing the
  peeled scene.
- The remaining plate is repaired as objects are removed.
- The user can stop, accept, reject, rename, merge, split, or manually edit at
  any point.

## Core Loop

Semantic Peel should keep several plates in memory for each run.

```text
Original Source
  Untouched visible composite captured at operation start.

Claimed Mask
  Union of regions already lifted out of the scene.

Remaining Plate
  Current working image after accepted objects are removed and repaired.

Candidate Target
  The next object or part proposed by the helper or user.

Layer Packet
  Imported cutout, mask, semantic metadata, grouping metadata, and provenance.
```

The helper should see the original source, the current remaining plate, and the
claimed-mask map. It should not reason from the damaged plate alone. The
original source preserves intent; the remaining plate shows what is still
available; the claimed mask prevents duplicate work.

Suggested loop:

1. Capture the original source image.
2. Initialize `claimedMask` as empty and `remainingPlate` as the source.
3. Ask the vision helper for the next target.
4. Convert that target into a segmentation prompt, box, point, or crop.
5. Run the selected promptable segmentation backend.
6. Refine the returned mask with cleanup and optional matting.
7. Reject masks that are duplicate, tiny, broad, low-confidence, or mostly
   outside the proposed target.
8. Import the cutout as a layer candidate.
9. Add accepted mask pixels to `claimedMask`.
10. Inpaint the accepted region on `remainingPlate`.
11. Ask the helper whether the scene still has useful peel targets.
12. Repeat until the mask budget is reached, the user stops, or the helper says
   no useful targets remain.

## Helper Responsibilities

The vision helper is the move chooser. It should make compact, auditable
decisions rather than generate a long private plan.

For each proposed target, it should return:

```json
{
  "targetId": "peel-014",
  "promptPhrase": "red cloth canopy",
  "semanticName": "Red Cloth Canopy",
  "groupLabel": "Market Awning",
  "depthRole": "foreground",
  "sceneRole": "structure",
  "repairRole": "remove-from-base",
  "reason": "large foreground canopy occluding the road and sky",
  "priority": 0.91,
  "expectedPartCount": 1
}
```

The helper may also return coarse boxes or points when it can estimate them.
Those hints should be treated as prompts, not truth.

The helper should prefer targets that are:

- visually distinct
- useful as movable layers
- likely to have a clean mask
- important to the repaired base
- not already claimed

The helper should avoid targets that are:

- broad background context unless the mode asks for background regions
- tiny texture fragments
- shadow-only fragments unless shadow extraction is enabled
- duplicate copies of already lifted objects

## Segmentation Responsibilities

The segmentation backend is the cutter. It should receive a target prompt and
return one or more candidate masks.

Useful backend classes:

- text-directed segmentation for prompts such as "red cloth canopy"
- detector-plus-SAM style segmentation for boxes or object proposals
- click or selection guided segmentation for user-directed peels
- automatic fallback masks only when no promptable path is available

Semantic Peel should not depend on one model family. It should use the same
pluggable backend registry described in
`docs/segmentation-backends-feature-request.md`.

## Mask And Matting Responsibilities

Mask cleanup is where Underpaint becomes an art tool instead of a model demo.
Every proposed peel should pass through an Underpaint-owned cleanup/refinement
stage before import.

Cleanup should:

- preserve source RGB and replace only alpha
- remove isolated specks
- fill only small pinholes
- preserve thin structures when the preset asks for fine detail
- reduce halos before base repair
- calculate edge confidence and overlap with claimed regions
- optionally run a matting/refinement model for hair, fur, fabric, petals, and
  other soft or detailed edges

The segmentation mask and the alpha matte are different artifacts. The mask can
drive repair and selection. The matte should drive the visible cutout layer.

## Repair Responsibilities

After an accepted peel, Underpaint should repair the remaining plate behind the
removed object. This is underpainting in miniature.

The repair pass should:

- build a repair mask from the accepted peel mask, expanded enough to cover
  halos
- use the current remaining plate as context
- keep the original source available for helper review and provenance
- run a conservative inpaint pass
- update `remainingPlate` for the next loop iteration
- keep repair candidates inspectable when the user asks for them

Automatic repair can be provisional. The user should be able to rerun repair
behind any peeled layer or group later.

## Layer Output

Semantic Peel should import a region set, not a pile of top-level layers.

Example:

```text
Semantic Peel
  Repaired Base
    Remaining Plate - Candidate 1
    Repair Masks
  Market Awning
    Red Cloth Canopy
    Green Awning Fabric
  Woman
    Hair
    Face
    Shirt
    Hands
  Road Objects
    Red Plastic Basin
    Blue Cloth Scrap
  Background Context
    Distant Mountain
    Sky / Horizon
  Masks
  Debug
```

Debug and source layers should be hidden by default. The layer tree should show
the usable artwork first.

## Job Contract Direction

The first implementation can fit into the existing `underpaint.ai-job.v1`
contract as a new job type:

```json
{
  "schema": "underpaint.ai-job.v1",
  "id": "8f6a2c75-4f5a-4c0d-9cb2-3e677e06f801",
  "operation": "semantic-peel",
  "inputs": [
    {
      "role": "source-image",
      "path": "/tmp/underpaint/source.png",
      "mimeType": "image/png"
    },
    {
      "role": "claimed-mask",
      "path": "/tmp/underpaint/claimed-mask.png",
      "mimeType": "image/png"
    },
    {
      "role": "remaining-plate",
      "path": "/tmp/underpaint/remaining-plate.png",
      "mimeType": "image/png"
    }
  ],
  "parameters": {
    "mode": "automatic",
    "targetPrompt": "",
    "maxPeels": 40,
    "repairBehindPeels": true,
    "segmentationPreset": "balanced",
    "mattingPreset": "fine-edge",
    "requireVisionHelper": true
  }
}
```

The response should keep the normalized region metadata from object
decomposition and add peel-specific provenance:

```json
{
  "id": "peel-014",
  "imagePath": "/tmp/underpaint/job/peel-014.png",
  "maskPath": "/tmp/underpaint/job/peel-014-mask.png",
  "mattePath": "/tmp/underpaint/job/peel-014-alpha.png",
  "label": "Red Cloth Canopy",
  "metadata": {
    "modelRole": "semantic-peel",
    "promptPhrase": "red cloth canopy",
    "groupLabel": "Market Awning",
    "depthRole": "foreground",
    "sceneRole": "structure",
    "repairRole": "remove-from-base",
    "bbox": {"x": 34, "y": 0, "width": 690, "height": 258},
    "peelIndex": 14,
    "claimedOverlap": 0.02,
    "edgeConfidence": 0.77,
    "segmentationBackend": "grounded-sam2",
    "mattingBackend": "birefnet"
  }
}
```

## UI Direction

Semantic Peel should feel like a guided art operation, not a node graph.

Suggested first UI:

- `Power Tools > Semantic Peel...`
- preset: `Fast`, `Balanced`, `Fine Edge`, `Prompted Target`
- max peels
- repair behind peeled objects
- edge refinement strength
- optional target prompt
- preview next peel before accepting
- stop button
- keep debug layers toggle

When running automatically, the progress window should show the current target:

```text
Peeling 14 / 40
Target: Red Cloth Canopy
Segmenting...
Refining edge...
Repairing remaining plate...
```

Later UI can add a "Next Target" panel where users accept, skip, rename, or
redirect the helper's next proposed peel.

## Failure Behavior

Semantic Peel depends on the vision helper for automatic operation. If the
helper is required and unavailable, the job should fail before changing the
document.

Allowed fallback modes:

- user-prompted peel with no helper, if the user supplies a target phrase or
  selection
- segmentation-only preview, clearly marked as unrepaired and unclassified
- object-decomposition fallback, clearly labeled as the older automatic mask
  path

Automatic repaired-base Semantic Peel should not silently continue with generic
names and generic foreground/background guesses.

## Acceptance Criteria

- A feature doc exists for Semantic Peel and is linked from the decomposition
  docs.
- The contract has a clear future job shape for `semantic-peel`.
- The workflow keeps source, claimed mask, remaining plate, and layer packets
  as distinct concepts.
- Automatic mode requires a working vision helper.
- Prompted mode can run from a user-provided target phrase or selection.
- Output imports as normal Underpaint layers, groups, masks, repair candidates,
  and metadata.
- Debug/source artifacts are hidden by default.
- The user can stop after any peel and keep the useful work.
- The design supports text-promptable segmentation backends without hard-coding
  one model.

## Non-Goals For The First Pass

- Perfect automatic decomposition.
- Real-time video object tracking.
- A node-based AI pipeline.
- Fully autonomous destructive edits.
- Bundling every candidate segmentation or matting model.
- Replacing manual selection and mask editing.

## Open Questions

- Should Semantic Peel replace Object Decomposition, or ship as a separate
  higher-quality mode first?
- Should every accepted peel repair the remaining plate immediately, or should
  repair batch after a group of related objects is removed?
- Should shadows be peeled with their parent object, as separate layers, or only
  during repaired-base generation?
- Should repeated small objects become separate layers, a grouped layer, or a
  single composite layer with individual masks?
- How should the UI expose helper confidence without making the workflow feel
  technical?
