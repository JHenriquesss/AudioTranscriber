# QUALITY-GATES.md

## Completion Gate
This phase is complete only when:
- Planned implementation exists.
- Relevant automated tests exist.
- Applicable commands were run from this phase folder.
- Failures were fixed or documented with blocker, impact, and required fix.
- `PHASE-RESULT.md` exists before the final response.
- Final response is exactly `I finished the implementation`.

## Evidence Gate
`PHASE-RESULT.md` must list:
- Commands run.
- Commands passed.
- Commands failed.
- Commands not run and why.
- Tests added.
- Coverage/mutation readiness evidence when applicable.
- Complexity, architecture, dependency, security, and audit review.
- Evidence-based quality score.

## Test Gate
- Add tests for every changed behavior that is not purely visual.
- Tests must validate results, not only execute code.
- Include negative/failure-path tests for typed errors.
- Keep fixtures small and deterministic.
- Regression tests are required for any fixed broken behavior.

## Coverage And Mutation Gate
- Measure coverage when tooling is available.
- For critical validation/business rules, structure tests so mutation testing can be added.
- If mutation tooling is unavailable, document the exact blocker and which tests protect the rules.

## Complexity Gate
- Functions should stay near 40 lines when possible.
- Headers should stay near 150 lines when possible.
- Implementation files should stay near 300 lines when possible.
- If a file/function grows beyond the guideline, document why or refactor before completion.
- Avoid hidden state, global mutable state, broad managers, and generic utilities.

## Architecture Gate
- Preserve feature-oriented modular folders from `AGENTS.md`.
- UI stays thin and does not own business workflow.
- External tools and engines stay behind narrow module boundaries.
- No forbidden Clean Architecture folder names.
- Dependency direction must match the phase plan.

## C++ Gate
Follow `LANGUAGE-QUALITY-GATE.md` together with this file. The stricter rule wins.

## Security And Privacy Gate
- No hidden network calls.
- No telemetry in V1.
- No transcript content in logs by default.
- No secrets, keys, credentials, or local absolute paths committed.
- Dependency additions must be documented and justified.

## Scoring Caps
- No successful build: maximum 35.
- No automated tests for changed business logic: maximum 50.
- No `PHASE-RESULT.md`: maximum 40.
- Formatting not checked: maximum 75.
- Compiler warnings/static analysis not run or not documented: maximum 80.
- Architecture boundary violation: maximum 65 unless explicitly approved.
- Legal/security/critical integration changes without contract or failure-path tests: maximum 60.
