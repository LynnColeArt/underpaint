# Rebrand Plan

Underpaint starts as a clean public fork of Drawpile. The rebrand should make the new product thesis clear without breaking inherited build, protocol, file-format, or collaboration behavior.

The rule is: rename visible product surfaces first, preserve compatibility-sensitive internals until there is a specific reason to change them.

## Keep As Drawpile Provenance

These surfaces should continue to identify the upstream project or inherited material:

- `docs/drawpile/`
- `AUTHORS`
- `ChangeLog`
- `LICENSE.txt`
- upstream remote reference
- inherited protocol docs
- inherited file format docs
- source comments that describe Drawpile behavior
- compatibility strings needed for existing sessions/files

README and product docs should clearly say Underpaint is derived from Drawpile under GPLv3.

## Rename Soon

These are user- or contributor-facing and help the fork make sense:

- README title and description
- repository overview
- planning docs
- build baseline docs
- visible "what is this fork?" language
- issue/PR templates when added
- early package metadata once packaging begins

## Rename After Build Baseline

These should wait until after client/server builds are repeatedly green:

- application display name
- window title
- desktop entry/appstream metadata
- icons and branding assets
- settings namespace for new Underpaint-only settings
- app data directory for Underpaint builds
- generated config names

## Rename Later

These have more risk or compatibility impact:

- binary names
- CMake target names
- project name in top-level CMake
- installer names
- package identifiers
- internal namespaces/classes
- protocol-visible names
- server executable names
- file format extensions

## Do Not Rename Casually

Avoid changing these without a written compatibility plan:

- protocol identifiers
- session history formats
- project recording formats
- existing import/export identifiers
- network compatibility metadata
- Drawpile file extensions
- server/listing compatibility strings

## First Public-Face Pass

The first actual rebrand pass should:

- rewrite `README.md` around Underpaint
- explain why Drawpile is a strong base
- link to the thesis, project plan, architecture, model research, and build baseline
- keep upstream Drawpile links for provenance
- avoid changing build targets or source identifiers

## Second Pass

After the public face is coherent:

- inventory all visible strings containing `Drawpile`
- separate user-visible strings from compatibility-sensitive strings
- rename low-risk UI copy
- leave protocol/server/file-format names alone until design requires otherwise

