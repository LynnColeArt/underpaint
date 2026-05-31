# Contributing to Underpaint

Underpaint welcomes human-written and AI-assisted contributions. The standard is
not whether a tool helped write the patch. The standard is whether the patch is
reviewable, licensed correctly, tested, maintainable, and aligned with the
project's direction.

Underpaint is a fork of Drawpile. Preserve Drawpile provenance and collaboration
behavior while moving this fork toward a local-first AI-assisted photo
restoration workspace.

## What We Value

- Small, focused changes that are easy to review.
- Clear bug reports with reproduction steps, expected behavior, and actual
  behavior.
- Test-driven fixes when a behavior can be captured in a test before the fix.
- Minimal implementations that solve the problem without speculative framework
  work.
- DRY code where duplication is real, not abstracted away before the shape is
  known.
- Conventional object-oriented C++ and Python that follow nearby code.
- Explicit provenance for copied, adapted, generated, or externally referenced
  material.

## AI-Assisted Contributions

AI tools are welcome when the contributor remains accountable for the result.

If you use AI assistance in a material way, disclose it in the pull request. A
good disclosure says what the tool helped with, what you reviewed manually, and
what verification you ran.

Acceptable AI assistance includes:

- generating first drafts of tests, docs, or implementation code;
- explaining unfamiliar code while you inspect it yourself;
- suggesting refactors that you then reduce, review, and test;
- helping produce reproduction scripts, diagnostic notes, or issue summaries.

AI assistance is not a substitute for contributor responsibility. Do not submit
code that you cannot explain, debug, license, or maintain. Do not paste large
generated changes without reducing them to a reviewable patch. Do not use hosted
AI tools on private user images, logs, credentials, unreleased source, model
weights, or other material you do not have the right to share.

AI-generated output must still meet the same bar as any other contribution:

- the patch must be your contribution under the project license;
- license-incompatible source or assets must not be copied into the repository;
- behavioral changes need tests or a clear manual verification path;
- the code should be simpler after review, not merely bigger;
- security, privacy, and user data implications must be called out.

## Project Boundaries

For Underpaint-specific work, keep these boundaries in mind:

- The main app remains C++/Qt.
- Heavy AI runtimes stay out of process behind the `underpaint.ai-job.v1` JSON
  contract.
- AI outputs should become inspectable layers, masks, maps, regions, and
  candidates.
- Inpaint and outpaint are explicit user actions.
- Drawpile collaboration, session, protocol, and file-format behavior should not
  be changed accidentally.
- Model weights and downloaded assets do not belong in the repository unless a
  license and packaging decision explicitly says otherwise.

## C++ Guidelines

- Follow the style and ownership patterns in the surrounding Drawpile code.
- Prefer clear Qt/C++ object lifetimes, RAII, and parent ownership where
  appropriate.
- Keep UI work responsive. Long-running work should not block the main thread.
- Avoid broad rewrites unless they are needed for the user-visible change.
- Keep public interfaces narrow and name them around the domain behavior they
  expose.
- Add tests for protocol, model, serialization, and behavior changes where the
  existing test surface makes that practical.

## Python Guidelines

- Keep AI workers and tooling explicit, boring, and easy to run locally.
- Prefer standard-library code unless a dependency is already part of the worker
  environment or is justified by the feature.
- Keep heavyweight model integrations behind worker or adapter boundaries.
- Validate JSON requests and fail with useful diagnostics.
- Avoid hidden network calls except for documented model downloads or explicitly
  configured services.
- Add focused tests or smoke commands for new worker behavior.

## Pull Requests

Good pull requests usually include:

- a short description of the problem and solution;
- screenshots or recordings for visible UI changes;
- reproduction steps for bug fixes;
- notes about Drawpile compatibility or collaboration/session risk;
- AI-use disclosure when applicable;
- verification commands and results.

For larger changes, split mechanical cleanup, tests, and behavior changes into
separate commits or pull requests when that makes review easier.

## Local Verification

Use the narrowest verification that proves the change. Common checks for the
current Linux development baseline include:

```bash
.venv/bin/python -m py_compile tools/ai/underpaint-diffusers-worker.py
ninja -C build-qt5-client-baseline drawpile underpaint-ai-worker-stub
git diff --check
```

If your change touches a different build target, test, worker, or UI workflow,
include the command or manual smoke path you used in the pull request.
