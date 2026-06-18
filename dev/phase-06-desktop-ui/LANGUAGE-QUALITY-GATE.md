## CPP-CODE-QUALITY-GATE.md

> Second-pass revision: this document is intentionally expansive. It is a C++-native quality gate, not a mechanical Go-to-C++ translation. Go-specific controls were replaced with C++ standard, compiler, build-system, linker, RAII, lifetime, undefined-behavior, ABI, template, sanitizer, header hygiene, dependency-management, and platform-portability controls.

## Purpose

This document defines the C++ language quality gate for this implementation phase.

Its purpose is to prevent low-quality C++ code from being generated, accepted, copied into the project root, or treated as complete without measurable evidence.

This is not a style preference document. It is an engineering control document.

C++ gives teams performance, deterministic resource management, low-level integration, strong abstraction tools, templates, generic programming, and control over layout and linkage. C++ code can still be poor software. C++ code can still be:

- architecturally wrong
- over-abstracted or under-abstracted
- undefined-behavior-prone
- lifetime-unsafe
- ownership-ambiguous
- exception-unsafe
- pointer-heavy
- data-race-prone
- deadlock-prone
- template-heavy without value
- ABI-breaking
- linker/build fragile
- warning-heavy
- sanitizer-hostile
- under-tested
- unauditable
- dependency-heavy
- supply-chain risky
- platform-specific without documentation
- legally/regulatorily wrong
- business-incorrect even when it compiles

The implementation is complete only when the code:

- configures, compiles, and links from the final repository state
- is formatted
- compiles under the project-approved compiler and C++ standard
- treats compiler warnings and configured static analysis as gates
- has meaningful automated tests
- preserves architectural boundaries
- models ownership and lifetimes explicitly
- avoids undefined behavior
- uses RAII for resources
- follows the project error-handling policy
- handles threads, atomics, coroutines, queues, and cancellation deliberately
- controls targets, include paths, macros, link flags, and dependency sprawl
- is secure by default
- is auditable where required
- has measurable evidence in PHASE-RESULT.md

This document must be followed together with:

- AGENTS.md
- PHASE-PLAN*.md
- QUALITY-GATES.md
- LANGUAGE-QUALITY-GATE.md
- architecture.md
- myrules.txt

If this file conflicts with a phase-specific rule, follow the stricter rule unless the deviation is explicitly documented in PHASE-RESULT.md.

---

## 1. Non-Negotiable Completion Rule

The implementation LLM must not declare the phase complete merely because C++ code was written.

A phase is complete only when:

1. The planned implementation exists.
2. Relevant automated tests exist.
3. The code configures, compiles, and links.
4. Formatting was checked.
5. Compiler warnings, static analysis, and configured linters were executed where available.
6. Applicable sanitizer/dynamic-analysis gates were executed where risk requires them.
7. The applicable quality gates were executed.
8. Failures were fixed or documented.
9. PHASE-RESULT.md was created.
10. The quality score is supported by evidence.

PHASE-RESULT.md must exist before the final message is sent.

The final message must be exactly:

```text
I finished the implementation
```

No extra words. No summary. No apology. No markdown.

### Second-pass C++ hardening

- Evidence must come from the exact repository state that will be handed off.
- Generated C++ counts as implementation unless explicitly excluded with a reason.
- A successful build alone is not enough when the phase changed ownership, public API, ABI, serialization, security, legal behavior, persistence, or concurrency.
- Any skipped command must include a concrete blocker.
- Any command run from the wrong directory is invalid evidence.
- Any command run before final code changes is stale evidence.
- Manual testing must be labeled manual and cannot replace automated tests for business logic.
- PHASE-RESULT.md must explain residual risk in plain language.
- Do not inflate the quality score for code that lacks failure-path tests.

---

## 2. C++ Toolchain and Standard Policy

## Recommendation

Use the compiler, standard library, language standard, generator, linker, and runtime already defined by the project.

## Always do

- Use the project-approved C++ standard.
- Use the project-approved compiler family and version range.
- Document actual compiler version output in PHASE-RESULT.md.
- Document standard library where relevant: libstdc++, libc++, or MSVC STL.
- Document OS, architecture, generator, build type, exception policy, RTTI policy, and ABI assumptions when relevant.
- Keep CI-equivalent commands runnable locally.
- Verify dependency additions do not silently require a newer C++ standard.
- Verify deployable binaries/libraries on intended target platforms where affected.

## Prefer

- Stable project-approved compilers.
- Explicit CMake presets, Meson native files, Bazel toolchains, or equivalent reproducible configuration.
- Warnings enabled for all first-party targets.
- Warnings-as-errors for new first-party code when project policy allows it.
- `compile_commands.json` for analysis tools.
- Clear Debug, Release, RelWithDebInfo, coverage, and sanitizer build profiles.
- Standard-library features before adding dependencies when compatible with the selected standard.

## Avoid

- Relying on whatever compiler happens to be installed locally.
- Raising the C++ standard without documenting impact.
- IDE-only build configuration.
- Local include/library paths.
- Changing exception, RTTI, visibility, or ABI settings without downstream review.
- Treating a local build as evidence for every platform.

## Almost never do

- Use experimental compiler branches in production business code without a documented reason.
- Disable warnings globally to finish a phase.
- Change target platform, standard library, exception policy, RTTI policy, or ABI assumptions without documenting why.

---

## 3. Build System and Reproducibility

## Recommendation

The build must be reproducible from a clean checkout using documented commands.

## Always do

- Use the project build system as the source of truth.
- Keep build definitions clean and minimal.
- Keep targets explicit.
- Scope include directories, link libraries, compile options, and compile definitions to targets.
- Avoid global flags unless project policy requires them.
- Keep dependency versions intentional.
- Use out-of-source builds.
- Use documented presets or documented command lines.
- Keep build behavior independent from IDE state and uncommitted local files.
- Document build commands in PHASE-RESULT.md.

## Prefer

- One target per coherent binary, library, plugin, or test.
- Correct PUBLIC/PRIVATE/INTERFACE usage in CMake or equivalent target visibility in other build systems.
- `CMAKE_EXPORT_COMPILE_COMMANDS=ON` for analysis.
- Reproducible generation and asset embedding.
- Separate production, test, benchmark, and tool targets.
- Explicit install/export rules for reusable libraries.

## Avoid

- One giant target that hides boundaries.
- Global `include_directories`, `link_directories`, or compiler flags for convenience.
- Absolute local paths.
- Build scripts that fetch remote resources during normal builds without approval.
- Missing dependencies that only compile because headers are leaked globally.
- Generated files with timestamps, local paths, or nondeterministic ordering.

## Almost never do

- Add a second build system to avoid fixing the existing one.
- Fetch and execute remote code during a normal build.
- Disable targets or tests to make a phase pass.
- Use local path overrides as an architecture escape hatch.

---

## 4. Dependency Manifest, Lockfile, and Vendor Policy

## Recommendation

C++ dependency management must be explicit and reproducible, whether the project uses CMake FetchContent, CPM, Conan, vcpkg, Bazel, submodules, vendored source, system packages, or another policy.

## Always do

- Follow the project-approved dependency mechanism.
- Commit manifests and lockfiles when the project uses them.
- Document dependency changes, options, profiles, triplets, overlays, remotes, and patches.
- Run tests after dependency graph changes.
- Run vulnerability/license checks when available and applicable.
- Avoid local-only path overrides.
- Verify dependencies do not alter exception, RTTI, ABI, allocator, threading, or standard-library assumptions silently.

## Prefer

- Version-pinned dependencies.
- Minimal dependency graph.
- Separate build-time, test-time, and runtime dependencies.
- Documented forks/patches with exit plans.
- SBOM generation when configured.

## Avoid

