# Reprise prompt for new Claude Code sessions

Check memory for full context. We're building Mullion — a minimal native Win32 window manager.
It builds, runs, and the core features work. Active branch: `feature/snap`.

## Current focus: file-by-file code review + refactoring

Goal: understand every line thoroughly, apply idiomatic C++20, design RAII classes for
HotKeys and MouseHook (Tray and Application are already done).

### Files done

**`Mullion.vcxproj`** — `LanguageStandard` set to `stdcpp20` (Debug and Release).

**`main.cpp`** — fully reviewed and refactored. Now a single call to `Application::Run(hInstance)`.

**`resource.h`** — fully reviewed and cleaned up:
- `WM_*` macros → `constexpr UINT`: `WmTrayNotification`, `WmSnap`, `WmCenter`
  - Naming convention: **verbs = commands** (`WmSnap`, `WmCenter`), **nouns = notifications** (`WmTrayNotification`)
- All enums are `enum class`
- `MenuCommand { AutoStart, Exit, ReloadConfig }` — values 100/101/102
- `SnapDirection { Down, Left, Right, Up }` — TODO: reorder to `Left, Right, Up, Down` (axis-grouped)
- `SnapModes { None = 0, Stretch = 0x1 }` — proper flag enum; TODO: add `operator|` / `operator&` when reviewing `placement.cpp`

**`Tray` (Windows/Shell/Tray.h/cpp)** — full RAII class:
- Constructor takes `HWND`, destructor removes tray icon
- Copy and move deleted
- `HandleNotification()`, `ToggleAutostart()`, `ShowContextMenu()`
- Autostart via registry (`Software\Microsoft\Windows\CurrentVersion\Run`)

**`Application` (Application.h/cpp)** — RAII orchestrator:
- `static int Run(HINSTANCE)` — message window, message loop
- `inline static std::optional<Windows::Shell::Tray> tray` — owns the Tray lifetime
  - Has `// NOLINT(clang-diagnostic-unique-object-duplication)` — intentional, single executable
- `WM_DESTROY` is clean — no manual teardown, RAII handles it

### Next: `hotkeys.cpp`

Still C-style free functions (`HotKeys_Install` / `HotKeys_Uninstall`).
Design and implement RAII `HotKeys` class during review.
Move to `Windows/Input/HotKeys.h` → `Be::Stateless::Mullion::Windows::Input::HotKeys`.

Then: `mouse_hook.cpp` → RAII `Mouse` class.
Move to `Windows/Input/Mouse.h` → `Be::Stateless::Mullion::Windows::Input::Mouse`.
("MouseHook" is an implementation detail — the class is just `Mouse`.)

Then: `placement.cpp` — ring logic, DWM offset math, slot tracking.

---

## Key design decisions

- Win+Alt+Arrow for ring navigation (not Ctrl+Alt — conflicts with Rider/ReSharper)
- WH_KEYBOARD_LL not RegisterHotKey (immune to Teams hotkey theft during screen sharing)
- DWM border fix for Chrome/modern app offset (invisible resize frame on Win10+)
- Sticky edges: primary monitor, 514px accumulated force, Left Ctrl to bypass
- Pure Win32, zero external dependencies, VS 2026 toolset v145
- Namespace mirrors folder structure exactly: `Be::Stateless::Mullion` is the root, subfolders add segments (e.g. `Windows/Shell/Tray.h` → `Be::Stateless::Mullion::Windows::Shell::Tray`) — same convention as Microsoft's own Win32/WinRT headers

---

## Known bugs (see memory/known_issues.md)

1. **Stale cross-axis slot** — after vertical nav, `g_last.h_slot` stale → wrong slot on next H hotkey
2. **WM_DISPLAYCHANGE not handled** — sticky edge geometry stale after monitor plug/unplug
3. **g_prev_pt cold-start** — likely already fixed via `GetCursorPos` in `InitPrimary`; verify
4. **ApplyRect timing** — `GetWindowRect` immediately after `SetWindowPlacement` may read unsettled rect

---

## Ring redesign (roadmap priority)

Current ring is non-monotonic — center slots cause backtracking mid-journey.
Proposed: strictly monotonic H/V rings, center positions moved to a dedicated Win+?+C ring.
See memory/roadmap.md for full spec.

---

## Also on the agenda

- Hotkey conflict audit (Win+Ctrl+Arrow = virtual desktops — CONFLICT)
- Config system — hotkeys, ring ratios, sticky force; "Reload Config" already stubbed
- Unit tests — ring arithmetic, DWM offset math, sticky edge logic (SpecKit?)
- Tray icon — replace IDI_APPLICATION with something real
- GitHub publish — SignPath.io OSS signing, GH Actions CI
