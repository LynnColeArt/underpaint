# Fooocus Adapter Plan

The Fooocus inpaint patch should stay separate from the regular Diffusers
inpaint path until it proves end-to-end generation. The patch is compatible
with regular SDXL checkpoint key layout, but it is not a normal Diffusers model
repository.

## Current Evidence

- Assets downloaded to `~/.underpaint/models/inpaint/fooocus/`.
- Probe script: `tools/ai/underpaint-fooocus-patch-probe.py`.
- Local Juggernaut X v10 probe:
  - head shape: `[320, 5, 3, 3]`
  - patch keys: 960
  - Juggernaut X key compatibility: `960 / 960` when patch keys are prefixed
    with `model.`

## Why This Is A Separate Adapter

The Acly ComfyUI reference implementation does two things:

1. Applies Fooocus patch weights to the SDXL UNet in a LoRA-like way.
2. Computes an inpaint-head feature from mask and latent conditioning, then
   injects that feature into an input block during sampling.

The second piece is runtime behavior, not just model loading. Underpaint should
not pretend this is a `StableDiffusionXLInpaintPipeline.from_single_file(...)`
variant.

## Preferred First Implementation

Start with an external Comfy-style worker:

```text
Underpaint AI job
-> underpaint-fooocus-worker
-> Comfy/Fooocus patch runtime
-> candidate image + diagnostics
-> Underpaint candidate layer
```

The worker should still implement `underpaint.ai-job.v1`, so the C++ editor
does not learn Comfy graph details.

## Worker Requirements

- Accept the existing request/response/job-dir arguments.
- Load a regular SDXL checkpoint such as Juggernaut X v10.
- Load `fooocus_inpaint_head.pth` and `inpaint_v26.fooocus.patch`.
- Encode image and mask into the Fooocus-compatible inpaint latent.
- Apply the model patch and input-block injection.
- Emit preview events when available.
- Return RGBA candidate files matching the current worker behavior.
- Include provenance for:
  - base checkpoint
  - Fooocus head
  - Fooocus patch
  - prefill mode
  - sampler and step settings

## Open Questions

- Should the first worker shell out to a ComfyUI server API, or import Comfy
  modules directly in a managed runtime directory?
- Can we keep model CPU offload behavior comparable to the Diffusers worker on
  a 12 GB RTX 4070?
- Does the Fooocus patch behave better than true SDXL inpaint models for
  outpaint borders and small-context repair?
- What exact license/packaging boundary do we want if we ship a Comfy adapter?

## Acceptance Test

The first acceptable proof should use the same source image, mask, prompt,
seed, scheduler, and render edge as `tools/ai/underpaint-inpaint-model-smoke.py`
and produce a candidate that replaces the masked area while preserving outside
context.