- Adding a dependency for trivial helpers.
- Blindly updating every dependency inside a phase.
- Ambient system packages without version documentation.
- Large vendor churn unrelated to the phase.
- Restrictive licenses without project-approved review.

## Almost never do

- Disable package integrity checks.
- Patch vendored code silently.
- Accept a critical advisory because it is transitive.
- Vendor binaries without source/provenance.

---

## 5. Mandatory Command Evidence

The implementation LLM must run applicable commands and document results in PHASE-RESULT.md. If a command cannot be run, document the concrete blocker.

Commands must be adapted to the project build system. The examples below assume CMake because it is common.

## Baseline commands

- _**`c++ --version`**_ or project compiler equivalent
- _**`cmake --version`**_ when CMake is used
- _**`cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS=ON`**_ or project preset equivalent
- _**`cmake --build build --parallel`**_
- _**`ctest --test-dir build --output-on-failure`**_
- _**`clang-format --dry-run --Werror <changed C++ files>`**_ or project formatting command
- _**`clang-tidy -p build <changed C++ files>`**_ or project static-analysis command when available
- _**compiler warnings check with project warning flags enabled**_
- _**dependency/package-manager verification command**_ when dependencies changed

## Stronger baseline for applications/services

- _**Debug build and tests**_
- _**Release or RelWithDebInfo build and tests**_
- _**clang-tidy / cppcheck / configured analyzer**_
- _**include-what-you-use**_ when configured
- _**coverage command**_ when configured
- _**vulnerability/license/SBOM command**_ when configured
- _**install/package target smoke test**_ when packaging changed

## Sanitizer and dynamic-analysis commands when applicable

- _**AddressSanitizer + UndefinedBehaviorSanitizer build and tests**_
- _**ThreadSanitizer build and tests**_ for concurrency-sensitive changes where supported
- _**MemorySanitizer build and tests**_ when supported and practical
- _**LeakSanitizer evidence**_ where available
- _**Valgrind / Dr. Memory / Application Verifier**_ where sanitizers are unavailable or project policy requires them

## Platform, ABI, and feature-configuration commands when applicable

- _**project-approved CMake/Meson/Bazel/MSBuild presets**_
- _**GCC and Clang builds when both are supported**_
- _**MSVC build when Windows is supported**_
- _**libstdc++ and libc++ builds when both are supported**_
- _**static and shared library builds when affected**_
- _**ABI checker command**_ when public ABI changed
- _**cross-compile or target-platform build**_ when deployables are affected

## Required evidence format

PHASE-RESULT.md must include:

```markdown
## Commands run

- `command here`

## Commands passed

- `command here`

## Commands failed

- `command here`
  - Reason:
  - Impact:
  - Required fix:
```

A command that was not run is not evidence. A command that failed but was ignored is negative evidence. A command that failed because a tool is unavailable must still be documented.

---

## 6. Formatting, Includes, and Style

## Recommendation

Use the project formatter. Do not debate whitespace. Automate it.

## Always do

- Run formatting checks.
- Use the committed `.clang-format` or project formatter policy.
- Keep includes clean.
- Remove unused includes.
- Keep headers self-contained.
- Avoid formatting churn unrelated to the phase.

## Prefer

- `clang-format` with committed configuration.
- Include ordering according to project policy.
- Include guards or `#pragma once` according to policy.
- Small files around cohesive responsibilities.
- Public headers that compile independently.

## Avoid

- Manual formatting fights.
- Long functions hidden by formatting.
- Header files that rely on include order accidents.
- Public headers full of implementation details.

## Almost never do

- Disable formatting checks.
- Commit generated formatting churn without reason.
- Use style to hide complexity.

---

## 7. Compiler Warnings, Static Analysis, and Lint Policy

## Recommendation

Compiler warnings and configured static analysis are quality gates.

## Always do

- Compile with project warning flags.
- Run configured static analysis.
- Treat new warnings as failures unless justified.
- Keep suppressions narrow and commented.
- Document new suppressions in PHASE-RESULT.md.

## Prefer

- GCC/Clang warnings such as `-Wall -Wextra -Wpedantic` plus project-approved stricter warnings.
- MSVC `/W4` or stricter where policy allows it.
- Warnings-as-errors for first-party code.
- `clang-tidy`, `cppcheck`, CodeQL, Semgrep, PVS-Studio, Coverity, or project-approved tools.
- Checks for bugprone, cert, cppcoreguidelines, modernize, performance, readability, security, lifetime, and concurrency risks.

## Avoid

- Broad `NOLINT` comments.
- Ignoring unchecked return values.
- Ignoring lifetime findings.
- Ignoring narrowing-conversion warnings.
- Debug prints in production code.
- Silencing warnings through casts without proving correctness.

## Almost never do

- Disable warnings globally for critical targets.
- Treat analyzer output as cosmetic when it identifies real risk.
- Suppress memory, race, security, or unchecked-error findings in legal/security/audit/persistence code.

---

## 8. Naming Rules

## Recommendation

Names must reveal intent.

## Always do

- Use domain language.
- Use precise names.
- Name functions by behavior.
- Name namespaces by responsibility.
- Name tests by expected behavior.
- Follow project C++ naming conventions.
- Use strong type names for important concepts.
- Distinguish raw data from validated data.

## Prefer

- `EmployeeExposurePeriod` over `PeriodData`.
- `ESocialEventBatch` over `Batch`.
- `SignedXmlDocument` over `XmlResult`.
- `OccupationalRiskAssessment` over `RiskInfo`.
- `EventTransmissionReceipt` over `ResponseData`.
- `RawEventPayload` and `ValidatedEventPayload` when state differs.

## Avoid

- `helper`, `utils`, `common`, `processor`, `manager`, `handler`, `data`, `stuff`.
- Ambiguous base classes such as `Manager` or `Processor`.
- Boolean parameters whose meaning is unclear.
- Generated names becoming domain vocabulary without review.

## Almost never do

- Use placeholder names in production code.
- Use single-letter names outside tiny scopes or mathematical formulas.
- Name domain types only after database tables or transport payloads.

---

## 9. Directory, Target, Header, and Namespace Structure

## Recommendation

C++ source layout must reflect architecture.

## Always do

- Preserve architecture.md.
- Keep domain logic separate from infrastructure.
- Keep application orchestration separate from adapters.
- Keep API/transport models separate from domain models.
- Keep external clients outside the domain.
- Keep public headers intentional and private headers private.
- Avoid circular includes and conceptual cycles.
- Align build target dependencies with architecture.

## Prefer

```text
app/
  service/main.cpp
include/project/
  public_api.hpp          # only if intentionally public
src/
  domain/
  application/
  infrastructure/
  transport/
  config/
tests/
  unit/
  integration/
  contract/
cmake/
```

## Avoid

- One huge `main.cpp`.
- One giant library target.
- A `common` directory that becomes a dumping ground.
- Domain targets linking infrastructure targets.
- Public headers exposed accidentally through install/export rules.

## Almost never do

- Put business rules in HTTP handlers, DB adapters, XML builders, generated payloads, message consumers, external clients, or CLI parsing.
- Let framework generators dictate architecture.

---

## 10. Architectural Boundaries

## Recommendation

Business rules must be explicit, isolated, and tested.

## Always do

- Keep dependency direction inward.
- Put business rules in domain/application code.
- Put side effects in infrastructure/adapters.
- Use interfaces/ports at boundaries where useful.
- Keep handlers thin.
- Make boundary crossing explicit with mappers.
- Keep framework-specific types out of the domain.

## Prefer

- Domain types with invariants.
- Application use cases for orchestration.
- Adapter implementations outside the core.
- DTO-to-domain mappers with tests.
- Target-level dependency restrictions.

## Avoid

