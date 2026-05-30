# Feature Request: Pluggable Detailer Detectors

## Summary

Underpaint should support additional ADetailer-style detector models through a
small registry-driven detailer system. The goal is to make face, body, hand, and
future region-specific detail passes feel as easy to add as they are in
Automatic1111 or ComfyUI, without exposing a node graph to the user.

This is a follow-up feature request. It should build on the existing face/body
detail pass and the crop-detail-reinsert runtime described in
`docs/model-research.md`.

## Motivation

Detailing is not one model. Artists may want different detectors for:

- faces
- full bodies
- hands
- eyes
- clothing
- jewelry
- scratches or damage
- custom restoration targets

The diffusion detail pass can stay mostly the same. What changes is the detector
model, class mapping, prompt phrase, confidence threshold, crop padding, and
maximum number of regions. Underpaint should make those pieces configurable and
discoverable.

## Product Goal

Underpaint should provide a practical detailer panel where users can enable
specialized detectors without learning the backend pipeline.

Expected user-facing behavior:

- Users can enable or disable each installed detailer.
- Users can choose detection confidence and max regions.
- Users can choose detail strength and detail render size.
- Inpaint and outpaint can run enabled detailers after the base candidate pass.
- Candidate previews and logs show which detailers ran and how many regions
  were processed.
- New detector models can be added through the model manager or registry.

## Detector Contract

A detector entry should describe what it can detect and how Underpaint should
use it.

Example registry shape:

```json
{
  "id": "face-yolov8n",
  "name": "Face YOLOv8n",
  "role": "detail-detector",
  "target": "face",
  "backend": "ultralytics",
  "path": "~/.underpaint/models/detail/adetailer/face_yolov8n.pt",
  "classes": ["face"],
  "defaultConfidence": 0.45,
  "defaultMaxRegions": 8,
  "defaultPaddingPx": 32,
  "defaultPromptPhrase": "clear natural face detail"
}
```

The first supported backend can be Ultralytics `.pt` YOLO detection or
segmentation models. Later backends can be added if they return the same
normalized result shape.

Normalized detector result:

```json
{
  "target": "face",
  "className": "face",
  "confidence": 0.82,
  "box": {"x": 412, "y": 160, "width": 92, "height": 108},
  "maskPath": "/tmp/underpaint-ai-job/face-1-mask.png"
}
```

`maskPath` is optional for bbox-only detectors. Segmentation models should
provide it when available.

## Runtime Flow

For each enabled detailer:

1. Run the detector on the current candidate image.
2. Filter detections by confidence and class mapping.
3. Sort detections by priority.
4. Limit to the configured max region count.
5. Expand each box by padding.
6. Crop the image region.
7. Expand to a square working crop when possible.
8. Upscale the crop with the selected detail-enhancing upscale backend.
9. Render the diffusion detail pass at a minimum 1024 px working width.
10. Resize the result back to the original crop.
11. Feather and blend the result back into the candidate.

Detailers should be independent. A face detector, hand detector, and body
detector should be separate registry entries that all use the same normalized
pipeline.

## UI Direction

Keep the main inpaint/outpaint dialog simple. The detailed configuration should
live in an AI menu panel or model-manager detailer section.

Suggested controls:

- enable face detailing
- enable body detailing
- enable hand detailing
- detection confidence
- max regions
- crop padding
- detail render edge
- detail pre-upscale backend
- detail strength
- run after inpaint
- run after outpaint

Advanced model-specific settings can remain hidden until the user expands a
detailer entry.

## Implementation Notes

- This feature does not require changing the main C++/Qt app into a Python app.
- Keep detection and diffusion out of process behind `underpaint.ai-job.v1`.
- Store detector settings in AI preferences, not in the prompt text.
- Add detector metadata to candidate provenance.
- Log detector count, skipped detections, confidence thresholds, crop sizes, and
  detail pass timing during development.
- Treat model licenses independently. Do not assume all YOLO or ADetailer
  weights are redistributable.

## Acceptance Criteria

- A detailer detector can be declared in the model registry.
- The worker can load at least one registered Ultralytics YOLO detector.
- The worker returns normalized boxes for the enabled detector.
- The detail pass crops each detection, upscales the working crop, renders at
  `detailRenderEdge` with a 1024 px floor, and reinserts the result at the
  original position.
- The UI can enable/disable at least face, body, and hand detector slots.
- The UI can select the detail pre-upscale backend independently of detector
  selection.
- Candidate provenance records detector id, confidence threshold, region count,
  crop size, and detail render edge.
- Missing detector models fail clearly and do not break base inpaint/outpaint.

## Open Questions

- Should custom detailers be user-editable in the model manager first, or should
  we start with a JSON registry and add UI editing later?
- Should body detailing use bbox-only person detection or segmentation by
  default?
- Should overlapping detailer crops be processed largest-to-smallest,
  smallest-to-largest, or by target priority?
- Should detailer results appear as temporary candidate overlays before they are
  merged back into the candidate image?
