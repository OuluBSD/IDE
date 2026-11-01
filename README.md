# OuluBSD/IDE

Next-generation modular Integrated Development Environment (DevEnv) for Ultimate++ (U++).

## Overview

`OuluBSD/IDE` separates the traditional monolithic U++ IDE (`TheIDE`) into a clean, decoupled architecture:
- **`uppsrc/DevEnv/Core`**: Headless build engine, package parser, compiler driver, and CLI UppHub client (Zero GUI dependencies).
- **`DevEnv/Draw`**: Syntax highlighting, themes, dark/light palette blending, high-DPI scaling.
- **`DevEnv/Ctrl`**: Reusable code editor canvas, gutter margin, project tree, and terminal console.
- **`DevEnv/Ctrl/Core`**: Workspace presenters, document manager, build state dispatcher.
- **`DevEnv/Ctrl/Lib`**: UppHub visual package explorer, assembly manager, toolchain configuration.
- **`DevEnv/App`**: Desktop application shell with docking layout and DropTerm console integration.
- **`uppsrc/ide`**: Full backward-compatible U++ IDE application with subpackages and AI MCP tool integrations.

## Upstream Tracking

This repository tracks upstream `ultimatepp` via:
- Baseline divergence point: `upstream-baseline-2026-01-04` (`dc3c90fe52faad0505bf2f832209b9e207ed6d89`)
- Upstream tracking branch: `upstream-master`
- Detailed reconciliation protocol: see [UPSTREAM_BASELINE.md](UPSTREAM_BASELINE.md).

## Building

Build with `uppbuilder`:
```bash
uppbuilder uppsrc/ide -m CLANG
# or the modular DevEnv shell:
uppbuilder uppsrc/DevEnv/App -m CLANG
```

## License
BSD 2-Clause License. See [LICENSE.md](LICENSE.md).
