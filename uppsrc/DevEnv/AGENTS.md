# DevEnv - Modular Development Environment Architecture

**Scope**: Applies to `uppsrc/DevEnv/*` and child packages.

## Architectural Purpose
`DevEnv` is the next-generation modular IDE suite decoupling monolithic `TheIDE` into clean, testable layers following rule `headless-gui-dual-purpose.md`:
1. **`DevEnv/Core`**: Headless build engine, package parser, toolchain discovery (`.bm`), dependency solver, and CLI UppHub client. Zero GUI dependencies.
2. **`DevEnv/Draw`**: Syntax tokenizers, theme palette blending, icon provider, high-DPI scaling.
3. **`DevEnv/Ctrl`**: Reusable developer UI controls (Code canvas, gutter, minimap, project tree, output console, diff viewer).
4. **`DevEnv/Ctrl/Core`**: Presenters, document lifecycle manager, auto-save, build event dispatchers.
5. **`DevEnv/Ctrl/Lib`**: UppHub GUI dialogs, assembly manager, toolchain editor, package browser.
6. **`DevEnv/App`**: Desktop application shell, docking layout manager, menu bar, DropTerm Quake console, main entry point.

## Legacy Compatibility
- `uppsrc/ide`: The complete existing U++ IDE application maintained alongside `DevEnv`. As features are ported into `DevEnv/*`, `ide` gradually delegates or acts as a backward-compatible wrapper.