- Domain code depending on HTTP frameworks, SQL drivers, logging frameworks, queue clients, or generated external payload headers unless architecture explicitly allows it.
- Infrastructure deciding domain outcomes.
- API DTOs reused as domain objects.
- Provider payloads leaking into core logic.

## Almost never do

- Hide business decisions inside SQL, XML serialization, mappers only, or incidental logs.
- Change architecture inside a phase without documenting the reason and residual risk.

---

## 11. C++ Type System as a Quality Tool

## Recommendation

Use C++ types to make invalid states hard to represent.

## Always do

- Use strong types for important identifiers and constrained values.
- Use `enum class` for closed sets.
- Keep invariants enforced at construction.
- Avoid exposing constructors or fields that allow invalid state.
- Distinguish raw, validated, signed, sent, rejected, and persisted states when useful.
- Make narrowing and unit conversions explicit.

## Prefer

- `EmployeeId` over raw `std::string`.
- `EventVersion` with validation.
- `SignedXmlDocument` over raw bytes/string.
- `DateRange` over two unrelated dates.
- `std::chrono` duration types over raw time units.
- Factories returning project-approved error results for invalid input.

## Avoid

- Generic maps as domain models.
- `void*`, `std::any`, unconstrained templates, or reflection-like type erasure as business data.
- Magic strings for states.
- Domain code accepting raw transport payloads.

## Almost never do

- Represent money, dates, measurements, certificates, event IDs, or legal codes as unvalidated primitives in domain code.
- Use comments to describe invariants that types could enforce.

---

## 12. Ownership, Lifetimes, References, Pointers, and Null

## Recommendation

Ownership and lifetime semantics must be obvious.

## Always do

- Choose value, reference, pointer, smart pointer, span, or view deliberately.
- Use references for required non-null borrowed values.
- Use `std::unique_ptr` for exclusive ownership.
- Use `std::shared_ptr` only for true shared ownership.
- Use `std::weak_ptr` to break shared cycles.
- Avoid raw owning pointers.
- Avoid dangling references, pointers, iterators, `std::span`, and `std::string_view`.
- Make null behavior explicit and tested.

## Prefer

- RAII ownership.
- Value objects.
- `std::optional<T>` for value absence.
- `std::span` and `std::string_view` only with clear lifetime.
- Defensive copies at boundaries when ownership is unclear.
- `gsl::not_null` or project equivalent where available.

## Avoid

- Raw `new` and `delete` in application/business code.
- `shared_ptr` by default.
- Passing smart pointers when a reference is enough.
- Returning references/views to destroyed temporaries or mutable internals.
- Storing non-owning views in long-lived objects without a documented lifetime invariant.

## Almost never do

- Use manual lifetime management in business code.
- Use `delete this`.
- Depend on static destruction order for business-critical cleanup.

---

## 13. Const-Correctness, Immutability, and Mutation

## Recommendation

Prefer immutable data and explicit state transitions.

## Always do

- Use `const` deliberately.
- Keep fields private when invariants matter.
- Make mutation explicit.
- Enforce invariants in constructors and mutation methods.
- Test state transitions.
- Do not let unrelated layers mutate domain state.

## Prefer

- Explicit methods such as `MarkAsSigned`, `MarkAsSent`, `Reject`, `Cancel`, and `Correct`.
- Immutable command/result structs.
- State-specific types for critical workflows.
- `const` member functions that are logically const and thread-safe under documented assumptions.

## Avoid

- Public mutable fields in domain objects.
- Setter-heavy domain objects that allow invalid intermediate states.
- `mutable` or `const_cast` to bypass design.
- Global mutable registries.

## Almost never do

- Use global mutable state for business behavior.
- Depend on destructors or static cleanup for business state transitions that can fail.

---

## 14. Error Handling

## Recommendation

C++ errors must be explicit, meaningful, and consistent with project policy.

## Always do

- Follow the project error-handling policy.
- Use exceptions, error codes, status objects, or expected-style results consistently.
- Preserve root cause information.
- Convert infrastructure errors at boundaries.
- Test failure paths.
- Distinguish business rejection from technical failure.
- Avoid leaking sensitive internals externally.

## Prefer

- `std::expected` or project-approved equivalent when explicit result handling is preferred.
- Exceptions only when project policy allows them and exception safety is designed.
- Domain-specific error types for structured business decisions.
- `std::error_code` / `std::system_error` for system-category failures where appropriate.
- Basic exception guarantee at minimum; strong guarantee where feasible.

## Avoid

- Throwing raw strings.
- Throwing from destructors.
- Returning null/default values for multiple failure modes.
- Parsing error strings in production logic.
- Mixing exception and non-exception APIs without clear boundaries.

## Almost never do

- Swallow errors.
- Let exceptions cross C ABI, plugin, destructor, `noexcept`, or thread boundaries unsafely.
- Treat eSocial/SST rejection as generic technical failure.

---

## 15. Assertions, Termination, TODO, and Fatal Policy

## Recommendation

Production C++ must not rely on assertions, termination, or process exit for normal control flow.

## Always do

- Avoid `std::terminate`, `std::abort`, `std::exit`, and fatal exits in library/domain/application code.
- Avoid assertion-based handling for recoverable failures.
- Remove TODO stubs, `throw std::logic_error("not implemented")`, `assert(false)`, and debug prints.
- Document intentional production fatal paths.

## Prefer

- Error results or exceptions with context.
- `assert` only for programmer invariants.
- `static_assert` for compile-time invariants.

## Avoid

- Assertions after parsing external input.
- Fatal exits after signing, hashing, DB, network, filesystem, XML, or JSON operations.
- Unhandled exceptions escaping thread functions.

## Almost never do

- Use fatal behavior in legal, security, audit, signing, persistence, or integration code.
- Call process exit from reusable libraries.

---

## 16. Initialization, Defaults, Nullability, and Absence

## Recommendation

Use default initialization, null, and optional values only when deliberate and tested.

## Always do

- Initialize variables before use.
- Use `std::optional` for legitimate absence of values.
- Use errors when failure reason matters.
- Keep default-state semantics clear.
- Test null, empty, default, and error cases.

## Prefer

- Clear constructors/factories that validate required values.
- Domain-specific absence states when absence has business meaning.
- Empty containers over null pointers when empty is valid.
- Member initializers and constructor initialization lists.

## Avoid

- Default constructors that create invalid objects.
- Optional booleans when a scoped enum clarifies meaning.
- `std::any` or pointers to avoid modeling absence.
- Returning default value and no error for invalid states.

## Almost never do

- Use null to hide technical failure.
- Depend on `memset` for non-trivial C++ object initialization.

---

## 17. Undefined Behavior and Low-Level Cast Gate

## Recommendation

Undefined behavior is forbidden by default.

## Always do

- Avoid UB.
- Avoid `reinterpret_cast`, pointer arithmetic, placement new, union punning, manual object-lifetime management, and C-style casts unless justified.
- Provide safety comments for non-obvious low-level operations.
- Expose safe abstractions over low-level internals.
- Run ASan/UBSan/fuzz tests where low-level code touches parsing, memory, or concurrency.

## Prefer

- Safe standard-library facilities.
- `std::byte`, `std::span`, `std::bit_cast`, and serialization helpers where appropriate.
- Static assertions for size/layout/alignment assumptions.
- Small private low-level modules.

## Avoid

- Signed overflow, out-of-bounds access, invalid iterators, strict-aliasing violations, uninitialized reads, use-after-move, dangling views, and data races.
- Layout assumptions without tests/static assertions.

## Almost never do

- Use low-level casts in business/legal/security logic.
- Introduce UB-prone code without sanitizer evidence and documentation.

---

## 18. C Interop, FFI, Plugins, and Native Boundaries

## Recommendation

C interop, FFI, dynamic loading, and plugin systems are high-risk integration code.

## Always do

