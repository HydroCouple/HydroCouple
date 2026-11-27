# Contributing to HydroCouple

Thank you for your interest in contributing to **HydroCouple**! This document outlines the governance model, contribution workflow, and standards that keep the project healthy, reproducible, and stable for the components that depend on it. Please read it carefully before opening an issue, submitting a pull request, or proposing a change to the interface standard.

HydroCouple is an **interface standard**, not an application. Its headers are compiled into every component and composition that implements it, so the bar for changing a published interface is deliberately higher than for ordinary application code. Section 8 and Section 15 describe that bar.

---

## Table of Contents

1. [Community Governance](#1-community-governance)
2. [Repository & Technical Management](#2-repository--technical-management)
3. [Succession & Delegation](#3-succession--delegation)
4. [Licensing, Intellectual Property & CLA](#4-licensing-intellectual-property--cla)
5. [Author Acknowledgment](#5-author-acknowledgment)
6. [How to Cite HydroCouple](#6-how-to-cite-hydrocouple)
7. [Versioning Strategy](#7-versioning-strategy)
8. [Interface Stability & Backward Compatibility](#8-interface-stability--backward-compatibility)
9. [Branching Model](#9-branching-model)
10. [Bug Fixes & Small Improvements](#10-bug-fixes--small-improvements)
11. [Pull Request Process](#11-pull-request-process)
12. [Continuous Integration & Automated Testing](#12-continuous-integration--automated-testing)
13. [Issue Reporting](#13-issue-reporting)
14. [Review Timeline Expectations](#14-review-timeline-expectations)
15. [Proposing Interface Changes](#15-proposing-interface-changes)
16. [Conflict of Interest in Reviews](#16-conflict-of-interest-in-reviews)
17. [Documentation Standards](#17-documentation-standards)
18. [Dependency Evaluation Policy](#18-dependency-evaluation-policy)
19. [Security Vulnerability Reporting](#19-security-vulnerability-reporting)
20. [Code of Conduct](#20-code-of-conduct)

---

## 1. Community Governance

HydroCouple is a community-driven open source project. All contributors are welcome regardless of affiliation, background, or experience level. The community operates on a model of open discussion, merit-based evaluation, and transparent decision-making. Key decisions affecting the project direction are made openly on GitHub, and all community members are encouraged to participate.

---

## 2. Repository & Technical Management

**Repository and technical management is the responsibility of the Technical Manager (currently [@cbuahin](https://github.com/cbuahin)).**

The Technical Manager is responsible for:

- Maintaining the integrity and health of the `master`, `dev`, and experimental branches.
- Setting and enforcing coding standards, testing requirements, and documentation guidelines.
- Triaging issues and pull requests in a timely manner.
- Making final decisions on merges, releases, and branch management.
- Adjudicating proposed changes to the published interface standard (Section 15).
- Coordinating with the community on roadmap items and governance questions.

Community members who wish to take on elevated responsibilities (e.g., becoming a recurring reviewer or branch maintainer) may express interest by opening a discussion on GitHub.

---

## 3. Succession & Delegation

The Technical Manager role is a single point of authority, which must be protected against absence or unavailability.

- In the event the Technical Manager is unavailable for an extended period (e.g., travel, illness, or leave), they may designate a **Temporary Delegate** with equivalent merge and release authority. The delegation will be announced publicly in GitHub Discussions.
- If the Technical Manager steps down permanently, the outgoing manager will nominate a successor from the active contributor community. The nomination is subject to a community feedback period of at least two weeks before taking effect.
- In the absence of a named delegate and in urgent situations (e.g., a critical security fix), any two senior community reviewers may jointly approve and merge a patch to `master`, with a written rationale posted to GitHub Discussions immediately afterward.

---

## 4. Licensing, Intellectual Property & CLA

HydroCouple — including the C++ interface headers, the Python bindings, the tests, and the documentation — is released under the license recorded in [License](./License).

All contributors must sign the project **Contributor License Agreement (CLA)** before their pull request can be merged. The CLA is detailed in [CLA.md](./CLA.md). Key points:

- **You retain your copyright.** The CLA grants a license; it does not transfer ownership.
- **Broad license grant.** You grant the Technical Manager a perpetual, irrevocable license to use, distribute, and **relicense** your contribution (e.g., for future dual open-source/commercial licensing).
- **Patent grant.** You grant a royalty-free patent license covering patents necessarily infringed by your contribution.
- **Representation of authority.** You confirm you have the right to submit the contribution and that it contains no unlicensed third-party material.
- **Corporate contributors** must additionally submit a Corporate CLA (CCLA) — see [CLA.md §6](./CLA.md#6-corporate-contributors).

### Signing the CLA

The project uses [CLA Assistant](https://cla-assistant.io). When you open your first pull request, a bot will post a comment with a signing link. You may also sign manually by posting the following comment on your PR:

> I have read the CLA Document and I hereby sign the CLA.

Once signed, the CLA covers all future contributions. You do not need to sign again.

If your contribution includes third-party code or data, you are responsible for ensuring that the third-party license is compatible with the license recorded in [License](./License) and that proper attribution is included.

---

## 5. Author Acknowledgment

All contributors whose work is incorporated into HydroCouple will be recognized in [AUTHORS.md](./AUTHORS.md).

- **Name, affiliation (if any), and scope of contribution** will be documented for each contributor.
- The Technical Manager maintains `AUTHORS.md` and will update it at each release.
- If you believe your contribution has been omitted or mis-described, please open an issue or contact the Technical Manager directly.

Contributors are encouraged to add themselves to `AUTHORS.md` as part of their pull request, subject to review and formatting consistency.

---

## 6. How to Cite HydroCouple

If you use HydroCouple in published research, teaching materials, or engineering reports, please cite the foundational publication:

```
Buahin, C.A., Horsburgh, J.S., 2018. Advancing the Open Modeling Interface (OpenMI)
for integrated water resources modeling. Environmental Modelling & Software 108, 133-153.
https://doi.org/10.1016/j.envsoft.2018.07.015
```

A machine-readable citation record is maintained in [CITATION.cff](./CITATION.cff), and the BibTeX entry is in the [README](./README.md#citation). When citing a specific version of the software, reference the corresponding release tag so that your citation is reproducible.

---

## 7. Versioning Strategy

HydroCouple follows **Semantic Versioning (SemVer)** as defined at [semver.org](https://semver.org), with formal pre-release trajectories during testing periods.

### Version Format

```
MAJOR.MINOR.PATCH[-PRERELEASE]
```

| Component | When to increment |
|-----------|-------------------|
| `MAJOR`   | Incompatible changes to the published interface standard |
| `MINOR`   | New interfaces or interface members added in a backward-compatible manner |
| `PATCH`   | Backward-compatible bug fixes, documentation updates, or build-system changes |

### Pre-Release Trajectories

During active release testing periods, the following pre-release labels are used in sequence:

| Stage              | Label example     | Purpose                                                                      |
|--------------------|-------------------|------------------------------------------------------------------------------|
| Alpha              | `2.0.0-alpha.1`   | Early feature-complete builds, internal or limited testing; may be unstable  |
| Beta               | `2.0.0-beta.1`    | Feature-frozen; broader community testing; known issues may exist            |
| Release Candidate  | `2.0.0-rc.1`      | Final candidate for release; only critical bug fixes accepted                |
| Stable             | `2.0.0`           | Fully validated, production-ready release                                    |

Pre-release builds are tagged in Git and published to the appropriate distribution channel with clear labels to prevent unintended use in production workflows.

---

## 8. Interface Stability & Backward Compatibility

Every component and composition that implements this standard compiles against these headers. A change that looks small here can break every downstream implementation at once, so HydroCouple is committed to giving implementers adequate notice before breaking changes are introduced.

### What counts as a breaking change

Within a published `MAJOR` version, the following are breaking and are not accepted outside a `MAJOR` bump:

- Removing or renaming an interface, an interface member, or an enumerator.
- Changing a signature: parameter types or order, return type, constness, or exception specification.
- Adding a pure virtual member to an existing interface, which invalidates every existing implementation.
- Reordering or renumbering existing enumerators.
- Tightening a documented pre-condition or weakening a documented post-condition.

Adding a **new** interface, or a new enumerator appended to the end of an enumeration, is backward-compatible and may land in a `MINOR` release.

### Deprecation process

1. **Announcement** — An interface or member to be removed is first marked as deprecated in the release notes and in code (via `[[deprecated]]` and a Doxygen `@deprecated` note). The announcement states the planned removal version and the replacement.
2. **Minimum notice period** — Deprecated items are not removed for at least **one full minor version cycle** following the announcement. For widely-implemented interfaces, a minimum of **one major version cycle** is preferred.
3. **Removal** — Deprecated items are removed in a `MAJOR` version bump, never in a `MINOR` or `PATCH` release.

### Compatibility commitment

- `PATCH` releases are guaranteed to be fully backward-compatible.
- `MINOR` releases are backward-compatible for all stable published interfaces.
- `MAJOR` releases may introduce breaking changes, which will be documented in a migration guide published alongside the release and summarized in [CHANGELOG.md](./CHANGELOG.md).

Interfaces explicitly marked experimental carry no backward compatibility guarantee and may change or be removed in any release.

---

## 9. Branching Model

| Branch                    | Purpose                                                                                     |
|---------------------------|---------------------------------------------------------------------------------------------|
| `master`                  | Stable, production-ready code. Only receives merges from release candidates.                |
| `dev`                     | Integration branch for ongoing development. All feature and bug-fix branches target here.   |
| `experimental/<name>`     | Sandboxed branches for exploratory or breaking interface work. See Section 15.              |
| `bugfix/<issue-id>-desc`  | Short-lived branches forked from `dev` to address specific bug reports.                     |
| `feature/<name>`          | Short-lived branches for minor additions, forked from `dev`.                                |
| `release/<version>`       | Release preparation branches (alpha → beta → rc) cut from `dev`.                            |

---

## 10. Bug Fixes & Small Improvements

For small, well-scoped bug fixes and minor improvements, follow this workflow:

1. **Open or reference an issue.** Verify the problem is reproducible and link the relevant issue number in all subsequent commits and pull requests.
2. **Fork the `dev` branch.** Name your branch `bugfix/<issue-id>-short-description` (e.g., `bugfix/42-argument-clone-lifetime`).
3. **Write a failing test first.** The test must demonstrate the defect before the fix is applied.
4. **Implement the minimal fix.** Touch only the code necessary to resolve the issue. Do not refactor adjacent code.
5. **Confirm the change is not a breaking interface change.** If it is, it belongs in Section 15, not here.
6. **Ensure the full test suite passes** on all three supported platforms, or document any failure and why it is acceptable in the pull request description.
7. **Submit a pull request** against `dev` following the process described in Section 11.

---

## 11. Pull Request Process

All contributions enter the codebase through a pull request (PR). PRs must satisfy the following requirements before they can be merged:

### Required Approvals

Every PR requires **all three** of the following approvals:

| Reviewer               | Role                                                                                                                               |
|------------------------|------------------------------------------------------------------------------------------------------------------------------------|
| **Technical Manager**  | [@cbuahin](https://github.com/cbuahin) (current Technical Manager) — final authority on interface design, code quality, and correctness |
| **AI Copilot Review**  | Automated AI-assisted review for style, logic, and common error patterns                                                            |
| **Community Reviewer** | At least one community contributor **other than the PR author** must approve                                                       |

Self-approvals are not permitted. The PR author may not count toward the community reviewer requirement.

### PR Checklist

Before requesting review, confirm that your PR:

- [ ] Targets the correct branch (`dev` for bug fixes and features; see Section 15 for interface changes)
- [ ] CLA signed — first-time contributors must sign the [CLA](./CLA.md) before the PR can be merged
- [ ] Includes a clear description of what was changed and why
- [ ] References any related issues (e.g., `Closes #42`)
- [ ] States explicitly whether it changes the published interface, and if so, links the Section 15 discussion
- [ ] Includes new or updated tests covering the change
- [ ] Builds and passes `ctest` on Linux, macOS, and Windows
- [ ] CI pipeline passes (see Section 12)
- [ ] Doxygen comments updated for every touched public element, and the Python bindings and their Sphinx docs updated if the C++ interface changed
- [ ] `AUTHORS.md` entry has been added or updated if this is your first contribution

### Merge Policy

- Merges into `master` are performed exclusively by the Technical Manager at release time.
- Squash merging is preferred for `bugfix` and `feature` branches to keep the `dev` history clean.
- Merge commits are used when integrating `dev` into a `release` branch to preserve the full history.

---

## 12. Continuous Integration & Automated Testing

All pull requests are automatically tested via the project's CI pipeline (GitHub Actions) before human review begins. A failing CI pipeline will block merge regardless of approvals.

Three workflows run on every push and pull request to `master` and `dev`:

| Workflow file | Name | What it does | Platforms |
|---|---|---|---|
| `build_and_unit_test.yml` | **Build and Unit Test** | Configures with `HYDROCOUPLE_BUILD_TESTS=ON`, builds, and runs `ctest` | ubuntu-latest (x64-linux), macos-latest (arm64-osx), windows-latest (x64-windows) |
| `build_and_package.yml` | **Build and Package** | Configures with `HYDROCOUPLE_BUILD_TESTS=OFF`, runs CPack to produce `.tar.gz`/`.zip`/`.nupkg`, pushes NuGet packages to GitHub Packages | ubuntu-latest, macos-latest, windows-latest |
| `build_docs.yml` | **Documentation** | Builds the Doxygen C++ docs and the Sphinx Python docs and co-deploys them to GitHub Pages | ubuntu-latest |

All workflows use vcpkg at ref `2025.02.14` and cache packages via the GitHub NuGet feed. The Sphinx build runs with `-W`, so a documentation warning fails the build.

Contributors are expected to build and run the test suite locally before opening a PR to minimize unnecessary CI failures. Build instructions are in the [README](./README.md#building).

---

## 13. Issue Reporting

### Bug Reports

A complete bug report must include:

- A clear, descriptive title.
- The version of HydroCouple affected (e.g., `2.0.0-alpha.1`).
- The operating system, compiler, and C++ standard version.
- A minimal reproducible example — the smallest component or code snippet that demonstrates the problem.
- Observed vs. expected behavior, including any relevant output or error messages.

Incomplete bug reports may be closed or deprioritized pending clarification.

### Feature Requests & Ideas

Open an issue for small, well-scoped additions. For proposals that would change the published interface standard, use the GitHub Discussions **Ideas** section instead (see Section 15).

### Issue Triage

The Technical Manager or a designated reviewer will triage new issues within **two weeks** of submission, assigning labels (e.g., `bug`, `enhancement`, `needs-clarification`, `wontfix`) and priority. Contributors are encouraged to comment on existing issues before opening duplicates.

---

## 14. Review Timeline Expectations

HydroCouple is maintained by volunteers. The following timelines are targets, not guarantees, but the Technical Manager is committed to keeping them:

| Action                                    | Target Timeline          |
|-------------------------------------------|--------------------------|
| Initial issue triage                      | Within 2 weeks           |
| First review response on a PR             | Within 3 weeks           |
| Follow-up review after requested changes  | Within 2 weeks           |
| Interface change discussion response      | Within 4 weeks           |

If you have not received a response within the stated window, you are welcome to post a polite follow-up comment on the issue or PR. PRs that become stale (no activity for 60 days) may be closed with a note that they can be reopened when the contributor is ready to continue.

---

## 15. Proposing Interface Changes

Any change to the published interface standard — a new interface, a new member on an existing interface, a signature change, or a removal — follows this process rather than the ordinary bug-fix workflow.

### Step 1 — Open a Community Discussion

Open a thread in the GitHub Discussions **Ideas** section describing:

- The modeling capability that is missing or the defect in the current design.
- The proposed interface, in the form of the actual header declarations.
- Which existing implementations would be affected, and how.
- Whether the change is backward-compatible under Section 8, and if not, why a breaking change is warranted.
- Prior art: how OpenMI, or another component-based standard, addresses the same problem.

### Step 2 — Community Feedback

Allow at least **four weeks** for feedback from implementers before proceeding. Interface changes are difficult to reverse once components depend on them, so the discussion period is deliberately longer than for ordinary features.

### Step 3 — Experimental Branch Implementation

Implement the proposal on an `experimental/<name>` branch, including:

- The interface headers with complete Doxygen documentation.
- The corresponding Python bindings.
- Tests demonstrating the new interface in use.
- At least one worked example of a component implementing the proposed interface, so that the design is validated against a real implementation rather than in the abstract.

### Step 4 — Peer Review

Interface changes receive review from the Technical Manager plus at least one implementer of an existing component, who evaluates the migration cost from the downstream side.

### Step 5 — Documentation & Migration Notes

Before merge, the proposal must include:

- Updated Doxygen and Sphinx documentation.
- A [CHANGELOG.md](./CHANGELOG.md) entry.
- For a breaking change, migration notes describing what implementers must do to adapt.

### Step 6 — Merge & Release Targeting

The Technical Manager determines which release the change targets, per the Section 7 versioning rules. Breaking changes are held for the next `MAJOR` release.

---

## 16. Conflict of Interest in Reviews

Scientific software reviews can intersect with contributors' professional interests. To protect the integrity of the review process:

- A reviewer who has a competing or closely related implementation, active publication, or financial interest in the outcome of a review **must disclose this conflict** before participating in the review.
- A conflicted reviewer may participate in discussion but **may not serve as one of the three required approvers** for that PR.
- The Technical Manager makes the final determination on whether a disclosed conflict disqualifies a reviewer on a case-by-case basis.
- Undisclosed conflicts of interest, if discovered after a merge, may trigger a re-review of the affected contribution.

---

## 17. Documentation Standards

HydroCouple maintains two co-deployed documentation sites: **Doxygen** for the C++ interface standard and **Sphinx** for the Python bindings. Because the headers *are* the standard, their documentation is normative, not incidental — an undocumented interface member is an incomplete contribution.

All public-facing code elements (interfaces, classes, structs, enumerations, and their members) must include Doxygen-compatible documentation comments.

Minimum documentation per element:

- `@brief` — one-line summary.
- `@param` — description of each parameter including units where applicable.
- `@return` — description of the return value.
- `@note` / `@warning` — any important behavioral caveats.
- Ownership and lifetime semantics for any pointer or reference crossing the interface boundary.
- Pre-conditions and post-conditions an implementer must honor.
- `@ref` or `@cite` — citation of the relevant literature where the interface follows a published formulation.

Documentation is reviewed as part of the pull request process. PRs with undocumented public interfaces will not be approved.

---

## 18. Dependency Evaluation Policy

Adding a third-party dependency increases the maintenance burden and risk surface of the project. This weighs especially heavily on an interface standard: a dependency added here is inherited by every component that implements the standard.

All proposed new dependencies must be evaluated against the following criteria before inclusion:

| Criterion                | Requirement                                                                                      |
|--------------------------|--------------------------------------------------------------------------------------------------|
| **License compatibility**| The dependency's license must be compatible with the license recorded in [License](./License), and must not impose obligations on downstream implementers that the Project's own license does not. |
| **Maintenance status**   | The dependency must be actively maintained with a responsive upstream community.                 |
| **Stability**            | The dependency must have a stable, versioned API. Unpinned or volatile dependencies are not acceptable. |
| **Interface exposure**   | A dependency whose types appear in the published interface headers is inherited by every implementer and requires an exceptionally strong justification. |
| **Platform support**     | The dependency must support all officially targeted platforms (Linux, macOS, Windows).           |

Proposals to add a new dependency should be raised in a GitHub Discussion or issue before implementation. Optional or test-only dependencies that are not required for the core build may be included under a feature flag, with lower scrutiny, provided they meet the license requirement.

---

## 19. Security Vulnerability Reporting

**Do not report security vulnerabilities through public GitHub issues.**

If you discover a potential security vulnerability in HydroCouple, please report it privately so that a fix can be prepared before the issue is publicly disclosed:

- **GitHub Private Advisory:** Use the [Security Advisories](../../security/advisories) tab to submit a draft advisory directly on GitHub. Tag [@cbuahin](https://github.com/cbuahin) (current Technical Manager) in the advisory.

Please include in your report:

- A description of the vulnerability and its potential impact.
- Steps to reproduce, including any relevant code.
- The version(s) of HydroCouple affected.
- Any known mitigations or workarounds.

The Technical Manager will acknowledge receipt within **five business days** and work with you on a coordinated disclosure timeline. Credit for responsibly disclosed vulnerabilities will be given in the release notes and security advisory, unless the reporter prefers to remain anonymous.

---

## 20. Code of Conduct

HydroCouple is committed to providing a welcoming, respectful, and inclusive environment for all contributors. All participants are expected to:

- Engage constructively and professionally in all project spaces (issues, pull requests, discussions).
- Respect differing viewpoints and scientific perspectives.
- Accept constructive criticism of their contributions in good faith.
- Prioritize the long-term health of the project over individual preferences.

Harassment, personal attacks, or exclusionary behavior of any kind will not be tolerated. Violations may be reported to the Technical Manager at [@cbuahin](https://github.com/cbuahin). Reported incidents will be reviewed and addressed promptly and confidentially.

---

*This document is maintained by the Technical Manager of HydroCouple (currently [@cbuahin](https://github.com/cbuahin)). Last updated: August 2026.*
