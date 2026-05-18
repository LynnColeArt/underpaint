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
- `generative-fill`
- `outpaint`
- `background-removal`
- `upscale`
- `depth-guide`
- `normal-guide`
- `pose-guide`
- `style-transfer`
- `face-restore`

## Request Shape

Requests contain exported editor assets and operation settings. Asset paths
should point at files in a temporary job directory owned by the editor.

```json
{
  "schema": "underpaint.ai-job.v1",
  "id": "d8b0f47e-8e0e-4f4e-b8ab-318c3e2a7a1a",
  "operation": "generative-fill",
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
    "uiEntryPoint": "AI/Generative Fill"
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
        "modelRole": "generative-fill"
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