- Keep FFI bindings outside domain logic.
- Wrap foreign handles with RAII.
- Validate inputs and outputs across the boundary.
- Document ownership, lifetime, thread-safety, allocator, and platform rules.
- Handle null pointers, error codes, errno, partial failures, and encoding issues.
- Avoid exceptions crossing C/FFI boundaries.

## Prefer

- Pure C++ alternatives when practical.
- Safe wrappers around C APIs.
- Integration tests with fixtures.
- Platform-specific files with explicit build conditions.

## Avoid

- Foreign pointers escaping into domain/application code.
- Mixing allocation/deallocation families.
- Assuming C strings are valid UTF-8.
- Loading plugins from untrusted paths.

## Almost never do

- Expose raw FFI directly to application/domain code.
- Add native dependencies without documenting build/deployment impact.

---

## 19. Concurrency and Thread Safety

## Recommendation

Concurrency must be explicit, bounded, observable, and tested.

## Always do

- Keep shared mutable state minimal.
- Use synchronization primitives deliberately.
- Define lock ownership and ordering.
- Avoid holding locks across slow operations.
- Set timeouts where blocking/waiting occurs.
- Test concurrency-sensitive behavior.
- Run ThreadSanitizer where supported and practical.
- Document concurrency assumptions.

## Prefer

- Immutable sharing or ownership transfer.
- RAII lock guards.
- `std::jthread` and `std::stop_token` where supported and approved.
- Bounded queues.
- Condition-variable waits with predicates.
- Deterministic cancellation/shutdown tests.

## Avoid

- Detached threads.
- Unbounded queues/thread creation.
- `volatile` for synchronization.
- Relaxed atomics without proof.
- Sleep-based synchronization in tests.

## Almost never do

- Fix races with sleeps.
- Hold locks while performing network, DB, signing, XML, filesystem, or slow operations.
- Build legal/eSocial/SST workflows on unsupervised background threads.

---

## 20. Threads, Async Work, Coroutines, Cancellation, and Lifetimes

## Recommendation

Threads, futures, callbacks, and coroutines must be designed, not sprinkled.

## Always do

- Use concurrency only when it solves a real problem.
- Pass cancellation/deadline signals through blocking operations.
- Join/stop owned threads.
- Propagate errors from threads/tasks/coroutines.
- Avoid detached fire-and-forget work.
- Test success, failure, timeout, and cancellation.

## Prefer

- Structured concurrency where available.
- Project-approved executors/thread pools.
- RAII task handles.
- Safe lambda captures.
- Bounded retries with idempotency.

## Avoid

- Capturing references in async callbacks without lifetime proof.
- Blocking forever on futures or condition variables.
- Coroutines without clear frame lifetime, cancellation, and executor behavior.

## Almost never do

- Hide failed tasks.
- Make legal/eSocial/SST sending fire-and-forget.
- Let callbacks outlive captured objects.

---

## 21. Interfaces, Templates, Concepts, and Generic Code

## Recommendation

Interfaces, templates, and concepts should model real variation or boundaries.

## Always do

- Keep interfaces small.
- Prefer concrete types when no meaningful abstraction exists.
- Use virtual interfaces only for real runtime variation.
- Use templates only when compile-time variation has clear value.
- Use concepts/constraints when they improve diagnostics and safety.
- Document public interfaces.

## Prefer

- Interfaces for repositories, clocks, signers, senders, storage, and publishers.
- Concrete domain types.
- `final` for classes not designed for inheritance.
- Virtual destructors for polymorphic bases.

## Avoid

- Interface for every class.
- Single-implementation interfaces without boundary value.
- Complex public template APIs without need.
- Inheritance for code reuse when composition is clearer.

## Almost never do

- Use templates to avoid modeling business concepts.
- Expose unconstrained generic APIs in domain code.

---

## 22. Code Generation, Macros, Reflection-Like Registries, and Metaprogramming

## Recommendation

C++ has powerful mechanisms that can hide behavior. Use them carefully.

## Always do

- Prefer ordinary C++ before macros, code generation, and metaprogramming.
- Keep generated code deterministic.
- Mark generated files clearly.
- Document generator commands and versions.
- Test generated behavior.
- Keep macro scope narrow and prefixed/namespaced.

## Prefer

- Generation only for schema/protocol code approved by the project.
- Golden tests for generated payloads.
- `constexpr` and functions over macros when possible.

## Avoid

- Macros for normal control flow.
- Macro pollution in public headers.
- Generator outputs that depend on local paths, timestamps, or network state.
- Hiding business rules in generated code or template expansion.

## Almost never do

- Generate UB-prone code without review and tests.
- Let generated DTOs dictate the domain model.

---

## 23. Serialization and Deserialization

## Recommendation

Serialization is a boundary concern.

## Always do

- Use DTOs at boundaries.
- Validate decoded data before domain use.
- Keep domain invariants independent from serialization annotations.
- Test required, optional, unknown, null, empty, malformed, oversized, and versioned payloads.
- Make defaults explicit and tested.

## Prefer

- Dedicated request/response structs.
- Explicit mappers from DTOs to domain types.
- Golden tests for stable payloads.
- Strict parsing where required.
- Stable date/time formats.

## Avoid

- Using domain structs as API DTOs by default.
- Silent defaults for required business fields.
- Leaking internal enum values into public contracts.
- Parsing untrusted input without size limits or error checks.

## Almost never do

- Treat decoding success as business validation.
- Use generic maps for legal/eSocial/SST payloads.

---

## 24. XML Safety

## Recommendation

XML processing must be hardened, especially for legal/regulatory payloads.

## Always do

- Use a dedicated XML library/encoder/parser.
- Avoid XML string concatenation.
- Validate schema where applicable.
- Test namespaces, encoding, required/optional fields, malformed XML, and external rejection paths.
- Keep schema version explicit.
- Keep canonicalization explicit when signatures are involved.
- Redact sensitive XML in logs.

## Prefer

- Version-specific XML models.
- Golden XML fixtures.
- Schema validation and canonicalization tests.
- Safe parser settings with external entity behavior reviewed.

## Avoid

- Building XML with `operator+`, `std::format`, `fmt::format`, or string streams.
- Ignoring namespaces or canonicalization.
- Logging complete sensitive XML.
- Letting XML builders decide business validity.

## Almost never do

- Generate legal XML without golden tests.
- Sign XML without deterministic canonicalization evidence.

---

## 25. Date, Time, Time Zones, and Clock

## Recommendation

Date/time bugs are business bugs.

## Always do

- Use `std::chrono` deliberately.
- Use date-only domain types for date-only concepts.
- Inject a clock when current time affects behavior.
- Test with fixed time.
- Define timezone and inclusive/exclusive range policy.
- Avoid local-machine timezone assumptions.

## Prefer

- Domain types for legal dates, periods, deadlines, validity windows, and timestamps.
- ISO-8601/RFC3339 at technical boundaries unless integration requires another format.
- Tests for month end, leap year, deadline boundaries, timezone conversion, and DST when relevant.

## Avoid

- Comparing dates as strings.
- Parsing dates repeatedly inside business rules.
- Raw integer timestamps without units.
- Tests depending on today’s date.

## Almost never do

- Use local machine time as business truth.
- Ignore timezone requirements in legal/eSocial/SST events.

---

## 26. Money, Decimals, Measurements, and Numeric Rules

## Recommendation

Use exact and domain-appropriate numeric types.

## Always do

- Define numeric units explicitly.
- Avoid magic numbers.
- Avoid floating point for money/legal/payroll calculations.
- Test boundary, rounding, minimum, maximum, zero, negative, fractional, invalid-unit, and overflow cases.
- Use checked/safe arithmetic where overflow matters.

## Prefer

