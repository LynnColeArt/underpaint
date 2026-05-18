# Underpaint

Underpaint is an experimental GPLv3 fork of [Drawpile](https://github.com/drawpile/Drawpile) exploring a local-first AI photo restoration and image reconstruction workspace.

The short version: we want AI restoration features that are approachable at first touch, but still precise enough for controlled image work, expressed through layers and masks instead of node graphs.

Underpaint is early. The current codebase is still mostly Drawpile, and that is intentional. Drawpile already has a serious collaborative paint editor foundation: canvas state, layers, masks, selections, transforms, import/export, project recordings, session history, chat, permissions, server hosting, and reconnect behavior. Those systems are interesting raw material for an AI restoration tool where generated work should become inspectable, editable artifacts rather than invisible magic.

## What Makes This Fork Interesting

Many AI image workflows either hide too much behind a single prompt or expose too much as technical plumbing. Underpaint is aiming for a different shape:

- **Original image stays sacred.** AI operations should create new layers, masks, maps, and candidate groups instead of destructively changing the source.
- **AI output is material.** A background removal produces a cutout and matte. A depth pass produces a visible guide layer. An inpaint produces candidate patch layers. A scene separation pass produces editable regions.
- **Layers replace nodes.** Internally, operations may use complex model chains. The user should still work with familiar art-tool concepts: selections, masks, layers, regions, prompts, seeds, CFG, denoise, and candidates.
- **Scene decomposition matters.** Underpaint's "magic layers" idea is not about finding fonts or design assets. It is about separating the image into meaningful visual regions and repairing what sits behind them.
- **Outpaint is intentional.** Expanding a canvas should create space, not automatically invent pixels. Outpaint should be a user-initiated generative fill operation.
- **Collaboration stays strategic.** Drawpile's collaborative substrate may become useful for shared restoration sessions, cloud rendering workers, agent participants, provenance, and review workflows.
- **Small, high-quality models are preferred.** The local target is RTX 4070-class hardware, so model choice, tiling, crop size, model swapping, and VRAM scheduling are core design concerns.

## Current Direction

The current product thesis is:

```text
approachable restoration tools
+ deep procedural control
+ layers and masks instead of node graphs
```

Planned AI capabilities include:

- scene/layer separation
- generative fill
- intentional outpaint
- background removal and matting
- detail-enhancing upscaling
- depth, normal, pose, edge, and segmentation guide layers
- face restoration with explicit identity-drift warnings
- model management and VRAM-aware scheduling
- optional cloud rendering/storage providers
- future MCP/agent control through scoped, undoable domain operations

## Project Docs

Underpaint planning docs live in `docs/`:

- [Underpaint Thesis](docs/underpaint-thesis.md)
- [Project Plan](docs/project-plan.md)
- [Architecture](docs/architecture.md)
- [Layer Separation And Generative Fill](docs/layer-separation-and-generative-fill.md)
- [Model Research](docs/model-research.md)
- [Build Baseline](docs/build-baseline.md)
- [Rebrand Plan](docs/rebrand-plan.md)

Inherited Drawpile docs are kept under `docs/drawpile/`.

## Build Status

The fork currently has verified Qt5 Linux baselines for:

- headless server
- desktop client

See [docs/build-baseline.md](docs/build-baseline.md) for exact configure/build commands and package notes.

The latest verified client smoke check was:

```bash
./build-qt5-client-baseline/bin/drawpile --version
./build-qt5-client-baseline/bin/drawpile --help
```

Qt6 is deferred until there is a concrete product or packaging reason to prioritize it.

## Upstream Drawpile

Underpaint is derived from Drawpile, a collaborative drawing program that lets people draw, paint, and animate together on the same canvas. Drawpile runs on Windows, Linux, macOS, and Android.

Original upstream:

- Drawpile repository: <https://github.com/drawpile/Drawpile>
- Drawpile website: <https://drawpile.net/>
- Drawpile build docs: <https://docs.drawpile.net/help/development/buildingfromsource>

This fork keeps Drawpile provenance visible while exploring a different product direction.

## License

Underpaint is based on Drawpile and remains under GPLv3. See [LICENSE.txt](LICENSE.txt).

Model weights and AI providers have their own licenses. Underpaint tracks model license and commercial-use status as a first-class model-manager concern.
