# Underpaint Model Research

Date: 2026-05-17

This is a first-pass research matrix for Underpaint's AI model needs. The bias is toward the smallest model that can still produce high-quality restoration artifacts on RTX 4070-class hardware.

This is not a benchmark result. Every candidate below still needs local testing on restoration images.

## Selection Rubric

Prefer models that:

- produce useful layer/mask/map artifacts
- have license-clean community-edition use
- fit a 12 GB VRAM target with conservative settings
- support ONNX, TensorRT, Diffusers, or clean Python worker integration
- can run as scoped operations on crops, masks, or tiles
- can unload cleanly between jobs

Avoid defaulting to models that:

- require non-commercial weights for community/pro workflows
- require huge always-resident model stacks
- only work through opaque cloud APIs
- force users into node graphs
- cannot preserve provenance

## Initial Shortlist

| Capability | Default candidate | Why | License notes | Research status |
| --- | --- | --- | --- | --- |
| Promptable segmentation | SAM 2.1 Hiera Tiny or Small | Official SAM 2.1 has 38.9M tiny and 46M small checkpoints; Apache-2.0 checkpoints; supports image prompting and automatic masks. | Apache-2.0 for checkpoints/code. | Strong first default. Benchmark tiny vs small on photo restoration masks. |
| High-quality SAM-family cutouts | HQ-SAM ViT Base | Sharper mask decoder that works with the current Transformers worker shape through `SamHQModel` and `SamHQProcessor`. | Apache-2.0 per model card/repo. | Downloaded to `~/.underpaint/models/segmentation/sam-hq-vit-base/` as the first pluggable segmentation backend. |
| Region naming / object labels | Florence-2-base-ft | 0.23B model, MIT, supports captioning, object detection, dense region captioning, and region proposal. | MIT. | Good small generalist for naming/grouping masks. Test quality on old photos. |
| Open-vocabulary detection | Grounding DINO | Strong text-prompt detection and commonly paired with SAM. | Apache-2.0. | Useful when user prompts "separate the tree branches" or "find rocks". Heavier than Florence path. |
| Background removal / matting | BiRefNet variants | MIT repo, high-quality dichotomous segmentation/matting family, ONNX releases exist, 2025 HR/dynamic variants available. | MIT repo. Verify each weight card before bundling. | Primary research candidate for community build. |
| Background removal / commercial source-clean option | BRIA RMBG-2.0 | High-quality background removal, trained on licensed data, clean commercial API path. | Self-hosted HF weights are CC BY-NC 4.0; commercial requires agreement or API. | Do not bundle as default community weights. Good pro/cloud candidate. |
| Depth map | Depth Anything V2 Small | 24.8M params, Apache-2.0, official ONNX/TensorRT/community integrations listed. | Small is Apache-2.0; Base/Large/Giant are CC-BY-NC-4.0. | Strong default for local guide layers. |
| Normal map | StableNormal | Apache-2.0 repo, strong normal-estimation goal. | Apache-2.0 code/license. Verify weight terms before bundling. | Research candidate. Need size and runtime test. |
| Normal map fallback | Marigold Normals v1.1 | Official Diffusers model for normals, openrail++, high quality but diffusion-based and likely heavier. | OpenRAIL++. | Quality candidate, not "smallest" default. |
| Normal map non-default | DSINE | High-quality CVPR 2024 normal estimation. | Non-commercial/internal/academic license. | Do not default for community/pro unless relicensed. |
| Pose map | DWPose tiny/small | Official whole-body pose models from tiny to large; ONNX path; Apache-2.0. | Apache-2.0. | Better fit than OpenPose for local ControlNet-style guides. |
| Control-guided generation | XL-compatible ControlNet or adapter variants | Standard control mechanism for depth, pose, edge, normal, scribble maps, but must match the XL-class generation target. | ControlNet repo Apache-2.0; individual weights vary. | Need weight-specific license and XL compatibility review. |
| Inpaint / outpaint | XL-class inpainting models | SD 1.5 is below the product quality bar. XL-class models should be the first serious target, with careful crop/mask/context scheduling for 4070-class GPUs. | License varies by checkpoint; SDXL inpainting uses OpenRAIL++. | Primary diffusion research target. Benchmark specific XL inpaint models for quality, memory, speed, and restoration behavior. |
| Lightweight fallback fill / outpaint | Defer | Smaller models are tempting, but the product should not launch around visibly weak restoration fills. | TBD. | Research only after an XL baseline is working. |
| Detail upscale | Real-ESRGAN x2/x4plus and realesr-general-x4v3 | Standard practical upscaler; tiny general model is explicitly described as lower memory/time. | Real-ESRGAN repo BSD-3-Clause; verify weights. | Strong first default for detail enhancement. |
| Face restoration | GFPGAN v1.3/v1.2 | Apache-2.0, no custom CUDA extensions for clean version, integrates with Real-ESRGAN background upsampler. | Apache-2.0. | Strong first default, but UI must warn that identity may change. |
| Face swap | Defer | Sensitive consent/safety/licensing area; less central to restoration thesis. | TBD. | Keep out of first model pack. |

