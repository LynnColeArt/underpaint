# Rebrand Inventory

This inventory maps the current Drawpile identity surfaces so Underpaint can
become visible without breaking inherited compatibility, provenance, packaging,
or session behavior.

The scan intentionally treated `docs/drawpile/`, translations, generated build
directories, historical checksums, and release-history blobs as noisy by
default. Those areas still matter, but they are not good first-pass rename
targets.

## Working Rule

Rename surfaces that help a person understand they are using Underpaint.
Preserve surfaces that identify inherited Drawpile history, storage,
protocols, file formats, package identity, server behavior, or upstream
release artifacts.

## Keep As Drawpile Provenance

These references should remain explicit unless a later document says otherwise:

- `README.md` upstream/provenance links and the statement that Underpaint is
  derived from Drawpile.
- `docs/drawpile/`, which is the inherited upstream documentation area.
- `AUTHORS`, `ChangeLog`, `LICENSE.txt`, and source headers.
- AppStream and release notes that describe historical Drawpile releases.
- Comments explaining inherited Drawpile behavior.
- Paths, logs, or docs that record the baseline build output, such as
  `docs/build-baseline.md`.

## Low-Risk Public Identity

These are the best candidates for the next small implementation pass:

- `src/desktop/main.cpp`: `setApplicationDisplayName("Drawpile")`.
- `metadata/en-US/title.txt`.
- `metadata/en-US/short_description.txt`.
- The current-facing summary text in `metadata/en-US/full_description.txt`.
- The visible `Name=` and `GenericName=` fields in
  `src/desktop/drawpile.desktop.in`.

These changes affect what the app calls itself, not how it stores settings,
joins sessions, opens files, or speaks the existing protocol.

## Rename With Care

These are visible, but they have migration or packaging consequences:

- `src/desktop/main.cpp`:
  - `setOrganizationName("drawpile")`
  - `setOrganizationDomain("drawpile.net")`
  - `setApplicationName("drawpile")`
  - `setWindowIcon(QIcon(":/icons/drawpile.png"))`
- `src/thinsrv/main.cpp`:
  - `QCoreApplication::setOrganizationName("drawpile")`
  - `QCoreApplication::setOrganizationDomain("drawpile.net")`
  - `QCoreApplication::setApplicationName("drawpile-srv")`
- `src/desktop/drawpile.desktop.in`: `Exec=drawpile %u` and
  `Icon=drawpile`.
- `src/desktop/drawpile.appdata.xml` and
  `src/desktop/net.drawpile.drawpile.appdata.xml`: current app identity can be
  changed later, but historical release entries should not be mass-renamed.
- `cmake/DrawpilePackaging.cmake`: package vendor, executable display names,
  component descriptions, and product icon.
- `cmake/DrawpileInstallDirs.cmake`: install and app data directories.

The application and organization names feed `QSettings` and platform data
paths. Renaming them too early can make existing settings, recents, and app
data appear to vanish unless we provide a migration path.

## Defer Until Compatibility Plan

These are not good early rename targets:

- Top-level `CMakeLists.txt` project name and homepage.
- CMake module filenames such as `cmake/DrawpileOptions.cmake`.
- Binary and target names: `drawpile`, `drawpile-srv`, `drawpile-cmd`,
  `drawpile-timelapse`, and related packaging target names.
- Android package names under `src/desktop/android/src/net/drawpile/`.
- Logging categories such as `net.drawpile.*`.
- Protocol and URL scheme handling for `drawpile://`.
- File type names, MIME types, UTIs, magic values, and extensions in
  `cmake/DrawpileFileExtensions.cmake`.
- Project recording and clipboard identifiers such as `drawpile` and
  `x-drawpile/pastesrc`.
- Server/listing/update URLs that point at `drawpile.net`.
- `src/thinsrv/headless/headless.cpp` server identity strings and
  `drawpile.net` external-auth validation.
- `src/thinsrv/systemd/`, `src/thinsrv/man/`, and server helper scripts.
- Translation files. Rename source strings first, then regenerate translations
  deliberately.

These surfaces carry build, desktop integration, file association, network,
storage, or inherited service behavior. They should move only when there is a
specific Underpaint replacement and a rollback path.

## First Safe Code Slice

Completed in the first public identity pass:

1. Change the desktop application display name to `Underpaint`.
2. Update the English public metadata summary to describe Underpaint.
3. Update the English desktop entry name and generic name.
4. Leave binary names, settings namespace, package IDs, file formats, URL
   schemes, server names, icons, and Drawpile provenance untouched.
5. Build the Qt5 desktop client again and smoke-test `--version` and `--help`.

That gives the fork a visible identity without pretending the inherited
substrate has already been fully renamed.
