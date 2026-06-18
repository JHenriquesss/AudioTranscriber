# PROMOTION-CHECKLIST.md

Use this only after the implementation LLM creates `PHASE-RESULT.md` and sends the exact final message.

- [ ] Read `PHASE-RESULT.md`.
- [ ] Re-run all commands listed as passed.
- [ ] Re-run or inspect every command listed as failed/not run.
- [ ] Verify tests are meaningful and include failure paths.
- [ ] Verify architecture boundaries from `AGENTS.md`.
- [ ] Verify no forbidden folders or hidden network/cloud behavior.
- [ ] Verify quality score is evidence-based and capped correctly.
- [ ] Update `CAVEMAN-QUALITY-REVIEW.md`.
- [ ] Fix gaps until score is 100/100 or blocker is explicit and accepted.
- [ ] Copy accepted files to project root.
- [ ] Run root-level tests after copying this phase.