## Feature-To-Artifact Mapping

### Scene Separation

Inputs:

- visible image
- optional user prompt
- optional region hints

Likely model chain:

```text
SAM 2.1 automatic masks
+ Florence-2 or Grounding DINO labels
+ user review/refinement
= named region masks and optional extracted layers
```

Outputs:

- segmentation map
- named region masks
- extracted object layers
- source snapshot
- provenance

### Repair Behind Separated Object

Inputs:

- source image
- object mask
- context padding
- optional prompt
- generation defaults

Likely model chain:

```text
mask cleanup
+ XL-class inpainting provider
+ N variants
= candidate repair layers
```

Outputs:

- candidate fill group
- mask used
- source snapshot
- seed/CFG/denoise/steps metadata

### Manual Inpaint

Inputs:

- active selection or mask
- visible image or selected source layers
- optional prompt
- seed
- CFG
- denoise
- variation count

Outputs:

- candidate layer group
- hidden alternates
- mask used
- provenance

First local baseline:

- worker: `tools/ai/underpaint-diffusers-worker.py`
- default model: `diffusers/stable-diffusion-xl-1.0-inpainting-0.1`
- execution: one candidate at a time, using the same source crop and mask
- environment switch: `UNDERPAINT_AI_WORKER`

This is a baseline for quality and runtime behavior, not a final bundled model
choice.

### Intentional Outpaint

Canvas expansion is not outpaint. The user must initiate outpaint.

Inputs:

- expanded canvas with transparent area
- edge context
- optional prompt
- generation parameters

Outputs:

- outpaint candidate layer group
- edge/empty-area mask
- provenance

### Detail Enhance

Inputs:

- selected layer/group or flattened visible image
- optional mask
- scale factor
- quality/detail mode

Likely model chain:

```text
Real-ESRGAN x2/x4
+ optional GFPGAN face pass
= enhanced candidate layer
```

Outputs:

- enhanced layer
- optional restored-face sublayer
- provenance and model warnings

### Guide Layer Creation

Inputs:

- visible image or selected region

Outputs:

- depth map
- normal map
- pose map
- edge map
- segmentation map

These maps should be normal layers, not hidden internal tensors. A generative operation may reference one or more guide layers.

## 4070 Runtime Strategy

The local runtime should assume:

- one heavy XL-class diffusion family resident at a time
- preprocessors should unload or move to CPU after creating guide layers
- image generation variants should run sequentially by default
- VAE slicing should be used for multiple outputs
- VAE tiling should be available for large images
- CPU/model offload should be available but not treated as free
- TensorRT/ONNX should be used for preprocessors where conversion is stable
- region crops should be preferred over whole-canvas diffusion

Diffusers documents VAE slicing, VAE tiling, CPU/model/group offload, and memory-efficient attention as memory reduction strategies. ONNX Runtime's TensorRT provider supports C++ and Python usage, FP16/INT8 options, workspace limits, engine caching, and CUDA fallback.

## Model Manager Requirements From Research

The first machine-readable registry lives at `tools/ai/model-registry.json`.
It is intentionally small and editable while the local backend strategy is
still moving. The desktop Model Manager reads it for concrete entries, and the
worker can resolve `modelId` values from job parameters into backend/model
loader facts.

Each model entry should include:

- id
- display name
- capability list
- backend
- source URL
- license
- commercial-use status
- default precision
- disk size
- expected VRAM
- preferred input size
- max tested input size
- output artifact type
- compatible providers
- dependencies
- safety notes

