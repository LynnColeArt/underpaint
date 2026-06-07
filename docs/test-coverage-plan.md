# Test Coverage Plan

Underpaint should eventually have automated coverage for every feature that can
break without a person noticing. The goal is not a vanity percentage. The goal
is a useful safety net: fast unit tests for small logic, integration tests for
cross-process contracts, smoke tests for model/runtime lanes, and manual scripts
only where humans or GPUs are still genuinely required.

## Coverage Levels

- **Unit:** deterministic logic tested without a running app, network, or GPU.
- **Integration:** multiple local components tested together, such as C++ job
  runner plus worker stub.
- **Smoke:** a real workflow is launched with minimal inputs to catch packaging,
  runtime, or model regressions.
- **Manual:** a person must inspect UI behavior, image quality, or interaction.
- **Gap:** important behavior exists without a repeatable test yet.

Every new feature should land with at least one automated test or a documented
manual smoke path. Bugs should get a failing test first when the behavior can be
captured deterministically.

## Current Automated Coverage

| Area | Current level | Notes |
|---|---|---|
| Drawdance common utilities | Unit | C tests cover base64, file, queue, rect, and vector behavior. |
| Drawdance protocol messages | Unit | C tests cover protocol version and message read/write round trips. |
| Drawdance engine | Unit | C tests cover annotations, layers, metadata, timeline, project loading, and pixel conversion/blending. |
| Drawdance import/export helpers | Unit | C tests cover thumbnails and resize behavior with fixture images. |
| Shared Qt/server support | Unit | QTest covers password hashing, filenames, listings, ULIDs, message queues, and auth tokens when libsodium is present. |
| Client support utilities | Unit | QTest covers HTML utilities, listing filtering, and news parsing. |
| Server and thin server | Unit | QTest covers filed history, session bans, ID queues, server logs, server config, templates, and DB logs. |
| Underpaint AI job schema | Unit | Desktop QTest covers operation/status keys plus request/response JSON round trips. |
| Underpaint worker runner | Integration | Desktop QTest runs the compiled worker stub, captures progress events, validates candidate artifacts, and checks failure responses. |
| Diffusers worker pure logic | Unit | Python tests cover model registry selection, backend aliases, parameter clamping, alpha masks, padding, and detail crop sizing without loading GPU models. |
| Prompt helper pure logic | Unit | Python tests cover fallback prompt rewriting, system-prompt selection, helper URL resolution, JSON extraction, classification normalization, and group-refinement normalization. |

## Coverage Matrix