- Domain value objects for money, percentages, measurements, rates, quantities, thresholds, and exposure levels.
- Integer minor units or approved decimal libraries.
- Constants named after business meaning.
- Types/fields that include units.

## Avoid

- Hidden unit conversion.
- Silent overflow.
- Narrowing conversions without review.
- Comparing floating-point values directly when exactness/tolerance matters.

## Almost never do

- Round legal/payroll/financial values without tests.
- Treat measurement units as comments instead of types/names.

---

## 27. Collections, Iteration, Algorithms, and Ranges

## Recommendation

Use the clearest collection construct, not the cleverest.

## Always do

- Choose containers by behavior.
- Make ordering explicit.
- Preserve deterministic output when contracts depend on it.
- Understand iterator/reference invalidation.
- Protect containers from concurrent mutation.

## Prefer

- `std::vector` for ordered contiguous collections.
- `std::map`/`std::set` when deterministic ordering matters.
- `std::unordered_map`/`std::unordered_set` when order does not matter.
- Plain loops for complex branching/error handling.
- Standard algorithms when they clarify logic.

## Avoid

- Hidden side effects inside algorithm predicates.
- Depending on unordered container iteration order.
- Returning views into temporary collections.
- Ranges pipelines that obscure lifetimes.

## Almost never do

- Hide persistence, network, signing, or message publishing side effects inside collection helpers.

---

## 28. Logging and Observability

## Recommendation

Logs are operational evidence, not decoration.

## Always do

- Use the project-approved logging/tracing system.
- Include correlation IDs where available.
- Never log secrets, private keys, passwords, tokens, certificate material, or raw sensitive legal/personnel/health payloads.
- Log failures with useful context.
- Make thread/task failures observable.
- Ensure logs do not replace audit trail.

## Prefer

- Structured logging.
- Stable event names.
- Redaction utilities.
- Domain identifiers instead of raw payloads.
- Separate audit records from debug logs.

## Avoid

- `std::cout`, `std::cerr`, `printf`, or debug dumps in production services.
- Logging entire XML/JSON payloads.
- Logging the same error repeatedly at every layer.

## Almost never do

- Log sensitive eSocial/SST XML unredacted.
- Hide failures because they were logged.

---

## 29. Auditability and Traceability

## Recommendation

Auditability is a business requirement when actions must be proven later.

## Always do

- Identify audit-critical flows.
- Record success and failure where policy requires it.
- Include stable identifiers, actor, tenant/account, event, batch, protocol, receipt, and correlation fields where safe.
- Avoid storing secrets or raw sensitive payloads.
- Test audit behavior and audit failure behavior.

## Prefer

- Append-only audit records.
- Stable event names and codes.
- Explicit mapping from business outcome to audit event.
- Redaction/hashing/encryption strategies for sensitive payload references.

## Avoid

- Logs as the only audit trail.
- Silent audit failures.
- Audit records that cannot be correlated with business records.

## Almost never do

- Ship legal/eSocial/SST operations without audit evidence.

---

## 30. Security Baseline

## Recommendation

C++ security quality requires memory safety, input validation, dependency hygiene, safe cryptography, and least privilege.

## Always do

- Treat external input as untrusted.
- Validate sizes, ranges, encodings, schemas, and formats.
- Avoid buffer overflows, use-after-free, double-free, dangling views, data races, and out-of-bounds access.
- Use approved cryptography.
- Do not log secrets.
- Run security/static/dynamic analysis where configured.
- Document security impact in PHASE-RESULT.md.

## Prefer

- Size limits on parsers and network input.
- Safe string/buffer APIs.
- Fuzzing for parsers/protocol handlers.
- Dependency vulnerability scanning.
- Least-privilege filesystem/network/database access.

## Avoid

- Disabling certificate verification.
- Shelling out with untrusted input.
- Loading plugins from untrusted paths.
- Silent fallback to insecure behavior.

## Almost never do

- Write custom encryption, signing, hashing protocols, or random number generators.
- Accept known critical vulnerabilities without mitigation.

---

## 31. Dependency and Supply Chain Hygiene

## Recommendation

Dependencies are code you ship or trust.

## Always do

- Keep dependency additions intentional and minimal.
- Prefer project-approved sources and registries.
- Verify integrity through lockfiles, checksums, package manager controls, or vendored review.
- Review license impact.
- Run vulnerability checks when available.
- Document runtime library/deployment impact.

## Prefer

- Stable, maintained libraries.
- SBOM generation where configured.
- Separate build-time, test-time, and runtime dependencies.
- Explicit patch management.

## Avoid

- Large frameworks for small features.
- Unpinned git dependencies.
- Unreviewed vendored drops.
- Shipping debug/test dependencies in production artifacts.

## Almost never do

- Execute downloaded scripts during build without review.
- Treat header-only as automatically low-risk.

---

## 32. Conditional Compilation, Platform Macros, and Feature Flags

## Recommendation

Conditional compilation must be explicit, tested, and minimal.

## Always do

- Isolate platform/compiler-specific code.
- Use build-system feature checks where possible.
- Test every affected configuration or document unverified ones.
- Keep defaults safe.
- Keep macro names namespaced/prefixed.

## Prefer

- Dedicated platform files and adapter targets.
- Small compatibility wrappers.
- CI matrix coverage for supported platforms.

## Avoid

- Deeply nested `#ifdef` logic.
- Business rules inside preprocessor branches.
- Feature flags that silently alter public API, ABI, serialization, legal, persistence, or security behavior.

## Almost never do

- Use preprocessor logic to bypass tests.
- Let `NDEBUG` change business correctness.

---

## 33. Public API and ABI Design

## Recommendation

Public C++ APIs and ABIs are commitments.

## Always do

- Identify whether changed code is public API or ABI.
- Keep public headers minimal and intentional.
- Document public behavior, ownership, lifetime, nullability, threading, and error/exception contracts.
- Preserve source/binary compatibility where promised.
- Review exported symbols.

## Prefer

- Pimpl or opaque handles where ABI stability matters.
- Explicit visibility/export macros.
- ABI checker tools for shared libraries.
- Downstream compile tests.

## Avoid

- Accidental public headers or exported symbols.
- Public mutable data members.
- Changing enum values, struct layout, virtual tables, or exception specifications without review.
- Leaking dependency types into public APIs unnecessarily.

## Almost never do

- Break public ABI silently.
- Expose private implementation headers through install rules.

---

## 34. Documentation and Examples

## Recommendation

Documentation must explain public behavior, invariants, ownership, and operational evidence.

## Always do

- Document public APIs where required.
- Document ownership, lifetime, nullability, error/exception, and thread-safety contracts.
- Keep docs updated with behavior changes.
- Keep examples buildable/testable where possible.

## Prefer

- Comments explaining why, not what.
- Contract comments at boundaries.
- README/architecture notes for new modules or targets.
- Migration notes for breaking changes.

## Avoid

- Stale comments.
- Hidden preconditions.
- Documentation-only invariants that code does not enforce.

## Almost never do

- Ship public C++ APIs without ownership and lifetime contracts.

---

## 35. Testing Strategy

## Recommendation

Tests must prove behavior, not merely execute code.

## Always do

- Add relevant automated tests for changed behavior.
- Test failure paths and edge cases.
- Test ownership/lifetime-sensitive behavior where possible.
- Test exception/error semantics.
- Test serialization/deserialization when contracts change.
- Keep tests deterministic.
- Document tests in PHASE-RESULT.md.

## Prefer

- Unit tests for domain behavior.
- Application tests with fakes.
- Integration tests for adapters.
- Contract tests for external APIs.
- Golden tests for stable payloads.
- Sanitizer runs for memory/lifetime-sensitive code.
- Fuzz tests for untrusted input.
- Regression tests for every fixed bug.

## Avoid