Example:

```yaml
id: depth-anything-v2-small
capabilities:
  - depth_map
backend: python
license: Apache-2.0
commercial_use: allowed
preferred_precision: fp16
output_artifact: grayscale_layer
runtime_policy:
  keep_warm: false
  unload_after_job: true
```

Diffusion model entries should distinguish model format from operation role.
For example, an SDXL refiner can appear as a Diffusers repository or as a GGUF
file, but those require different runtime adapters:

```yaml
id: sdxl-refiner-q4-gguf
capabilities:
  - refine
backend: gguf
format: gguf
source_url: https://huggingface.co/gpustack/stable-diffusion-xl-refiner-1.0-GGUF
preferred_precision: q4_1
output_artifact: rgba_candidate
runtime_policy:
  keep_warm: false
  unload_after_job: true
  requires_external_runner: true
```

GGUF is attractive for RTX 4070-class local workflows because disk and memory
pressure may be lower than full SDXL Diffusers checkpoints. It should be treated
as an experimental backend lane until a concrete image runtime proves inpaint,
outpaint, refiner, progress, and output parity with the main job contract.

## Fooocus SDXL Inpaint Patch

The Fooocus inpaint assets are a promising way to decouple photoreal base-model
choice from inpaint capability. Instead of requiring a separate inpaint
fine-tune for every checkpoint, the Fooocus patch can be applied to regular
SDXL checkpoints and used with inpaint conditioning.

Important integration notes:

- The assets live at `lllyasviel/fooocus_inpaint` and should be stored under
  `~/.underpaint/models/inpaint/fooocus/`.
- The key files are `fooocus_inpaint_head.pth`,
  `inpaint_v26.fooocus.patch`, and `fooocus_lama.safetensors`.
- The patch is not a Diffusers drop-in. The ComfyUI reference implementation
  applies LoRA-like UNet patches and injects an input-block feature computed
  from mask and latent conditioning.
- The reference notes say regular SDXL checkpoints should be used; distilled
  Turbo, Lightning, and Hyper merges are not expected to behave well.
- Underpaint should treat this as a separate backend/adapter lane until a
  native patcher or a Comfy-style worker proves end-to-end generation.

Helper scripts:

```bash
tools/ai/download-underpaint-fooocus-inpaint.sh
tools/ai/underpaint-fooocus-patch-probe.py \
  --checkpoint ~/.underpaint/models/checkpoints/juggernaut-x-v10/Juggernaut-X-RunDiffusion-NSFW.safetensors
```

Initial probe against the local Juggernaut X v10 checkpoint found the Fooocus
head shape is valid (`[320, 5, 3, 3]`) and all 960 patch keys match the
checkpoint when the patch keys are prefixed with `model.`. That suggests the
weight patch is compatible with the original SDXL checkpoint key layout. The
remaining integration work is the dynamic input-block feature injection from
mask/latent conditioning.

External project and asset intake for Fooocus, ComfyUI references, Diffusers,
and other runtime dependencies is tracked in `docs/source-intake.md`.

## Photoreal SDXL Inpaint Candidates

RealVisXL V4.0 Inpaint is now tracked as a first photoreal candidate for the
existing Diffusers worker:

```text
model id: realvisxl-v4-inpaint-diffusers
model: OzzyGT/RealVisXL_V4.0_inpainting
launcher: tools/ai/run-underpaint-realvisxl-inpaint.sh
smoke harness: tools/ai/underpaint-inpaint-model-smoke.py
```

Initial results:

- first run cached the model and took about ten minutes
- cached 512x512 smoke at 24 steps completed in about 13 seconds
- peak allocated VRAM with CPU offload was about 5.2 GB
- the model replaced the masked test region at denoise 1.0

This is a practical near-term lane because it uses a true
`StableDiffusionXLInpaintPipeline` repository. Fooocus remains the larger
adapter lane for patching regular SDXL photoreal checkpoints.

## Detail Pass Runtime

Face/body/hand detailing should not run only at the full candidate image
resolution. Small detected regions need their own crop/detail pass:

```text
detection box -> padded square crop -> detail-enhancing upscale
-> inpaint crop at 1024+ px working width -> resize back -> blend into candidate
```

