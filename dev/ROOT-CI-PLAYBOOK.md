# Root CI Playbook

Run this process after each isolated phase finishes.

## Phase Verification
- Read `PHASE-RESULT.md`.
- Re-run the phase commands.
- Compare produced files with the phase plan.
- Check architecture boundaries and forbidden folders.
- Check tests are behavior tests, not file-existence tests.
- Assign or update the Caveman quality score.

## Correction Loop
- Fix missing tests first.
- Fix implementation until tests pass.
- Refactor only what is necessary to reduce complexity or boundary violations.
- Re-run phase checks.
- Copy to root only after the phase reaches 100/100 or accepted blockers are explicit.

## Root Promotion Loop
For each phase in order:
1. Copy accepted files from the phase folder into the root project.
2. Run root build/test commands.
3. Fix root integration issues immediately.
4. Record the promotion in `sessions/YYYY-MM-DD.md`.
5. Continue to the next phase only when root is green.
