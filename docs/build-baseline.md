# Build Baseline

Date: 2026-05-17

This note records the first local build probe for the Underpaint fork.

## Current Repository State

- Branch: `main`
- Origin: `https://github.com/LynnColeArt/underpaint.git`
- Upstream: `https://github.com/drawpile/Drawpile.git`
- Latest checked commit: `7b78eb9e5 Document Underpaint product and model direction`

## Local Toolchain

Observed tools:

```text
cmake 3.28.3
ninja 1.11.1
g++ 13.3.0
cargo 1.91.0
pkg-config 1.8.1
Qt 5.15.13 base dev packages partially installed
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

## Minimal Server Probe

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

Result:

```text
CMake Error:
Could not find a package configuration file provided by "Qt5WebSockets"
with any of the following names:

  Qt5WebSocketsConfig.cmake
  qt5websockets-config.cmake
```

First hard blocker:

```text
libqt5websockets5-dev is not installed.
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

This may disappear once the normal Qt component set is installed and configured. If it persists after installing Qt WebSockets and related Qt tools, investigate the interaction between top-level `find_package(QT ...)`, `CMAKE_AUTOMOC`, and `cmake-config`.

## Missing Packages Observed

Available but not installed:

```text
libqt5websockets5-dev
qt6-base-dev
qt6-websockets-dev
libkf5archive-dev
libzip-dev
libsodium-dev
libmicrohttpd-dev
libsystemd-dev
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

## Environment Blocker

Installing packages from this session failed because `sudo` requires an interactive password:

```text
sudo: a terminal is required to read the password
sudo: a password is required
```

Next step after dependency installation:

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
```

