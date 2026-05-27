# Feature Request: Familiar Photo Workspace

## Summary

Underpaint should provide a polished photo-editing workspace preset inspired by
the clarity of PhotoGIMP's GIMP profile: a compact left tool rail, familiar menu
organization, obvious color controls, and right-side docks that prioritize
layers, masks, candidates, and tool settings.

This is a UI and workflow feature request, not a request to copy another
application's source or branding. The goal is to make Underpaint feel more
finished and more immediately legible to artists who already understand common
photo-editing layouts.

## Motivation

Drawpile has many of the pieces Underpaint needs, but its default surface is
oriented around collaborative drawing rather than photo restoration and AI
assisted editing. Important tools are available but not always obvious.

PhotoGIMP demonstrates that a focused editorial pass can make a creative app
feel dramatically more approachable without changing the underlying engine. Its
strength is the default composition:

- a narrow vertical tool rail
- a large central canvas
- visible foreground/background color controls
- right-side docks for tool options and layers
- a menu structure that is easy for photo editors to scan
- a curated visible tool set instead of every possible tool competing at once

Underpaint should make a similar product choice in its own Qt/Drawpile-native
way.

## Product Goal

Underpaint should offer a "Photo Workspace" preset that becomes the default
public editing surface for the fork.

Expected behavior:

- The workspace looks and feels like a mature photo editor.
- Core restoration tools are visible without digging through menus.
- AI tools remain explicit actions in the `Power Tools` menu and related UI.
- Drawpile collaboration/session behavior remains available.
- The layout can be saved, restored, and reset.
- Advanced or collaborative panels can still be shown when needed.

## Menu Direction

The menu bar should be reorganized around photo-editing tasks while preserving
Drawpile capabilities.

Suggested top-level shape:

- `File`
- `Edit`
- `Image`
- `Layer`
- `Selection`
- `View`
- `Tools`
- `Filters`
- `Power Tools`
- `Window`
- `Help`

`Session` and collaboration-specific commands should remain available, but they
do not need to dominate the default menu layout. They can live under a
collaboration/session submenu or a dedicated workspace panel if that fits the
existing Drawpile architecture better.

## Tool Rail Direction

The left rail should expose the tools most likely to matter in photo
restoration, inpainting, masking, and layer manipulation.

Suggested visible groups:

- move and transform
- rectangle/ellipse/free selection
- inpaint selection action
- crop
- color picker
- heal/repair
- brush, pencil, and eraser
- clone or stamp-style tools if available
- fill and gradient
- blur/smudge/dodge-style retouch tools where available
- text
- zoom and pan

Less common tools should remain accessible through menus, grouped flyouts, or an
advanced workspace mode.

## Color And Material Layout

Color controls should be obvious and stable.

Suggested layout:

- foreground/background swatches in the lower-left tool area
- swap and reset controls near the swatches
- current brush/material preview near the tool rail or tool options panel
- detailed color picker in a right-side dock
- brushes, gradients, and patterns in nearby tabs

The goal is for color state to be inspectable at a glance, even when the user is
working mostly with selections, masks, and generated candidates.

## Dock Direction

The right side should prioritize editing state and inspection.

Suggested primary docks:

- tool options
- color
- brushes/materials
- layers
- masks
- generated candidates
- history/provenance
- model or worker status when relevant

Layer and candidate panels should feel like first-class editing surfaces, not
debug output. AI-generated content should continue to arrive as inspectable
layers, masks, and candidates.

## Source Intake Notes

PhotoGIMP is GPL-3.0 and is compatible with Underpaint's GPL direction, but its
files are GIMP profile/config files rather than reusable Qt UI code. Underpaint
should treat PhotoGIMP as a design reference and source-intake item.

Do not copy PhotoGIMP branding, splash art, icons, or product language. If any
specific configuration, shortcut mapping, or asset is copied later, record the
exact upstream file, commit, license, and destination in `docs/source-intake.md`.

## Non-Goals

- Replacing Drawpile's collaboration/session system.
- Copying GIMP configuration files directly into the Qt app.
- Using another product's branding or trademarked public language.
- Removing advanced tools from the app.
- Making a node-based AI workflow.

## Acceptance Criteria

- Underpaint has a selectable Photo Workspace layout preset.
- The default tool rail exposes the core photo-restoration tool set.
- Foreground/background colors are visible in a stable location.
- The right dock layout prioritizes tool options, colors/materials, layers,
  masks, candidates, and history.
- The menu layout includes `Image`, `Layer`, `Selection`, `Filters`, and
  `Power Tools` in a way that feels natural for photo editing.
- Drawpile collaboration/session commands remain reachable.
- The workspace can be reset if the user customizes or breaks the layout.
- Source intake is documented if any PhotoGIMP configuration or assets are
  copied rather than independently reimplemented.

## Open Questions

- Should Photo Workspace become the default immediately, or should it ship as a
  selectable preset first?
- Should collaboration/session commands stay top-level or move under a
  workspace-aware submenu?
- Should shortcut presets be part of this ticket or a separate follow-up?
- How much of the existing Drawpile left brush panel should remain visible by
  default?
- Should AI candidates get a permanent dock tab in the first pass?
