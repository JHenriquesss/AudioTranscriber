# Dev Implementation Environments

This folder contains isolated implementation environments. Each phase folder is meant to be handed to an implementation LLM with only one prompt:

```text
Implement the PHASE-PLAN(number).md following the rules of your AGENTS.md until the end and send me a message: (I finished the implementation) at the end
```

## Phase Order
1. `phase-01-foundation` - C++20/CMake foundation, shared primitives, first test harness.
2. `phase-02-transcript-export` - transcript model and TXT/SRT/VTT/JSON exporters.
3. `phase-03-app-runtime` - settings, paths, job lifecycle, cancellation, logs.
4. `phase-04-audio-ffmpeg` - process runner and FFmpeg audio normalization boundary.
5. `phase-05-whisper-pipeline` - whisper.cpp boundary and transcription pipeline.
6. `phase-06-desktop-ui` - Qt Widgets UI wired to app/runtime boundaries.
7. `phase-07-storage-packaging` - SQLite, model registry, packaging, release verification.

## Promotion Workflow
After a phase implementation returns `I finished the implementation`:

1. Read that phase's `PHASE-RESULT.md`.
2. Run the documented commands in the phase folder.
3. Write or update the phase's `CAVEMAN-QUALITY-REVIEW.md` with a 0-100 evidence score.
4. Fix gaps inside the phase until the score is 100/100 or all blockers are explicit and acceptable.
5. Copy accepted files into the project root.
6. Run root-level tests after each promotion.
7. Move to the next phase only after the promoted root state is green.

## CI Role
The reviewer acts as local CI until a real repository exists. Evidence beats opinion. A phase is not complete because code exists; it is complete when the relevant checks pass or blockers are documented honestly.
