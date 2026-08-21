# Contribution Guide

The Contribution Guide for all ARENA projects can be found [here](https://docs.arenaxr.org/content/contributing.html).

The `apriltag-js-standalone` uses [Release Please](https://github.com/googleapis/release-please) to automate CHANGELOG generation and semantic versioning. Your PR titles *must* follow Conventional Commit standards (e.g., `feat:`, `fix:`, `chore:`).

> [!CAUTION]
> **Never use `BREAKING CHANGE` in commit/PR bodies or the `!` suffix on commit/PR types (e.g., `feat!:`, `fix!:`).** These tokens cause release-please to automatically bump the major version. Major version increments are reserved for the maintainer's explicit decision — contributors and agents do not decide what constitutes a breaking change for semver purposes.

> [!IMPORTANT]
> **Issue and PR References in Commit & PR Messages:**
> Only use `#NN` notation in commit messages, PR titles, and PR descriptions if they correspond to actual GitHub issues or pull requests. Do **not** use `#NN` notation for internal enumerations of planning docs or triage items (e.g., use `Task NN` or plain text instead), as this creates erroneous links and may result in unintended automatic actions.

## CI & Dependency Management Conventions
- **GitHub Actions Tag SHA Pinning**: All GitHub Action references in `.github/workflows/` MUST be pinned to the exact commit SHA of the official release tag (e.g., `uses: actions/checkout@0717577d45739eb3c851188b29f50ed6c0b2194e # v2.8.0`).
- **Inline Version Comments**: The inline comment next to the SHA MUST specify the exact tag version used. This enables Dependabot to recognize the release version, generate human-readable SemVer PR titles (`from X.Y.Z to A.B.C`), and automatically update version comments during upgrades.
- **Dependabot Configuration**: Dependabot version updates are enabled via `.github/dependabot.yml` for the `github-actions` ecosystem.