- Mock-only tests for critical business behavior.
- Tests that rely on sleeps or local machine state.
- Tests that only cover happy paths.
- Tests that only check non-null results.

## Almost never do

- Declare a phase complete with no automated tests for changed business logic.
- Remove failing tests to pass the gate.

---

## 36. Test Types Required by Risk

## Recommendation

Higher-risk code requires stronger tests.

## Always do

- Match test type to risk.
- Use domain tests for business rules.
- Use mapper tests for boundary conversions.
- Use serializer/deserializer tests for payload contracts.
- Use persistence tests for database mapping/query/migration behavior.
- Use fake-server/contract tests for integrations.
- Use audit tests for audit-critical flows.
- Use sanitizer/dynamic-analysis tests for memory, concurrency, and low-level code.

## Prefer

- Golden fixtures for legal/XML/API payloads.
- Contract tests for success, rejection, malformed response, timeout, retry, and duplicate response.
- Fixed clocks.
- Fuzz tests for parsers.

## Avoid

- Blindly regenerating golden files.
- Tests that ignore contractually important errors/codes.
- Tests requiring live credentials by default.

## Almost never do

- Ship legal/eSocial/SST serialization without golden tests.
- Ship concurrency changes without concurrency evidence.

---

## 37. Coverage and Mutation Testing

## Recommendation

Coverage is useful evidence, not a quality score by itself.

## Always do

- Measure coverage when project policy requires it.
- Explain coverage gaps for changed critical code.
- Avoid lowering coverage without reason.
- Ensure failure paths are covered for critical behavior.

## Prefer

- Line and branch coverage for changed business logic.
- Mutation testing for validation, legal, audit, and security rules when available.
- Coverage reports from the final build state.

## Avoid

- Inflating coverage through trivial tests.
- Using coverage to justify weak assertions.
- Ignoring uncovered failure branches.

## Almost never do

- Claim high quality because coverage is high while failure paths are untested.

---

## 38. Fuzzing and Property-Style Tests

## Recommendation

Fuzzing and property-style tests are required when input space is large, adversarial, or parser-heavy.

## Always do

- Consider fuzzing for parsers, decoders, serializers, protocol handlers, XML/JSON processing, compression, and low-level conversion code.
- Preserve discovered crashing inputs as regression tests.
- Run sanitizers with fuzzing when practical.
- Document fuzz commands or blockers.

## Prefer

- libFuzzer, AFL++, honggfuzz, or project-approved harness.
- ASan/UBSan fuzz builds.
- Round-trip, canonicalization, and rejection properties.

## Avoid

- Nondeterministic fuzz targets.
- Catch-all handlers that hide crashes.
- Treating fuzzing as a replacement for semantic tests.

## Almost never do

- Ship new untrusted-input parsers with no malformed-input testing.

---

## 39. Static and Dynamic Analysis

## Recommendation

C++ requires both static and dynamic analysis.

## Always do

- Run configured static analysis.
- Run sanitizer/dynamic-analysis builds when risk requires and platform supports it.
- Treat findings as quality failures unless justified.
- Keep suppressions narrow and documented.
- Run analysis on the final code state.

## Prefer

- `clang-tidy`, `cppcheck`, CodeQL, Semgrep, PVS-Studio, Coverity.
- ASan, UBSan, LSan, TSan, MSan.
- Valgrind/Dr. Memory where sanitizers are unavailable.

## Avoid

- Running analysis on stale builds.
- Suppressing sanitizer errors.
- Treating sanitizer-clean as proof of business correctness.

## Almost never do

- Ship memory/lifetime-sensitive code without sanitizer evidence when supported.
- Ignore UB or data-race findings.

---

## 40. Persistence

## Recommendation

Persistence code must preserve domain meaning, transactions, consistency, migrations, and error semantics.

## Always do

- Keep persistence adapters outside pure domain code.
- Map database records to validated domain/application types.
- Test mapping, query behavior, migrations, transaction boundaries, and error mapping.
- Handle not-found, conflict, timeout, cancellation, and constraint errors deliberately.
- Use parameterized queries.

## Prefer

- RAII wrappers for connections, statements, results, and transactions.
- Integration tests against project-approved test database setup.
- Explicit nullable/default field tests.

## Avoid

- SQL string concatenation with untrusted input.
- Database null/default behavior defining domain invariants.
- Ignoring commit/rollback errors.

## Almost never do

- Ship migration changes without migration evidence.

---

## 41. API and Transport Layers

## Recommendation

API and transport layers are boundaries.

## Always do

- Keep handlers/controllers thin.
- Validate request shape at the boundary.
- Map DTOs to validated domain/application inputs.
- Map outcomes to safe response DTOs.
- Enforce authorization/tenancy where architecture requires it.
- Test success and failure responses.

## Prefer

- Dedicated DTOs.
- Strict decoding where contracts require it.
- Versioned API models.
- Safe error envelopes.
- Boundary mappers with tests.

## Avoid

- Returning raw infrastructure errors.
- Logging raw request bodies containing sensitive data.
- Treating decode success as validation success.

## Almost never do

- Put legal/eSocial/SST decisions in API mappers only.

---

## 42. External Integrations

## Recommendation

External integrations fail in ways local tests often miss.

## Always do

- Keep external clients outside domain logic.
- Use timeouts and cancellation.
- Validate external responses.
- Handle malformed responses, transport errors, provider rejections, retries, and duplicate responses.
- Preserve provider correlation/protocol/receipt identifiers.
- Use fake-server or contract tests.
- Redact sensitive payloads.

## Prefer

- Dedicated adapter targets.
- Explicit DTOs.
- Bounded backoff retries with idempotency.
- Tests for timeout, cancellation, malformed response, retry exhaustion, and duplicate protocol.

## Avoid

- Infinite retries.
- Fire-and-forget legal/business operations.
- Default tests depending on live services.

## Almost never do

- Ship external integration changes without fake-server/contract tests.

---

## 43. eSocial/SST Strict Gate

## Recommendation

eSocial/SST code is legal/regulatory code.

## Always do

- Treat eSocial/SST rules as critical legal rules.
- Keep legal rules out of XML builders and transport adapters.
- Use domain value objects for identifiers, dates, versions, receipts, statuses, certificates, and event states.
- Validate event versions, required fields, lifecycle transitions, legal deadlines, and date periods.
- Generate deterministic XML.
- Test golden XML fixtures.
- Validate schemas where available.
- Test signing/canonicalization when signing is involved.
- Preserve receipts/protocols and audit events.
- Redact sensitive payloads.

## Prefer

- Version-specific event models.
- State-specific types for raw, validated, signed, transmitted, accepted, rejected, cancelled, and corrected states.
- Fake provider responses for acceptance, rejection, malformed response, timeout, and duplicate protocol.

## Avoid

- Generated XML payload structs as domain models.
- Provider rejection as generic exception.
- Fire-and-forget transmission.
- Magic strings for legal codes/statuses.

## Almost never do

- Ship eSocial/SST changes without golden/contract/audit tests.

---

## 44. Cryptography, Certificates, and Signing

## Recommendation

Cryptography and signing are specialized security code.

## Always do

- Use project-approved crypto/signing libraries.
- Keep private keys and secrets out of logs, exceptions, dumps, and audit records.
- Validate certificate chains, validity periods, key usage, and revocation policy where required.
- Use secure randomness.
- Test signing success and failure paths.
- Test invalid, expired, wrong-purpose, and missing certificates.

## Prefer

- Dedicated signer interfaces.
- RAII wrappers for crypto handles.
- Non-production test fixtures.
- Golden canonicalization/signature fixtures where practical.
- Clear separation between unsigned and signed payload types.

## Avoid

- Homegrown cryptography.
- Disabling certificate verification.
- Logging PEM/private key/certificate material.
- Mixing signing and business validation in one function.

