# Build Baseline

Date: 2026-05-17

This note records the first local build probe for the Underpaint fork.

## Current Repository State

- Branch: `main`
- Origin: `https://github.com/LynnColeArt/underpaint.git`
- Upstream: `https://github.com/drawpile/Drawpile.git`
- Latest checked commit during the successful server build: `ca217659e Document build baseline blockers`

## Local Toolchain

Observed tools:

```text
cmake 3.28.3
ninja 1.11.1
g++ 13.3.0
cargo 1.91.0
pkg-config 1.8.1
Qt 5.15.13
```

Additional packages installed after the first failed probe:

```text
libqt5websockets5-dev
libsystemd-dev
libsodium-dev
```

## Preset Shape

Drawpile's CMake presets all use `build/` as `binaryDir`. For exploratory probes, use explicit scratch build directories so failed configurations do not overwrite each other.

Example:

```bash
cmake -S . -B build-qt5-server-baseline -G Ninja \
  -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
  -DQT_VERSION=5 \
  -DCLIENT=OFF \
  -DSERVER=ON \
  -DSERVERGUI=OFF \
  -DBUILTINSERVER=OFF \
  -DTESTS=OFF \
  -DTOOLS=OFF \
  -DUPDATE_TRANSLATIONS=OFF \
  -DUSE_GENERATORS=OFF
```

## Minimal Server Probe: Working

Command:

```bash
cmake -S . -B build-qt5-server-baseline -G Ninja \
  -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
  -DQT_VERSION=5 \
  -DCLIENT=OFF \
  -DSERVER=ON \
  -DSERVERGUI=OFF \
  -DBUILTINSERVER=OFF \
  -DTESTS=OFF \
  -DTOOLS=OFF \
  -DUPDATE_TRANSLATIONS=OFF \
  -DUSE_GENERATORS=OFF
```

Initial result before dependency install:

```text
CMake Error:
Could not find a package configuration file provided by "Qt5WebSockets"
with any of the following names:

  Qt5WebSocketsConfig.cmake
  qt5websockets-config.cmake
```

Initial hard blocker:

```text
libqt5websockets5-dev is not installed.
```

After installing `libqt5websockets5-dev`, `libsystemd-dev`, and `libsodium-dev`, the same configure command succeeded and generated build files in `build-qt5-server-baseline/`.

Build command:

```bash
cmake --build build-qt5-server-baseline
```

Build result:

```text
[186/186] Linking CXX executable bin/drawpile-srv
```

Smoke check:

```bash
./build-qt5-server-baseline/bin/drawpile-srv --help
```

Result:

```text
Usage: ./build-qt5-server-baseline/bin/drawpile-srv [options]
Standalone server for Drawpile
```

## Core-Only Probe

Command:

```bash
cmake -S . -B build-core-baseline -G Ninja \
  -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
  -DCLIENT=OFF \
  -DSERVER=OFF \
  -DSERVERGUI=OFF \
  -DBUILTINSERVER=OFF \
  -DTESTS=OFF \
  -DTOOLS=OFF \
  -DUPDATE_TRANSLATIONS=OFF \
  -DUSE_GENERATORS=OFF
```

Result:

```text
-- Configuring done
CMake Error: AUTOMOC for target cmake-config:
Could not find moc executable target Qt5::moc
```

The `moc` executable exists at `/usr/bin/moc` and `/usr/lib/qt5/bin/moc`, but CMake generation still cannot resolve the imported `Qt5::moc` target in this stripped configuration.

The normal Qt5 server configuration no longer hits this issue after the server dependency set is installed. Keep this note only as a reminder that extreme stripped configurations can behave differently from supported presets.

## Missing Packages Observed During First Probe

Available but not installed:

```text
qt6-base-dev
qt6-websockets-dev
libkf5archive-dev
libzip-dev
libmicrohttpd-dev
qttools5-dev-tools
```

Installed relevant packages include:

```text
qtbase5-dev
qtbase5-dev-tools
libqt5opengl5-dev
libqt5webchannel5-dev
libssl-dev
libkf5archive-data
libqt5websockets5-dev
libsystemd-dev
libsodium-dev
```

## Suggested Linux Dependency Install

For the first Qt5 headless server baseline:

```bash
sudo apt-get update
sudo apt-get install -y --no-install-recommends \
  libqt5websockets5-dev \
  libsystemd-dev \
  libsodium-dev
```

For a fuller Qt5 client/server development baseline, expect to add:

```bash
sudo apt-get install -y --no-install-recommends \
  qttools5-dev-tools \
  libkf5archive-dev \
  libzip-dev \
  libmicrohttpd-dev
```

Qt6 baseline packages are available from Ubuntu, but not installed:

```bash
sudo apt-get install -y --no-install-recommends \
  qt6-base-dev \
  qt6-websockets-dev
```

## Original Environment Blocker

Installing packages from this session failed because `sudo` requires an interactive password:

```text
sudo: a terminal is required to read the password
sudo: a password is required
```

## Verified Server Baseline Command

```bash
cmake -S . -B build-qt5-server-baseline -G Ninja \
  -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
  -DQT_VERSION=5 \
  -DCLIENT=OFF \
  -DSERVER=ON \
  -DSERVERGUI=OFF \
  -DBUILTINSERVER=OFF \
  -DTESTS=OFF \
  -DTOOLS=OFF \
  -DUPDATE_TRANSLATIONS=OFF \
  -DUSE_GENERATORS=OFF

cmake --build build-qt5-server-baseline
./build-qt5-server-baseline/bin/drawpile-srv --help
```

## Next Baseline Targets

- Qt5 server with tests enabled.
- Qt5 client configure.
- Qt6 server configure once Qt6 dev packages are installed.
