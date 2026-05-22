# Reprise prompt for new Claude Code sessions

Paste this when starting a new session in Rider or elsewhere:

---

Check memory for full context. We're building WinSplit 12 — a minimal native Win32 window manager in `v12.0.0/` (never touch other folders). It builds, runs, and the core features work.

## Current focus: file-by-file code review + refactoring

Goal: understand every line thoroughly, apply idiomatic C++20, design RAII classes for Tray/Hotkeys/MouseHook as each file is reviewed. No unit tests yet — proceed carefully.

### Files done

**`runtime.vcxproj`** — bumped `LanguageStandard` to `stdcpp20` (both Debug and Release).

**`main.cpp`** — fully reviewed and refactored:
- Anonymous namespace for `ApplicationName`, `HandleCommand`, `HandleMessage`
- C++20 designated initializers for `WNDCLASSEXW`
- Naming: `singleInstanceGuard`, `messageWindow`, `messageType`, `HandleMessage`, `HandleCommand`
- `WM_DESTROY` still calls `Tray_Remove()`, `Hotkeys_Uninstall()`, `MouseHook_Uninstall()` explicitly — remove after RAII classes are built

**`resource.h`** — fully reviewed and cleaned up:
- `#define WIN32_LEAN_AND_MEAN` + `#include <Windows.h>` (self-contained)
- `WM_*` macros → `constexpr UINT`: `WmTrayNotification`, `WmSnap`, `WmCenter`
  - Naming convention: **verbs = commands** (`WmSnap`, `WmCenter`), **nouns = notifications** (`WmTrayNotification`)
- All enums are `enum class` with `// NOLINT(performance-enum-size)` (consciously not using `uint8_t` base)
- `IconId { Tray = 1 }` — move to `tray.h` when reviewing tray.cpp
- `MenuCommand { AutoStart, Exit, ReloadConfig }` — values 100/101/102
- `SnapDirection { Down, Left, Right, Up }` — TODO: reorder to `Left, Right, Up, Down` (axis-grouped)
- `SnapModes { None = 0, Stretch = 0x1 }` — proper flag enum; TODO: add `operator|` / `operator&` when reviewing `placement.cpp`

### Next: `tray.cpp`
Design and implement RAII `Tray` class during review.
After all three RAII classes done → clean up `WM_DESTROY` in `main.cpp`.

---

The core idea: user has multiple monitor setups (home/work/laptop) and apps forget their positions on layout changes. Hotkeys let them rapidly reposition windows. No UI, no dialogs — tray + keyboard only.

Key design decisions already made:
- Win+Alt+Arrow for ring navigation (not Ctrl+Alt — conflicts with Rider/ReSharper)
- WH_KEYBOARD_LL not RegisterHotKey (immune to Teams hotkey theft during screen sharing)
- DWM border fix for Chrome/modern app offset (invisible resize frame on Win10+)
- Sticky edges: primary monitor, 514px accumulated force, Left Ctrl to bypass
- Pure Win32, zero external dependencies, VS 2026 toolset v145

---

## Start here: explain these bugs, then ask which to fix first

**Bug 1 — Stale cross-axis slot**
After vertical navigation, `g_last.h_slot` is stale. Horizontal hotkey on same window → `SamePosition()` returns true, uses wrong slot → window jumps to unexpected position.
Fix: track which axis was last used, invalidate the other on each placement.

**Bug 2 — Monitor config changes not handled**
`g_primary_mon`, `g_primary_rect`, virtual desktop bounds captured once at startup. Plug/unplug monitor → sticky edges use stale geometry until restart.
Fix: handle `WM_DISPLAYCHANGE` in `WndProc`, call `MouseHook_Reinit()`.

**Bug 3 — g_prev_pt cold-start (likely already fixed, verify)**
`g_prev_pt` was `{0,0}` — could falsely trigger friction on first mouse move if cursor is on secondary. `InitPrimary()` now calls `GetCursorPos` — confirm it guards the first event.

**Bug 4 — GetWindowRect timing in ApplyRect**
`SetWindowPlacement` to restore maximized window, then immediately `GetWindowRect` for DWM offset — rect may not be settled yet. May need `UpdateWindow` or deferred approach.

---

## Ring redesign (discuss with user)

Current ring is **non-monotonic** — pressing → goes through center slots mid-journey:
`L66 → Full → C66 → C50 → C66 → Full → R66` — backtracks, feels bumpy.

Proposed fix — **strictly monotonic, no duplicates**:
- H ring: `L33 → L50 → L66 → Full → R66 → R50 → R33 → [wrap]`
- V ring: `T33 → T50 → T66 → Full → B66 → B50 → B33 → [wrap]`

Center positions move to a dedicated **Win+?+C centered ring**:
`100% → 80%×80% → 66%×66% → 50%×50% → 33%×33% → [wrap]`

---

## Hotkey situation (needs resolution)

Current: Win+Alt+Arrow — Win+Alt+Shift is ergonomically bad on user's keyboard.
Proposed: Win+Ctrl (primary), Win+Ctrl+Shift (secondary).
**Blocker**: Win+Ctrl+Arrow = Windows virtual desktop switching — CONFLICT.
Need comprehensive conflict audit against: Windows, Rider/ReSharper, Word, VS Code,
Sublime, Terminal/PowerShell, Edge, Chrome, Bitwarden.
Config system is the real answer — let users bind whatever fits their keyboard.

---

## Also on the agenda for tomorrow

- **Name the tool** — needs a real name before GitHub publish (see ROADMAP.md brainstorm)
- **GitHub publish** — source + portable signed release as GH Release; SignPath.io for free OSS signing; GH Actions CI
- **SpecKit** — user wants to bring it in for unit tests; clarify which framework they mean
- **ROADMAP.md** is now in the repo — keep it updated

## Polishing agenda (see ROADMAP.md + memory)

- Unit tests — needed throughout (ring arithmetic, DWM offset math, sticky edge logic, etc.)
- Code quality — pass Rider code analysis, clean structure, Rider default C++ formatting
- Config system — hotkeys, ring ratios, sticky force; "Reload Config" already stubbed
- Tray icon — replace default IDI_APPLICATION with something real
- Auto-placement per app — remember position by process+class name (key for home/work/laptop)
- Throw to next monitor — move window to adjacent monitor (had it in v9)
- Always-on-top toggle
- WinSplit12.slnx is the active solution (Rider), keep it
