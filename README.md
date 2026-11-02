# OuluBSD/IDE

Next-generation modular Integrated Development Environment (DevEnv) for Ultimate++ (U++).

## Dependency Hierarchy

Part of the **OuluBSD U++ Modularization Architecture**:
```
UppBootstrap  <--  AdvancedBuilder (UppBuilder)  <--  BazaarCore  <--  IDE (DevEnv)
```
1. **`UppBootstrap`**: Zero-dependency toolchain bootstrap (`umk` compiler & build methods).
2. **`AdvancedBuilder` (`UppBuilder`)**: Meta-builder with 3-pillar bridge, multi-assembly chaining, deterministic config keys.
3. **`BazaarCore`**: Foundation assembly (`patched-uppsrc`), single amalgamated `Core/Ext`, and Tier 3 packages (`Docking`, `Terminal`, `Manager`...).
4. **`IDE` (`DevEnv`)**: Modular developer environment application built on top of `BazaarCore` via `UppBuilder`.

## Overview

`OuluBSD/IDE` separates the traditional monolithic U++ IDE (`TheIDE`) into a clean, decoupled architecture:
- **`uppsrc/DevEnv/Core`**: Headless build engine, package parser, compiler driver, and CLI UppHub client (Zero GUI dependencies).
- **`uppsrc/DevEnv/Draw`**: Syntax highlighting, themes, dark/light palette blending, high-DPI scaling.
- **`uppsrc/DevEnv/Ctrl`**: Reusable code editor canvas, gutter margin, project tree, and terminal console.
- **`uppsrc/DevEnv/Ctrl/Core`**: Workspace presenters, document manager, build state dispatcher.
- **`uppsrc/DevEnv/Ctrl/Lib`**: UppHub visual package explorer, assembly manager, toolchain configuration.
- **`uppsrc/DevEnv/App`**: Desktop application shell with docking layout and DropTerm console integration.
- **`uppsrc/MCP`**: Model Context Protocol server/client for AI agent integration.

## Building

Build with `uppbuilder`:
```bash
uppbuilder DevEnv/App
```

Run the built application:
```bash
./bin/App
```

## License
BSD 2-Clause License. See [LICENSE.md](LICENSE.md).
