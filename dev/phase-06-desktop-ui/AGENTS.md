# 

Behavioral guidelines to reduce common LLM coding mistakes. Merge with project-specific instructions as needed.

**Tradeoff:** These guidelines bias toward caution over speed. For trivial tasks, use judgment.

## 1\. Think Before Coding

**Don't assume. Don't hide confusion. Surface tradeoffs.**

Before implementing:

* State your assumptions explicitly. If uncertain, ask.
* If multiple interpretations exist, present them - don't pick silently.
* If a simpler approach exists, say so. Push back when warranted.
* If something is unclear, stop. Name what's confusing. Ask.

## 2\. Simplicity First

**Minimum code that solves the problem. Nothing speculative.**

* No features beyond what was asked.
* No abstractions for single-use code.
* No "flexibility" or "configurability" that wasn't requested.
* No error handling for impossible scenarios.
* If you write 200 lines and it could be 50, rewrite it.

Ask yourself: "Would a senior engineer say this is overcomplicated?" If yes, simplify.

## 3\. Surgical Changes

**Touch only what you must. Clean up only your own mess.**

When editing existing code:

* Don't "improve" adjacent code, comments, or formatting.
* Don't refactor things that aren't broken.
* Match existing style, even if you'd do it differently.
* If you notice unrelated dead code, mention it - don't delete it.

When your changes create orphans:

* Remove imports/variables/functions that YOUR changes made unused.
* Don't remove pre-existing dead code unless asked.

The test: Every changed line should trace directly to the user's request.

## 4\. Goal-Driven Execution

**Define success criteria. Loop until verified.**

Transform tasks into verifiable goals:

* "Add validation" → "Write tests for invalid inputs, then make them pass"
* "Fix the bug" → "Write a test that reproduces it, then make it pass"
* "Refactor X" → "Ensure tests pass before and after"

For multi-step tasks, state a brief plan:

```
1. \[Step] → verify: \[check]
2. \[Step] → verify: \[check]
3. \[Step] → verify: \[check]
```

Strong success criteria let you loop independently. Weak criteria ("make it work") require constant clarification.

\---

**These guidelines are working if:** fewer unnecessary changes in diffs, fewer rewrites due to overcomplication, and clarifying questions come before implementation rather than after mistakes.

---

# Phase-Specific Agent Contract

## Single Prompt Contract
The implementation LLM should receive only this prompt:

```text
Implement the PHASE-PLAN(number).md following the rules of your AGENTS.md until the end and send me a message: (I finished the implementation) at the end
```

The final response must be exactly:

```text
I finished the implementation
```

No extra words. No markdown. Do not send the final response until `PHASE-RESULT.md` exists and the available quality gates have been run or documented as blocked.

## Project Goal
Build an offline Windows desktop transcription application for Portuguese and English audio/video files.

## Stack
- C++20
- Qt 6 Widgets
- whisper.cpp isolated behind `src/whisper_engine`
- SQLite for local metadata/history
- FFmpeg as a bundled external executable behind `src/audio` and `src/platform`
- CMake Presets
- Windows installer or portable ZIP

## Architecture Rules
- Build a feature-oriented modular desktop application, not Clean Architecture.
- Allowed main paths: `apps/desktop`, `src/app`, `src/audio`, `src/transcription`, `src/whisper_engine`, `src/export`, `src/storage`, `src/platform`, `src/shared`, `tests`, `docs`, `tools`, `models`, `packaging`.
- Do not create `src/domain`, `src/application`, `src/infrastructure`, `usecases`, `entities`, or generic framework folders.
- UI must not call whisper.cpp, FFmpeg, SQLite repositories, exporters, or long-running transcription directly.
- Only `src/whisper_engine` may include or call whisper.cpp headers/APIs.
- FFmpeg command construction belongs only in `src/audio` or `src/platform`.
- Exporters depend only on the transcript model and shared primitives.
- Business workflow rules must be explicit in product modules, not hidden inside widgets, database code, XML/JSON serializers, or integration clients.
- The product must remain fully offline during normal use. No telemetry, accounts, browsers, Docker, Python runtime, cloud fallback, or hidden network calls.

## Development Rules
- Use TDD where practical: write failing behavior tests before implementation.
- Keep changes limited to this phase plan.
- Prefer simple C++ with RAII, explicit ownership, typed errors, and deterministic tests.
- Do not add abstractions for single-use code.
- Do not add features outside the current phase.
- Always respond, explain, and write code comments in English.
- Keep comments in English and use them only to explain non-obvious decisions.
- Do not log transcript content by default.
- Keep functions around 40 lines when possible, headers around 150 lines, and implementation files around 300 lines.

## Required Evidence
Before finishing, create `PHASE-RESULT.md` with:
- what was implemented
- what tests were added
- commands run
- commands passed
- commands failed
- commands not run and why
- known limitations
- quality score from 0 to 100
- remaining work required to reach 100/100

The quality score must be evidence-based:
- 0-40: code exists but is not safely verified
- 41-60: basic implementation with weak tests or unclear structure
- 61-75: working implementation with meaningful tests and acceptable structure
- 76-90: strong implementation with good tests, low complexity, and clean boundaries
- 91-100: production-grade implementation with strong automated evidence, clear boundaries, strong error handling, and no known quality gaps

## Required Checks
Run applicable checks from `QUALITY-GATES.md` and `LANGUAGE-QUALITY-GATE.md`. A missing tool is not a pass. Document the blocker, impact, and replacement evidence in `PHASE-RESULT.md`.
