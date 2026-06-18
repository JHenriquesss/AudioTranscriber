# Phase 07: Storage, Model Registry, Packaging, Release Checks

## Scope
Implement SQLite-backed history/metadata, model registry loading, portable folder behavior, packaging scripts, and release verification. This phase turns the tested workflow into a distributable Windows desktop product.

## Entry condition
- Phase 06 desktop workflow exists and can run a deterministic smoke path.
- SQLite, Qt deployment, and packaging tool availability are known or blockers are documented.

## Exit condition
- Database schema initializes and job/transcript/model metadata roundtrips in tests.
- Portable package layout is generated or packaging blockers are documented with replacement evidence.
- Final phase result includes release-quality evidence and promotion notes.

## Must-exist checklist
- [ ] `src/storage/Database.*`, `JobRepository.*`, `TranscriptRepository.*`, and `ModelRepository.*` exist.
- [ ] `models/index.json` contract is implemented and tested.
- [ ] `tools/package-windows.ps1` and `tools/verify-release.ps1` exist with documented commands.
- [ ] `packaging/windows/installer.iss` and `packaging/windows/app.manifest` exist if Inno Setup is available or blocker is documented.
- [ ] Tests verify schema creation, repository create/update/read, model registry parsing, and portable path behavior.

## Must-not-exist checklist
- [ ] No transcript body stored only in SQLite when architecture says JSON files remain inspectable.
- [ ] No production package that omits licenses for bundled third-party binaries.
- [ ] No installer/ZIP evidence based only on IDE build output.
- [ ] No online model downloader or hidden network call.
- [ ] No packaging script with absolute local machine paths.

## Test plan
### Positive
- [ ] Database initializes on first launch and runs migrations idempotently.
- [ ] Job and transcript metadata roundtrip through repositories.
- [ ] Portable package contains exe, models, tools/ffmpeg placeholder or documented binary, data, logs, exports, Qt runtime files, and licenses.

### Negative
- [ ] Database open failure maps to `DatabaseError`.
- [ ] Invalid model index returns clear typed error.
- [ ] Release verification fails when required files are missing.

## Test tree integration
- Trunk touch point: Completed job -> transcript/export metadata persisted -> app packaged -> clean release verification.
- New branches added: SQLite schema, repositories, model registry, portable path, package verification, license checklist.

## Quality gates for this phase
- Follow `QUALITY-GATES.md`.
- Follow `LANGUAGE-QUALITY-GATE.md`.
- Run or document these commands as applicable:
- `cmake --preset windows-msvc-release`
- `cmake --build --preset release`
- `ctest --test-dir build/windows-msvc-release --output-on-failure`
- `powershell -ExecutionPolicy Bypass -File tools/package-windows.ps1`
- `powershell -ExecutionPolicy Bypass -File tools/verify-release.ps1`
- `clang-format --dry-run --Werror <changed C++ files>`
- Document coverage evidence or why coverage tooling is unavailable.
- Keep business/workflow decisions out of UI, database, external tool, and engine adapter code.
- Create `PHASE-RESULT.md` before sending the final message.

## Next phase seed
After Phase 07, run root-level CI promotion: review every phase result, correct gaps, copy accepted files to root, and run full tests after each promotion.