## Almost never do

- Ship signing changes without canonicalization/signature tests.

---

## 45. Configuration

## Recommendation

Configuration is input.

## Always do

- Parse configuration at the boundary.
- Validate required configuration.
- Use typed immutable configuration objects.
- Avoid global mutable configuration.
- Redact secrets.
- Document new configuration keys.
- Test valid and invalid configuration.

## Prefer

- Validation at startup/composition.
- Safe defaults only when documented.
- Explicit units in configuration names.
- Centralized environment-variable reads.

## Avoid

- Reading environment variables deep inside domain logic.
- Silent fallback to insecure values.
- Logging full configuration when it contains secrets.

## Almost never do

- Store secrets in source code.
- Use production credentials in tests.

---

## 46. Performance

## Recommendation

Performance work must be measured.

## Always do

- Define the performance goal.
- Measure before and after.
- Use representative inputs.
- Keep correctness tests.
- Avoid UB-prone optimizations.
- Document benchmark commands and results.

## Prefer

- Profiling before optimization.
- Release/RelWithDebInfo builds for performance evidence.
- Allocation measurement where relevant.
- Move semantics when ownership transfer is intended.

## Avoid

- Unmeasured cleverness.
- Unsafe casts for allocation avoidance without benchmarks and sanitizer evidence.
- Benchmarking debug builds as release performance evidence.

## Almost never do

- Sacrifice correctness for speed in legal/security/audit code.

---

## 47. Resource Management

## Recommendation

Resources must be acquired and released safely and deterministically.

## Always do

- Use RAII for files, sockets, handles, locks, memory, transactions, crypto contexts, and foreign resources.
- Avoid manual cleanup paths.
- Handle cleanup errors when they matter.
- Avoid leaks, double cleanup, and use after close/free.
- Test failure paths that require cleanup.

## Prefer

- Move-only resource owners.
- Scope-bound locks and transactions.
- Custom RAII wrappers for C handles.
- Explicit close/flush/commit APIs when destructors cannot report errors.

## Avoid

- Raw `new`/`delete`, manual `malloc`/`free`, and manual lock/unlock pairs outside low-level wrappers.
- Ignoring flush/close/commit errors where they matter.

## Almost never do

- Manage resources manually in business code.
- Rely on destructors for business-critical actions that can fail.

---

## 48. Generated C++ and Generated Artifacts

## Recommendation

Generated C++ is still C++.

## Always do

- Mark generated files clearly.
- Keep generation deterministic.
- Document generator command and version.
- Test generated behavior.
- Review generated public APIs.
- Verify generated includes and source references do not point to local scratch paths.

## Prefer

- Schema-driven generation for external contracts.
- Golden tests for generated serializers/payloads.
- Diff checks after regeneration.
- Narrow warning suppressions for generated code when unavoidable.

## Avoid

- Hand-editing generated files without policy.
- Generated code that changes every run.
- Generated macros in public headers.
- Generated code bypassing architecture.

## Almost never do

- Treat generated legal XML/signing behavior as correct without golden tests.

---

## 49. Cross-Compilation, Platform, Runtime, and Deployment

## Recommendation

C++ behavior is platform-sensitive.

## Always do

- Identify target platforms.
- Build for affected targets where practical.
- Document unverified platforms.
- Isolate platform-specific code.
- Document runtime library requirements.
- Verify packaging/install behavior when changed.

## Prefer

- CI matrix coverage.
- Tests for path, newline, locale, encoding, filesystem, endian, and timezone behavior when relevant.
- Explicit shared-library export/import symbols.

## Avoid

- Assuming x86_64 little-endian behavior.
- Runtime dependencies not packaged with deployables.
- ABI assumptions across compilers.

## Almost never do

- Claim portability after testing one host platform only.

---

## 50. Regression Tests

## Recommendation

Every bug fix needs a regression test unless impossible.

## Always do

- Add a test for every fixed bug when practical.
- Test the observable failure that previously occurred.
- Keep minimized fuzz/crash inputs.
- Document missing regression tests with a concrete reason.

## Prefer

- Small focused regression tests.
- Golden fixtures for payload regressions.
- Sanitizer regression evidence for memory/lifetime bugs.

## Avoid

- Manual verification as the only regression evidence.
- Tests that merely check no exception without validating result.

## Almost never do

- Fix production bugs without regression tests.

---

## 51. LLM-Specific C++ Anti-Patterns

## Recommendation

The implementation LLM must avoid common generated-C++ failure modes.

## Always do

- Prefer simple, idiomatic C++.
- Use RAII.
- Make ownership explicit.
- Use strong domain types.
- Keep functions small.
- Keep templates justified.
- Keep public headers minimal.
- Run build, tests, formatting, and analysis.

## Avoid

- Fake enterprise architecture.
- `Manager`, `Processor`, `Handler`, `Helper`, and `Utils` sprawl.
- Raw `new`/`delete`.
- `shared_ptr` everywhere.
- Catching all exceptions and returning success.
- Silent defaults.
- `reinterpret_cast` shortcuts.
- Public headers full of implementation details.
- Global mutable state.
- Magic strings and numbers.
- TODO stubs.
- Debug prints.
- Build scripts that work only locally.

## Almost never do

- Invent a custom framework.
- Hide business rules in templates/macros.
- Declare completion without PHASE-RESULT.md.

---

## 52. Recommended Tooling Matrix

## Build and test

- CMake, Meson, Bazel, MSBuild, Make, or project build system
- CTest, GoogleTest, Catch2, doctest, Boost.Test, or project test framework
- Ninja or project-approved generator

## Formatting and include hygiene

- clang-format
- include-what-you-use
- project include-order checks

## Compiler warnings and static analysis

- GCC, Clang, MSVC warnings
- clang-tidy
- cppcheck
- CodeQL
- Semgrep
- PVS-Studio
- Coverity
- MSVC `/analyze`

## Dynamic analysis

- AddressSanitizer
- UndefinedBehaviorSanitizer
- LeakSanitizer
- ThreadSanitizer
- MemorySanitizer
- Valgrind
- Dr. Memory
- Application Verifier

## Coverage, fuzzing, dependencies, ABI

- llvm-cov, gcov/gcovr, lcov, OpenCppCoverage
- libFuzzer, AFL++, honggfuzz
- Conan/vcpkg/Bazel/CMake dependency tools
- OSV-Scanner, SBOM/license tools
- libabigail, abi-compliance-checker, abi-dumper

## Recommendation

A missing tool is not a pass. If a tool is expected but unavailable, document blocker, impact, and replacement evidence.

---

## 53. Architecture Test Ideas

## Useful checks

- Domain targets do not link infrastructure targets.
- Domain headers do not include HTTP/database/framework headers.
- Public headers do not include private implementation headers.
- Public headers compile independently.
- Public headers do not leak unnecessary third-party dependency headers.
- Generated DTO headers are not included by domain targets.
- Transport targets depend on application ports, not persistence implementations directly.
- Persistence targets do not define business outcomes.
- XML/signing targets serialize/sign validated data rather than deciding validity.
- Test targets do not force production code to export internals.
- Install/export rules include only intended public headers.
- Build targets have scoped include directories and link dependencies.

## Prefer

- Lightweight scripts or CI checks over repeated manual review.
- Include graph tools where useful.
- Downstream sample-project compile tests for public libraries.

## Avoid

- Architecture rules only in prose.
- Broad global include directories.
- Tests requiring internals to be exported.

---

## 54. Definition of Done

## Required

A phase is done only when:

