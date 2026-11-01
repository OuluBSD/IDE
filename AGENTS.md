# AGENTS - OuluBSD/IDE Guide

**Scope**: Applies to the root of `OuluBSD/IDE` and all packages within.

## Repository Overview
`OuluBSD/IDE` is the decoupled IDE repository for the U++ ecosystem, providing:
1. `uppsrc/DevEnv/*`: The modular next-generation developer environment.
2. `uppsrc/ide/*`: The production-grade feature-complete U++ IDE with AI integration, MCP bridges, and subpackages (`MainCtrl`, `Core`, `Builders`, `Browser`, etc.).
3. Upstream tracking of `https://github.com/ultimatepp/ultimatepp.git`.

## Upstream Divergence Baseline
- **Baseline Commit**: `dc3c90fe52faad0505bf2f832209b9e207ed6d89` (tag: `upstream-baseline-2026-01-04`, Tree SHA: `c111f1978447d46ccef796e791bd7d78c684c3f3`).
- **Upstream Tracking Branch**: `upstream-master`.
- Always consult `UPSTREAM_BASELINE.md` before attempting to port or reconcile changes from upstream `ultimatepp`.

## Key Rules & Conventions
- Maintain the headless/GUI split per `headless-gui-dual-purpose.md`: headless models belong in `DevEnv/Core`, never in GUI files.
- Build using `uppbuilder` (`bin/uppbuilder`).
- Code style: follow `/CODESTYLE.md` and `ai/rules/naming-conventions.md`.
