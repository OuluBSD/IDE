# Upstream Tracking & Divergence Baseline

**Repository**: `OuluBSD/IDE`  
**Upstream Remote**: `https://github.com/ultimatepp/ultimatepp.git`  
**Tracking Branch**: `upstream-master`  
**Local Upstream Reference**: `/home/sblo/Dev/ultimatepp`  

---

## 1. Divergence Baseline Overview

When `ai-upp` was originally forked and modularized, the last commit created by an upstream author (prior to changes by Seppo Pakonen and AI agents) was:

| Field | Baseline Record |
| :--- | :--- |
| **Commit in upstream `ultimatepp`** | `dc3c90fe52faad0505bf2f832209b9e207ed6d89` |
| **Commit in `ai-upp`** | `cf30db52f4f96dd2e0d43a155665e2526bd95cc3` |
| **Tree SHA (Identical)** | `c111f1978447d46ccef796e791bd7d78c684c3f3` |
| **Author** | `Zbigniew Rębacz <klugier@yahoo.com>` |
| **Author Date** | `Sun Jan 4 19:18:15 2026 +0100` |
| **Commit Message** | `UMK, Doc: .cosmetics` |
| **Git Tag in this repo** | `upstream-baseline-2026-01-04` |

Both `ultimatepp` and `ai-upp` share the exact identical tree object `c111f1978447d46ccef796e791bd7d78c684c3f3` at this commit.

---

## 2. Upstream Repository State at Initialization

At the time `OuluBSD/IDE` was initialized:

| Field | Upstream Status |
| :--- | :--- |
| **Current Upstream HEAD** | `74d19285b014cb8ba0197f5395af7d0c8ad99fd1` |
| **Author** | `Mirek Fidler <mirek.fidler@gmail.com>` |
| **Author Date** | `Fri Oct 30 23:03:50 2026 +0100` |
| **Commit Message** | `ide: first attempt at AI review` |
| **Git Tag in this repo** | `upstream-head-2026-10-30` |
| **Total Commits Ahead of Baseline** | 491 commits in `ultimatepp` |
| **Commits Touching `uppsrc/ide/`** | 206 commits |

---

## 3. Git Tracking Architecture

This repository maintains the entire upstream object database in `.git/` via the tracking branch `upstream-master`.

### Remotes
- `origin`: `git@github.com:OuluBSD/IDE.git` (Active development repository)
- `upstream`: `https://github.com/ultimatepp/ultimatepp.git` (Upstream pristine source)

### Tracking Tags
- `upstream-baseline-2026-01-04`: Points directly to baseline commit `dc3c90fe52faad0505bf2f832209b9e207ed6d89`.
- `upstream-head-2026-10-30`: Points directly to upstream HEAD commit `74d19285b014cb8ba0197f5395af7d0c8ad99fd1`.

---

## 4. Reconciliation & Merge Protocol for AI Agents

Because `OuluBSD/IDE` was branched from `dc3c90fe52faad0505bf2f832209b9e207ed6d89`, git shares a genuine 3-way merge base with `upstream-master`.

### 4.1 Inspecting Upstream IDE Commits
To view all commits made to `uppsrc/ide` by upstream developers since divergence:
```bash
git log --oneline upstream-baseline-2026-01-04..upstream-master -- uppsrc/ide
```

### 4.2 Inspecting a Specific Upstream Commit
```bash
git show <commit-sha> -- uppsrc/ide
```

### 4.3 Diffing a Specific File or Directory
To see the full diff between the baseline and upstream HEAD for any file:
```bash
# Compare upstream HEAD against baseline
git diff upstream-baseline-2026-01-04..upstream-master -- uppsrc/ide/vcpkg.cpp

# Compare our active working tree against upstream HEAD
git diff upstream-master -- uppsrc/ide/
```

### 4.4 Porting an Upstream Commit
When porting upstream improvements (e.g. clang updates, cross-compile, SBOM, debugger fixes):
1. Check the commit diff with `git show <commit-sha>`.
2. Apply the change to `uppsrc/ide/` or adapt it into the modularized structure in `uppsrc/DevEnv/`:
   - If the file was moved to a subpackage (e.g., `uppsrc/ide/MainCtrl/` or `DevEnv/Ctrl/Lib/`), apply the relevant diff surgically to the target file.
3. Test compilation using `bin/uppbuilder`.
4. Commit with reference to upstream commit SHA:
   ```bash
   git commit -m "port(ide): <description> (upstream <commit-sha>)"
   ```

---

## 5. Fetching New Upstream Changes

To update the local upstream tracking branch with future upstream commits:
```bash
# From local ultimatepp clone (instant):
git fetch /home/sblo/Dev/ultimatepp master:upstream-master

# Or directly from GitHub:
git fetch upstream master:upstream-master
```
