# Feature Request: Cross-Platform AI Runtime Support

## Summary

Underpaint should support Windows, Linux, and macOS as first-class platforms.
The C++/Qt application, AI job contract, model manager, and layer/candidate
workflow should remain shared across platforms, while the AI runtime can choose
the best local or cloud provider for the user's hardware.

This is a follow-up feature request. It should wait until the first titular
underpainting/layer decomposition workflow is usable.

## Motivation

Underpaint's audience is not one operating system.

- Windows users are often already deep in AI-art tooling and NVIDIA workflows.
- Linux users are likely to be local-first hackers and early technical adopters.
- macOS users include serious art and design users who expect a polished
  creative application.

The product should not turn any of these users away. The right promise is not
identical performance everywhere. The right promise is that Underpaint runs
everywhere, uses local AI where the machine can support it, and routes cleanly
to another provider when local hardware is the wrong tool.

## Product Goal

Underpaint should provide a universal creative surface:

- Same restoration/layer/mask/candidate UI on Windows, Linux, and macOS.
- Same `underpaint.ai-job.v1` request/response contract.
- Platform-aware local workers.
- Platform-aware model compatibility and install state.
- Optional cloud providers without making cloud mandatory.

## Platform Profiles

### Windows NVIDIA

Intended status: first-class local high-performance target.

Expected runtime:

- CUDA Diffusers worker.
- Local SDXL-class inpaint/outpaint.
- Detailer/refiner workflows where VRAM allows.
- Local llama.cpp helper, optionally CUDA or Vulkan/CPU depending on install.

Needed work:

- Windows launcher scripts: `.ps1` and/or `.cmd`.
- `.venv/Scripts/python.exe` worker support.
- `%USERPROFILE%\\.underpaint` model/cache/log paths.
- CUDA/PyTorch install documentation.
- Confirm `QProcess` worker launch behavior with paths containing spaces.

### Linux NVIDIA

Intended status: first-class local high-performance target.

Expected runtime:

- CUDA Diffusers worker.
- Primary development and debugging target for early AI work.
- Local llama.cpp helper.

Needed work:

- Continue polishing current `.sh` launchers.
- Convert ad hoc launch scripts into model-manager/provider entries.
- Keep Linux instructions up to date.

### macOS Apple Silicon

Intended status: first-class creative app target with Metal-backed local AI
where practical.

Expected runtime:

- Full C++/Qt Underpaint UI.
- Local llama.cpp helper through Metal.
- Local segmentation, mask, guide, and prompt-helper workflows where practical.
- MPS/Metal diffusion as an experimental or conservative local render lane.
- Cloud provider handoff for heavy SDXL/refiner/detailer jobs.

Needed work:

- macOS launcher scripts.
- `~/.underpaint` model/cache/log paths.
- llama.cpp Metal helper setup.
- Benchmark MPS Diffusers for inpaint/outpaint.
- Define Mac-safe presets rather than reusing 4070 CUDA defaults blindly.
- Mark unsupported or experimental model/runtime combinations clearly in the
  model manager.

### CPU-Only

Intended status: usable app with limited local AI.

Expected runtime:

- Full editor UI.
- Model manager.
- Local lightweight helpers where tolerable.
- Cloud-ready render workflow.
- Local diffusion marked slow/limited.

## Model Manager Requirements

The model manager should report platform capability plainly:

- Available
- Installed
- Recommended
- Experimental
- Unsupported
- Cloud only

Compatibility should be based on:

- Operating system.
- GPU backend: CUDA, Metal/MPS, CPU, future Vulkan/DirectML/ONNX/TensorRT lanes.
- VRAM/RAM estimate.
- Model format.
- Worker availability.
- License and source metadata.

## Runtime Boundary Requirements

The AI worker contract must stay platform-neutral:

- File paths should be valid JSON strings and not assume POSIX separators.
- Workers should be launched as executables with arguments, not shell-only
  command strings.
- Environment variables should have documented Windows, Linux, and macOS forms.
- Job cancellation should terminate the active worker process on every platform.
- Worker errors should explain whether a failure is model, runtime, driver,
  memory, or platform compatibility.

## Acceptance Criteria

- The app can be built and launched on Windows, Linux, and macOS.
- Each platform has at least one documented local helper/runtime path.
- Model manager can explain why a model is available, experimental, or
  unsupported on the current machine.
- Inpaint/outpaint jobs can run locally on Linux and Windows NVIDIA machines.
- macOS can run local helper features and at least one documented local AI lane
  or a clearly marked cloud handoff.
- Cloud rendering is optional and explicit.

## Non-Goals For The First Pass

- Perfect performance parity across operating systems.
- Bundling every model/runtime into the desktop installer.
- Making CPU-only local diffusion feel fast.
- Replacing the JSON job contract.