The current worker exposes `detailRenderEdge`, `minCropEdge`, and
`upscaleBackend` in `detailPass`. Refiner settings also carry an
`upscaleBackend` so small candidate images can be enlarged before the global
polish pass. Detail rendering should not run below a 1024 px working width.
The first-pass backend can use either plain Lanczos or Lanczos plus unsharp
masking as a lightweight detail-enhancing pre-pass before diffusion. A
model-backed Real-ESRGAN lane should replace or augment this once the upscaler
worker is implemented.

Detailing is detector-gated. The worker should not run a diffusion detail crop
unless an enabled detector returns at least one valid face/body/hand box after
class, size, aspect-ratio, and confidence filtering. If no valid boxes are
found, the detail pass should report `no-detections` and leave the candidate
unchanged.

## License Notes

License status must be tracked per model, not per feature.

Important examples:

- SAM 2.1: Apache-2.0 for model checkpoints/code.
- Florence-2-base: MIT.
- Depth Anything V2 Small: Apache-2.0.
- Depth Anything V2 Base/Large/Giant: CC-BY-NC-4.0.
- BRIA RMBG-2.0 HF weights: CC BY-NC 4.0; commercial use requires agreement or API.
- DSINE: non-commercial/internal/academic research license.
- GFPGAN: Apache-2.0.
- ControlNet code: Apache-2.0; weights need per-checkpoint review.
- SD 1.5 inpainting: CreativeML OpenRAIL-M, but not a product-default target.
- SDXL inpainting: OpenRAIL++.
- Fooocus inpaint patch: OpenRAIL.

## Sources

- SAM 2 official repo: https://github.com/facebookresearch/sam2
- Florence-2-base model card: https://huggingface.co/microsoft/Florence-2-base
- Grounding DINO official repo: https://github.com/IDEA-Research/GroundingDINO
- BiRefNet official repo: https://github.com/ZhengPeng7/BiRefNet
- BRIA RMBG-2.0 model card: https://huggingface.co/briaai/RMBG-2.0
- Depth Anything V2 official repo: https://github.com/DepthAnything/Depth-Anything-V2
- StableNormal repo/license: https://github.com/Stable-X/StableNormal
- DSINE license: https://raw.githubusercontent.com/baegwangbin/DSINE/main/LICENSE
- Marigold normals model card: https://huggingface.co/prs-eth/marigold-normals-v1-1
- DWPose official repo: https://github.com/IDEA-Research/DWPose
- ControlNet official repo: https://github.com/lllyasviel/ControlNet
- Diffusers inpainting docs: https://github.com/huggingface/diffusers/blob/main/docs/source/en/using-diffusers/inpaint.md
- Diffusers memory optimization docs: https://huggingface.co/docs/diffusers/main/optimization/memory
- SD 1.5 inpainting model card: https://huggingface.co/stable-diffusion-v1-5/stable-diffusion-inpainting
- SDXL inpainting model card: https://huggingface.co/diffusers/stable-diffusion-xl-1.0-inpainting-0.1
- Fooocus inpaint assets: https://huggingface.co/lllyasviel/fooocus_inpaint
- ComfyUI inpaint nodes reference: https://github.com/Acly/comfyui-inpaint-nodes
- Real-ESRGAN model zoo: https://github.com/xinntao/Real-ESRGAN/blob/master/docs/model_zoo.md
- GFPGAN official repo: https://github.com/TencentARC/GFPGAN
- ONNX Runtime TensorRT provider: https://onnxruntime.ai/docs/execution-providers/TensorRT-ExecutionProvider.html

## Next Research Tasks

- Benchmark SAM 2.1 tiny vs small on old-photo object separation.
- Test Florence-2-base-ft for region naming from SAM masks.
- Compare BiRefNet variants for hair, water, branches, and photo borders.
- Run Depth Anything V2 Small as a ControlNet guide layer candidate.
- Compare Real-ESRGAN x2plus, x4plus, and realesr-general-x4v3 on scanned photos.
- Test GFPGAN v1.2 vs v1.3 on historical portraits and document identity drift.
- Establish exact XL-class inpaint runtime settings for 4070: resolution, crop size, context padding, VAE tiling, batch size, variant scheduling, and model unload policy.
- Decide whether Fooocus inpaint patching should run through a Comfy-style
  external worker first or through a native Diffusers-compatible patcher.
- Create a model registry schema and fill it with the candidates above.