| Feature surface | Desired level | Current status | Next tests |
|---|---|---|---|
| App startup/version/help | Smoke | Covered by manual CLI smoke. | Add scripted smoke that runs `drawpile --version` and `--help` in CTest for desktop builds. |
| Canvas/layer core | Unit + integration | Strong inherited engine coverage. | Add tests for Underpaint-specific candidate layer grouping once that logic is split from main window code. |
| Selection export and masks | Unit + integration | Engine selection has coverage; Underpaint mask export is not isolated yet. | Extract mask/export helpers and test alpha-channel masks, odd sizes, feathering, and empty-selection behavior. |
| Inpaint request creation | Unit + integration | AI schema and worker runner are covered; UI request assembly is not. | Move request assembly into a testable helper and verify prompt, seed, mask, region bounds, and preferences. |
| Inpaint worker stub | Integration | Covered by desktop QTest. | Add golden-image or pixel assertions for mask application if placeholder output becomes important. |
| Diffusers inpaint worker | Unit + smoke | Pure functions covered; GPU generation covered by manual smoke script. | Add small CPU-allowed smoke for validation-only paths and nightly/manual GPU smoke for cached models. |
| Candidate chooser and preview | UI integration + manual | Gap. | Add testable model/state object for candidate visibility decisions before attempting GUI automation. |
| Object decomposition | Unit + integration + smoke | Worker-stub metadata is covered; Python pure helpers partly covered. | Add tests for region ranking/group metadata and a no-model fixture path that imports generated masks. |
| Background removal | Unit + smoke | Gap for automated worker behavior. | Add fixture test for response shape using rembg-unavailable fallback or a deterministic fake backend. |
| Prompt helper | Unit + integration | Fallback and JSON normalization are covered; live helper calls are not. | Add an integration test with a fake OpenAI-compatible HTTP server for request body shape and bad helper responses. |
| Proompt Manager | Unit + UI integration | Gap. | Extract prompt-history storage/search/delete behavior into a model with QTest coverage. |
| AI Preferences | Unit + UI integration | Shell exists; automated coverage gap. | Test settings persistence and request override/default resolution. |
| Model Manager and registry | Unit | Python registry selection partly covered. | Add C++ registry/parser tests for visible model roles, install states, and alternate registry paths. |
| Refiner/detail pass controls | Unit + smoke | Python normalization and crop sizing are covered. | Add tests for C++ parameter serialization and smoke detail-pass diagnostics with detectors absent. |
| Outpaint | Unit + smoke | Worker prompt/prefill helpers partly covered through pure worker tests. | Add request assembly tests, canvas-resize no-autofill guard, and tiny worker smoke with explicit mask. |
| Import/export file behavior | Unit + smoke | Inherited impex tests plus recent manual smoke. | Add regression fixtures for WebP transparency and file URL drops if they can be isolated. |
| Collaboration/session behavior | Unit + integration | Inherited server/protocol tests cover substrate. | Add Underpaint-specific assertions that AI artifacts stay normal session/layer events. |
| Packaging/install | Smoke | Gap. | Add Linux package smoke once packaging exists: install, launch `--version`, locate worker, locate docs/assets. |
| GPU/CUDA environment | Smoke | Manual checks only. | Keep fast CUDA availability check separate from expensive model generation; run full generation only manually/nightly. |
| External model/download scripts | Smoke | Manual. | Add dry-run or metadata validation mode for scripts so CI can verify paths and registry ids without downloading weights. |
| Cross-platform builds | CI smoke | Upstream workflow exists; Underpaint AI runtime packaging is Linux-first. | Add Windows/macOS smoke once runtime behavior is defined there. |

## Recommended Commands

Configure and run the full C++/Qt unit suite:

```bash
cmake -S . -B build-test-probe -G Ninja \
  -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
  -DQT_VERSION=5 \
  -DCLIENT=ON \
  -DSERVER=ON \
  -DSERVERGUI=OFF \
  -DBUILTINSERVER=ON \
  -DTESTS=ON \
  -DTOOLS=OFF \
  -DUPDATE_TRANSLATIONS=OFF \
  -DUSE_GENERATORS=OFF

cmake --build build-test-probe --parallel "$(nproc)"
ctest --test-dir build-test-probe --output-on-failure
```

Run the Python worker unit tests:

```bash
tools/ai/run-underpaint-python-unit-tests.sh
```

Run the current lightweight validation set:

```bash
.venv/bin/python -m py_compile \
  tools/ai/underpaint-diffusers-worker.py \
  tools/ai/underpaint-prompt-helper.py
tools/ai/run-underpaint-python-unit-tests.sh
ninja -C build-qt5-client-baseline drawpile underpaint-ai-worker-stub
git diff --check
```

Run a real model smoke only when GPU/model cache time is acceptable:

```bash
.venv/bin/python tools/ai/underpaint-inpaint-model-smoke.py realvisxl-v4-inpaint-diffusers
```

## Testing Rules For New Work

- Prefer unit tests for pure request-building, settings, parsing, masks, layer
  grouping, and model-registry logic.
- Use the worker stub for deterministic C++ integration tests.
- Keep heavyweight model tests opt-in unless they are tiny, cached, and reliable
  on RTX 4070-class hardware.
- Do not let hosted services or model downloads become required for normal
  CTest runs.
- When UI behavior is trapped inside `MainWindow`, extract the stateful decision
  into a small testable object before adding more UI code.
- Any feature that remains manual should have a named smoke script or checklist.