- Planned C++ implementation exists.
- Relevant automated tests exist.
- Build configuration is updated intentionally.
- Code configures, compiles, and links.
- Relevant test targets pass.
- Formatting check passes.
- Compiler warnings/static analysis pass or exceptions are documented.
- Sanitizers/dynamic analysis are run when risk requires them or blockers are documented.
- Dependency changes are documented.
- Public API/ABI changes are documented.
- Architecture boundaries are preserved or deviations documented.
- Security/audit/legal implications are reviewed.
- PHASE-RESULT.md exists.
- PHASE-RESULT.md contains command evidence, blockers, residual risk, and quality score.
- Final response follows the exact required text.

## Not done

A phase is not done if code was written but not built, tests are missing, failure paths are untested, ownership/lifetimes are ambiguous, tooling was skipped without documentation, PHASE-RESULT.md is missing, or the final response includes extra summary text.

---

## 55. PHASE-RESULT.md Required Template

PHASE-RESULT.md must be created before the final response.

```markdown
# PHASE-RESULT.md

## Phase summary

- Planned change:
- Actual change:
- Files changed:
- Build system changes:
- Dependency changes:
- Public API changes:
- ABI changes:
- Configuration changes:
- Security/audit/legal impact:

## Toolchain evidence

- Compiler:
- Compiler version output:
- C++ standard:
- Standard library:
- Build system:
- Build generator:
- Build type(s):
- Target OS/architecture:
- Exception policy:
- RTTI policy:
- Sanitizers used:
- Package manager/toolchain file/preset:

## Commands run

- `command here`

## Commands passed

- `command here`

## Commands failed

- `command here`
  - Reason:
  - Impact:
  - Required fix:

## Commands not run

- `command here`
  - Reason:
  - Impact:
  - Replacement evidence:

## Test evidence

- Unit tests:
- Integration tests:
- Contract tests:
- Golden tests:
- Regression tests:
- Fuzz/property tests:
- Coverage:
- Sanitizer/dynamic analysis:
- Manual tests:

## Static analysis and formatting evidence

- Formatting:
- Compiler warnings:
- clang-tidy:
- cppcheck:
- Other analyzers:
- Suppressions added:

## Architecture evidence

- Boundary review:
- Target dependency review:
- Public header review:
- Include hygiene:
- Generated code review:

## Ownership/lifetime/resource review

- Raw owning pointers:
- Smart pointer ownership:
- Non-owning views/references:
- RAII resources:
- Manual cleanup:
- Thread/task lifetimes:

## Error-handling review

- Error style:
- Exception behavior:
- Failure paths tested:
- Error mapping:
- Sensitive error leakage review:

## Security, audit, and compliance review

- Input validation:
- Secret handling:
- Sensitive logging/redaction:
- Audit records:
- Crypto/signing:
- eSocial/SST:
- Dependency vulnerabilities:
- License review:

## Residual risk

Explain remaining risk in plain language.

## Quality score

Score: __ / 100

Reasoning:

- Build/reproducibility:
- Tests:
- Failure paths:
- Architecture:
- Ownership/lifetimes:
- Static/dynamic analysis:
- Security/audit:
- Maintainability:
- Documentation:
```

---

## 56. Quality Score Model

## Suggested scoring

- Build and reproducibility: 15
- Formatting and warning cleanliness: 10
- Static/dynamic analysis: 10
- Tests and meaningful assertions: 20
- Failure-path coverage: 10
- Architecture and boundaries: 10
- Ownership, lifetimes, RAII, and UB avoidance: 10
- Security, audit, legal, and dependency hygiene: 10
- Documentation and maintainability: 5

## Score caps

- No successful build: maximum 35.
- No automated tests for changed business logic: maximum 50.
- No PHASE-RESULT.md: maximum 40.
- Formatting not checked: maximum 75.
- Compiler warnings/static analysis not run or not documented: maximum 80.
- Sanitizer-required memory/concurrency code without sanitizer evidence or blocker: maximum 75.
- Public API/ABI changed without review: maximum 70.
- Error/failure paths untested for critical behavior: maximum 70.
- Legal/eSocial/SST changes without golden/contract/audit tests: maximum 60.
- Security/crypto/signing changes without negative tests: maximum 65.
- Dependency changes without documentation: maximum 80.
- Architecture boundary violation: maximum 65 unless explicitly approved.
- Raw owning pointers/manual cleanup in business code without justification: maximum 70.
- Known UB or sanitizer failure: maximum 45.
- Final response rule violated: gate failure regardless of score.

Evidence wins. Do not inflate the score because the implementation looks right.

---

## 57. Caveman Quality Review

Ask bluntly:

- Does it build from a clean checkout?
- Can I run the same commands?
- Are tests meaningful?
- Are failure paths tested?
- Are ownership and lifetimes obvious?
- Are there raw owning pointers?
- Are there dangling views/references/captures?
- Are exceptions/errors handled consistently?
- Are resources managed by RAII?
- Are public headers minimal?
- Are dependencies necessary?
- Are warnings clean?
- Did static analysis run?
- Did sanitizer/dynamic analysis run where risk required it?
- Is business logic in the right layer?
- Would a maintainer know where to change this later?
- Is cleverness justified?
- Are secrets protected?
- Are audit/legal rules explicit?
- Is PHASE-RESULT.md honest?

## Red flags

- “It compiles” is the main evidence.
- Tests only cover the happy path.
- `shared_ptr` everywhere.
- Raw `new`/`delete` in business code.
- `reinterpret_cast` without safety proof.
- Stored `std::string_view` without lifetime guarantee.
- Detached threads.
- Sleeps in concurrency tests.
- Global mutable state.
- Hand-built XML.
- Error strings parsed by callers.
- Public headers include everything.
- Build works only in one local IDE.
- Generated code changed nondeterministically.
- PHASE-RESULT.md hides skipped commands.

---

## 58. Final Checklist

- [ ] Planned implementation exists.
- [ ] PHASE-RESULT.md exists.
- [ ] Commands were run from the correct repository directory.
- [ ] Command evidence is from the final code state.
- [ ] Compiler version and C++ standard are documented.
- [ ] Build system configuration is documented.
- [ ] Code configures, compiles, and links.
- [ ] Relevant tests pass.
- [ ] Failure-path tests exist.
- [ ] Formatting was checked.
- [ ] Compiler warnings were checked.
- [ ] Static analysis was run or blocker documented.
- [ ] Sanitizers/dynamic analysis were run where risk required or blocker documented.
- [ ] Dependency changes are intentional and documented.
- [ ] Public API changes are documented.
- [ ] ABI changes are reviewed where relevant.
- [ ] Architecture boundaries are preserved.
- [ ] Public headers are intentional and self-contained where required.
- [ ] Include/link dependencies are scoped correctly.
- [ ] Ownership and lifetime behavior is explicit.
- [ ] Raw owning pointers/manual cleanup are absent or justified.
- [ ] Non-owning views/references/captures were reviewed.
- [ ] RAII is used for resources.
- [ ] Error/exception policy is followed.
- [ ] Null/optional/default semantics are tested.
- [ ] Undefined-behavior risk was reviewed.
- [ ] Concurrency/cancellation/shutdown behavior is tested when applicable.
- [ ] Thread/task failures are observable.
- [ ] Serialization/deserialization contracts are tested when changed.
- [ ] XML/legal payloads have golden/schema/canonicalization evidence when applicable.
- [ ] Persistence changes have migration/query/mapping tests.
- [ ] External integrations have timeout/error/malformed-response tests.
- [ ] Audit-critical flows record success and failure events.
- [ ] Sensitive data redaction was reviewed.
- [ ] Crypto/signing/certificate behavior is tested when applicable.
- [ ] Generated code is deterministic and reviewed.
- [ ] Platform/feature configurations affected by the phase were tested or documented.
- [ ] PHASE-RESULT.md contains exact command evidence.
- [ ] Residual risk is explained plainly.
- [ ] The quality score is evidence-based.
- [ ] The final response rule is followed exactly.
