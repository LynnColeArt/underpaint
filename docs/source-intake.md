# Source Intake Register

This register tracks external projects, apps, model assets, and implementation
references used while building Underpaint. It is intentionally conservative:
if a project influenced implementation direction, appears in helper scripts, or
may become an adapter lane, list it here even when no source code has been
copied into the repository.

## Intake Rules

- Preserve Drawpile provenance and GPLv3 license continuity.
- Do not copy external source into Underpaint without recording the exact
  upstream project, files, commit or release, license, and local destination.
- Treat model weights separately from source code. Model licenses and
  commercial-use limits belong in the model registry and research notes.
- Prefer out-of-process adapters for heavyweight AI runtimes so the C++/Qt app
  remains responsive and license boundaries stay inspectable.

## Current Register

| Project or Asset | Role in Underpaint | Current Intake Status | License Notes |
|---|---|---|---|
| Drawpile | Upstream application substrate: canvas, layers, collaboration, session behavior, file/protocol compatibility, build system. | Forked source code is present in this repository. Preserve provenance in README, docs, headers, AUTHORS, ChangeLog, and LICENSE. | GPLv3. Underpaint remains GPLv3. |
| Hugging Face Diffusers | First runnable local image backend for SDXL inpaint, img2img/refiner experiments, worker callbacks, and memory/offload behavior. | Used as an installed Python dependency through `.venv`; no Diffusers source is vendored into this repo. | Apache-2.0 for Diffusers code. Model weights have separate licenses. |
| AUTOMATIC1111 Stable Diffusion WebUI | Reference implementation for inpaint behavior: mask blur, masked-content fill modes, crop-to-mask plus padding, inpaint conditioning, and latent blending concepts. | Source was consulted as implementation reference. No A1111 source files are copied into this repo. Underpaint implementations should be independently written and kept behind the AI worker contract. | AGPL-3.0 upstream; avoid source copying unless we deliberately accept and document compatibility obligations. |
| Fooocus inpaint assets | Experimental SDXL inpaint patch lane that may let regular photoreal SDXL checkpoints behave like inpaint models. | Model assets download to `~/.underpaint/models/inpaint/fooocus/`; not committed to the repo. `tools/ai/underpaint-fooocus-patch-probe.py` inspects compatibility only. No render path is wired yet. | OpenRAIL for Fooocus inpaint assets. Track exact files in model registry. |
| RealVisXL V4.0 Inpaint | Photoreal true SDXL inpaint candidate for the current Diffusers worker. | Model can be cached by Diffusers or downloaded with `tools/ai/download-underpaint-realvisxl-inpaint.sh`; weights are not committed to this repo. | OpenRAIL++ per Hugging Face tags. Track exact model card before bundling. |
| Acly ComfyUI Inpaint Nodes | Reference implementation for applying the Fooocus patch and understanding its mask/latent input-block injection. | Source was consulted as implementation reference. No ComfyUI node source is copied into this repo. A future adapter may call Comfy or implement a native equivalent. | Check upstream license before copying code. Prefer adapter boundary first. |
| ComfyUI | Candidate external worker style for graph/runtime adapter lanes, especially Fooocus patching. | No ComfyUI source or runtime is present in this repo. No local ComfyUI checkout was found during the Fooocus probe. | Check upstream license and extension licenses before vendoring or bundling. |
| Qwench | Temporary source of local llama.cpp runtime binaries and existing local Qwen model/projector paths for prompt-helper experiments. | Runtime may be used from local filesystem while Underpaint is young; Underpaint-specific model state should migrate to `~/.underpaint/`. No Qwench source is copied here. | Treat as local runtime dependency, not Underpaint source. |
| llama.cpp | OpenAI-compatible local prompt helper server runtime. | Used as an external executable when configured; no llama.cpp source is copied into this repo. | MIT for llama.cpp code; model licenses vary. |
| Bingsu ADetailer models | Face/body/hand detection weights for targeted detail passes. | Helper script downloads YOLO weights to `~/.underpaint/models/detail/adetailer/`; weights are not committed to this repo. | Model-card/license review required before bundling. |
| Ultralytics | YOLO runtime used by the detail-pass worker when ADetailer models are enabled. | Optional Python dependency; no Ultralytics source is vendored. | Check installed package license and deployment implications before packaging. |

## Recent Fooocus Probe

The Fooocus assets downloaded successfully to:

```text
~/.underpaint/models/inpaint/fooocus/
```

Local probe against the Juggernaut X v10 checkpoint found:

- `fooocus_inpaint_head.pth`: valid head shape `[320, 5, 3, 3]`
- `inpaint_v26.fooocus.patch`: 960 patch keys
- Juggernaut X key compatibility: `960 / 960` patch keys match when prefixed
  with `model.`

This means the weight patch appears compatible with the original SDXL
checkpoint layout. The remaining implementation issue is runtime behavior: the
Fooocus path also needs dynamic mask/latent input-block injection, so it should
be implemented as a Comfy-style adapter lane or a carefully written native
patcher rather than loaded as a normal Diffusers checkpoint.

## Recent RealVisXL Probe

RealVisXL V4.0 Inpaint was added as a true Diffusers inpaint candidate:

```text
model id: realvisxl-v4-inpaint-diffusers
model: OzzyGT/RealVisXL_V4.0_inpainting
launcher: tools/ai/run-underpaint-realvisxl-inpaint.sh
```

The first smoke run included initial Hugging Face caching and took about ten
minutes. A cached 512x512 smoke at 24 steps and denoise 1.0 completed in about
13 seconds and replaced the masked test region. Peak allocated VRAM was about
5.2 GB with model CPU offload enabled.

## When This File Must Change

Update this register whenever Underpaint:

- copies source code from another project
- adapts a nontrivial algorithm from another app
- adds a new external runtime, adapter, model family, or downloaded asset
- changes from "reference only" to "vendored code"
- starts bundling model weights or runtime binaries in release artifacts
