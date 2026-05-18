# Underpaint Project Plan

This plan is a living roadmap for turning the Drawpile fork into Underpaint: a GPLv3, local-first, AI-assisted photo restoration workspace.

## Guiding Constraints

- Preserve Drawpile's collaboration/session machinery.
- Keep the main app C++/Qt.
- Run AI out of process behind a stable job API.
- Treat AI results as layers, masks, maps, annotations, and candidates.
- Optimize local workflows for RTX 4070-class hardware.
- Avoid node-based user workflows.
- Prefer smallest high-quality models with license-clean defaults.
- Keep premium features as services or providers, not secret code glued into the desktop app.

## Milestone 0: Fork Stabilization

Goal: make the fork clearly belong to Underpaint while preserving upstream provenance.

Tasks:

- Rename visible project surfaces from Drawpile to Underpaint where appropriate.
- Keep inherited Drawpile docs under `docs/drawpile/`.
- Add Underpaint docs under `docs/`.
- Verify remotes: `origin` should be `LynnColeArt/underpaint`, `upstream` should remain Drawpile.
- Establish upstream sync policy.
- Establish license and model-license policy.

Acceptance criteria:

- Fresh clone identifies as Underpaint.
- Drawpile provenance remains clear.
- Local branch tracks the Underpaint repo.
- Docs explain the fork thesis and early roadmap.

## Milestone 1: Build And Runtime Baseline

Goal: make the existing app build and run locally before deep changes.

Tasks:

- Document Linux build dependencies.
- Resolve local Qt dependency gaps.
- Build client and server presets.
- Build a minimal test target.
- Record known build blockers.
- Keep the current build probe notes in `docs/build-baseline.md`.

Acceptance criteria:

- A developer can build the app from documented commands.
- Any missing optional dependencies are named.
- At least one test target or smoke run is verified.

## Milestone 2: Restoration Workspace Shell

Goal: make the first screen feel like a restoration workspace rather than a collaborative drawing room.

Tasks:

- Keep layers, canvas, masks, and collaboration substrate intact.
- Introduce restoration-oriented wording and layout.
- Add top-level AI/restoration actions as intentional user-triggered entry
  points.
- Add a first-step flow for opening a photo.
- Preserve advanced drawing tools where they help mask/repair workflows.
- Decide which social/collab UI is visible by default.

Acceptance criteria:

- User can open an image and understand the restoration workspace.
- AI operations are visible as explicit actions, not automatic canvas behavior.
- Collaboration still works or is explicitly hidden behind a stable path.
- No core Drawpile session behavior is removed without a written reason.

## Milestone 3: AI Job Boundary

Goal: add the first end-to-end AI operation path without committing to final model choices.

Operation shape:

```text
source layers + mask/region + parameters -> AI job -> candidate layer group
```

Tasks:

- Define AI job request/response schema.
- Add C++ request/response types for the schema.
- Export active selection or region as image plus mask.
- Include source metadata and operation parameters.
- Add a local worker process stub.
- Add C++ runner that launches the worker out of process.
- Return a generated or placeholder image as a new layer.
- Store provenance on the result layer/group.

Acceptance criteria:

- The job schema is documented and available to C++ editor code.
- The editor has a native runner for request/response worker execution.
- The app can submit a selected region to a worker and receive a new candidate layer.
- The worker can crash or fail without crashing the app.
- The result is undoable or removable through normal layer behavior.

## Milestone 4: Generative Region Operations

Goal: implement Photoshop-like generative fill and intentional outpaint UX.

Tasks:

- Add a Generative Fill panel for active selections.
- Add prompt, seed, CFG, denoise, quality preset, context padding, and variation count.
- Generate multiple candidates as a layer group.
- Add rerun-with-same-settings and rerun-with-new-seed.
- Add intentional outpaint entry point for empty canvas expansions.
- Ensure canvas resize never triggers automatic outpaint.

Acceptance criteria:

- User can select an area, generate candidates, and choose one.
- User can expand canvas and intentionally outpaint only when requested.
- User can set seed/CFG/denoise for repeatable, controlled outputs.

## Milestone 5: Layer Separation Pass

Goal: decompose a photo into editable, mask-backed scene regions.

Tasks:

- Run promptable or automatic segmentation on the visible image.
- Create proposed scene regions.
- Name/group regions using a lightweight captioning/object-labeling pass.
- Let users accept, rename, merge, split, hide, show, and refine regions.
- Generate background repair candidates behind selected separated objects.

Acceptance criteria:

- User can click Separate Image and receive editable proposed regions.
- Each accepted region has a mask and optional extracted layer.
- Repair fills are non-destructive and appear as candidate layers.

## Milestone 6: AI Preferences

Goal: make AI behavior predictable and user-configurable.

Status: initial preferences shell exists in the desktop client.

Tasks:

- Add AI Preferences page.
- Add default variation count per operation.
- Add default seed mode, CFG, denoise, quality, and context padding.
- Add hardware profile and VRAM budget.
- Add output behavior defaults.
- Add cloud/privacy prompts.

Acceptance criteria:

- Generative fill, outpaint, and scene repair use global defaults.
- Operation panels can override defaults.
- Cloud jobs require explicit user permission.

## Milestone 7: Model Manager

Goal: manage model capabilities, licenses, installs, providers, and memory behavior.

Status: initial model-role inventory shell exists in the desktop client.

Tasks:

- Add model registry format.
- Track capability, license, backend, precision, size, and memory estimates.
- Add install/uninstall/update state.
- Add local provider selection.
- Add cloud provider placeholder.
- Add VRAM-aware job admission and model unload policy.

Acceptance criteria:

- User can see which capabilities are available.
- The app can explain why a model cannot run locally.
- The runtime can unload and swap model families between jobs.

## Milestone 8: First Local Model Pack

Goal: ship a useful local AI pack before chasing every possible model.

Initial candidate capabilities:

- segmentation
- object/region naming
- background removal/matting
- depth map
- detail upscale
- face restoration
- XL-class generative fill/outpaint

Acceptance criteria:

- All included models have documented licenses.
- All models fit the RTX 4070 target with conservative settings.
- Each model output becomes an inspectable layer/mask/map.

## Milestone 9: Control And Advanced Guides

Goal: support control maps without exposing node graphs.

Tasks:

- Add "Create Guide Layer" operations.
- Support depth, normal, pose, edge, and segmentation guide layers.
- Let generative fill/outpaint use guide layers.
- Add provenance for guide usage.

Acceptance criteria:

- User can create a guide map and select it as guidance for a generation.
- The user never has to assemble a node graph.

## Milestone 10: Agents And Collaboration

Goal: make agents and cloud workers fit the collaboration model.

Tasks:

- Define agent-as-participant permissions.
- Add an MCP bridge prototype.
- Add tool schema for layer/selection/job operations.
- Let agent actions create proposed artifacts.
- Log agent actions in project/session history.

Acceptance criteria:

- An agent can create a proposed layer or annotation without destructive edits.
- Agent permissions are visible and scoped.
- Human users can accept, hide, delete, or rerun agent output.

## Milestone 11: Community Release

Goal: release a coherent GPL community edition.

Tasks:

- Package app for Linux first.
- Document model install flow.
- Document privacy and cloud behavior.
- Publish source and build instructions.
- Prepare issue templates and contribution guide.

Acceptance criteria:

- A user can install Underpaint, open a photo, run at least one local AI operation, and save the result.
- A developer can build and modify the app from public source.
