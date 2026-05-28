# Underpaint

Underpaint is an experimental GPLv3 fork of [Drawpile](https://github.com/drawpile/Drawpile) building a local-first AI photo restoration, object removal, and image reconstruction workspace.

The short version: Underpaint treats AI output as art-tool material. Model results should become editable layers, masks, guide maps, regions, candidates, and provenance instead of disappearing behind a prompt box or a cloud service.

Underpaint is early. The current codebase is still mostly Drawpile, and that is intentional while the fork is being established. Drawpile already has a serious collaborative paint editor foundation: canvas state, layers, masks, selections, transforms, import/export, project recordings, session history, chat, permissions, server hosting, and reconnect behavior. Those systems are useful raw material for an AI restoration tool where generated work should become inspectable, editable artifacts.

## Mission

Underpaint exists to explore a few connected problems:

- **Local-first AI art tools.** Many AI art workflows are cloud-first by default. Underpaint may eventually support cloud rendering and storage providers, but it is not designed as a cloud system. It is designed for users to run, inspect, modify, and own.
- **Privacy and ownership matter.** Underpaint should not depend on invasive telemetry or opaque hosted pipelines. We believe artists should be able to own their AI tools at the edge, keep sensitive images local, and decide when a cloud provider is involved.
- **AI should feel like an art tool.** Too many AI image systems treat the artwork as a prompt result. Underpaint starts from art-tool concepts: layers, masks, selections, brushes, candidates, guides, undo, provenance, and manual control.
- **Drawpile compatibility matters, but Underpaint is separate.** Underpaint is not associated with Drawpile and has a different mission. Drawpile remains upstream so provenance is clear and so Drawpile can adopt any useful changes if they want them.
- **Collaboration should be preserved without brand confusion.** Where possible, Underpaint will preserve compatibility with Drawpile's collaboration features and workflows while developing its own product identity.

## What Works Today

The prototype already has the first local AI workflow pieces:

- `Power Tools > Inpaint Selection...` exports the selected region, source context, and mask.
- XL-class Diffusers inpainting can run out of process on CUDA through the local worker.
- Inpaint results import as normal candidate layers that can be previewed, accepted, canceled, or undone.
- The inpaint dialog exposes prompt, negative prompt, candidates, seed, CFG, denoise, steps, and edge feather.
- The progress dialog streams worker preview images and uses a determinate progress bar.
- The `Proompt Manager` dock tracks the last 25 prompts used in the current document window for quick search, reuse, and deletion.
- `Power Tools > Object Decomposition...` imports SAM object/part masks as editable grouped layers.
- Object decomposition can be tested as a no-generation layer extraction pass before attempting base repair.
- A local llama.cpp/Qwen prompt helper can rewrite inpaint prompts, classify decomposition regions, and group related object parts.
- Underpaint-owned local model state is being organized under `~/.underpaint/`.

For day-to-day local AI testing, use the one-command launcher:

```bash
tools/ai/run-underpaint.sh
```

It starts the prompt helper if needed, waits until it is reachable, sets the
helper URL and Diffusers worker environment, then launches Underpaint.

## Linux Developer Install

Underpaint is still a development fork. The currently verified path is a Qt5
Linux build with local Python AI workers. Windows and macOS support are planned,
but the AI setup is not packaged yet.

On Ubuntu-like systems, install the native build dependencies:

```bash
sudo apt-get update
sudo apt-get install -y --no-install-recommends \
  build-essential \
  cmake \
  ninja-build \
  pkg-config \
  git \
  cargo \
  python3 \
  python3-venv \
  qtbase5-dev \
  qtbase5-dev-tools \
  qttools5-dev-tools \
  libqt5opengl5-dev \
  libqt5webchannel5-dev \
  libqt5websockets5-dev \
  libqt5svg5-dev \
  libssl-dev \
  libkf5archive-dev \
  libzip-dev \
  libmicrohttpd-dev \
  libsystemd-dev \
  libsodium-dev
```

Clone and build the Qt5 client:

```bash
git clone https://github.com/LynnColeArt/underpaint.git
cd underpaint

cmake -S . -B build-qt5-client-baseline -G Ninja \
  -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
  -DQT_VERSION=5 \
  -DCLIENT=ON \
  -DSERVER=OFF \
  -DSERVERGUI=OFF \
  -DBUILTINSERVER=OFF \
  -DTESTS=OFF \
  -DTOOLS=OFF \
  -DUPDATE_TRANSLATIONS=OFF \
  -DUSE_GENERATORS=OFF

cmake --build build-qt5-client-baseline --target drawpile
```

Set up the Python worker environment:

```bash
uv venv --python /usr/bin/python3 .venv
uv pip install --python .venv/bin/python -r tools/ai/requirements-diffusers.txt
```

If `uv` is not installed, a standard venv also works:

```bash
python3 -m venv .venv
.venv/bin/python -m pip install -U pip
.venv/bin/python -m pip install -r tools/ai/requirements-diffusers.txt
```

Optional local model helpers can be downloaded into `~/.underpaint/`:

```bash
tools/ai/download-underpaint-object-detector.sh
tools/ai/download-underpaint-sam-hq.sh
tools/ai/download-underpaint-birefnet.sh
```

Then launch the local AI build:

```bash
tools/ai/run-underpaint.sh
```

First launch may download Diffusers model weights into the normal Hugging Face
cache. The prompt helper also expects a local `llama-server` and compatible
Qwen model/projector; if those are missing, the launcher will report the missing
path instead of silently disabling semantic features.

## What Makes This Fork Interesting

Many AI image workflows either hide too much behind a single prompt or expose too much as technical plumbing. Underpaint is aiming for a different shape:

- **Original image stays sacred.** AI operations should create new layers, masks, maps, and candidate groups instead of destructively changing the source.
- **AI output is material.** A background removal produces a cutout and matte. A depth pass produces a visible guide layer. An inpaint produces candidate patch layers. A scene separation pass produces editable regions.
- **Layers replace nodes.** Internally, operations may use complex model chains. The user should still work with familiar art-tool concepts: selections, masks, layers, regions, prompts, seeds, CFG, denoise, and candidates.
- **Scene decomposition matters.** Underpaint's "magic layers" idea is not about finding fonts or design assets. It is about separating the image into meaningful visual regions and repairing what sits behind them.
- **Decomposition should be iterative.** The long-term target is Semantic Peel: identify one meaningful object, cut it into an editable layer, repair the plate behind it, record the decision, and repeat.
- **Outpaint is intentional.** Expanding a canvas should create space, not automatically invent pixels. Outpaint should be a user-initiated region operation beside inpaint.
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
- semantic peel object extraction and plate repair
- inpaint
- intentional outpaint
- background removal and matting
- detail-enhancing upscaling
- depth, normal, pose, edge, and segmentation guide layers
- face restoration with explicit identity-drift warnings
- model management and VRAM-aware scheduling
- document-scoped prompt history and reuse
- optional cloud rendering/storage providers
- future MCP/agent control through scoped, undoable domain operations

## Why Drawpile?

Underpaint starts from Drawpile because Drawpile already solves hard editor and
collaboration problems that are useful for this experiment: layered canvas
state, networked sessions, reconnect behavior, chat, permissions, history,
import/export, project recordings, and a mature C++/Qt desktop surface.

That does not make Underpaint a Drawpile product. Underpaint is an independent
fork with a different goal: local-first AI-assisted restoration and image
reconstruction. Drawpile remains upstream for license clarity, provenance, and
potential upstream collaboration.

The license is not changing. Underpaint remains GPLv3 because the project is
firmly committed to open source, user freedom, and the copyleft values that made
this kind of fork possible in the first place.

## Project Docs

Underpaint planning docs live in `docs/`:

- [Underpaint Thesis](docs/underpaint-thesis.md)
- [Project Plan](docs/project-plan.md)
- [Architecture](docs/architecture.md)
- [Layer Separation And Inpaint](docs/layer-separation-and-inpaint.md)
- [Semantic Peel Feature Request](docs/semantic-peel-feature-request.md)
- [Segmentation Backends Feature Request](docs/segmentation-backends-feature-request.md)
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

External apps, runtimes, implementation references, and downloaded model assets
are tracked in [docs/source-intake.md](docs/source-intake.md).
