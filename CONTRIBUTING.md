# Contribution Guide

The Contribution Guide for all ARENA projects can be found [here](https://docs.arenaxr.org/content/contributing.html).

## CI & Dependency Management Conventions
- **GitHub Actions Tag SHA Pinning**: All GitHub Action references in `.github/workflows/` MUST be pinned to the exact commit SHA of the official release tag (e.g., `uses: actions/checkout@0717577d45739eb3c851188b29f50ed6c0b2194e # v2.8.0`).
- **Inline Version Comments**: The inline comment next to the SHA MUST specify the exact tag version used. This enables Dependabot to recognize the release version and generate human-readable SemVer PR titles.
- **Dependabot Configuration**: Dependabot version updates are enabled via `.github/dependabot.yml` for the `github-actions` ecosystem.
