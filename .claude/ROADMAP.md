# Mullion — Roadmap

> Lean native Win32 window manager. One process, zero dependencies, hotkey-driven.

---

## Immediate — bugs & ring feel

- [ ] **Ring redesign** — make directional rings strictly monotonic, no duplicate waypoints
  - H: `L33 → L50 → L66 → Full → R66 → R50 → R33 → [wrap]`
  - V: `T33 → T50 → T66 → Full → B66 → B50 → B33 → [wrap]`
- [ ] **Centered ring** — Win+?+C cycles centered sizes: `100% → 80% → 66% → 50% → 33% → [wrap]`
- [ ] **Bug: stale cross-axis slot** — after vertical nav, h_slot is stale; fix by tracking last axis
- [ ] **Bug: WM_DISPLAYCHANGE** — reinit sticky edges geometry on monitor plug/unplug
- [ ] **Bug: ApplyRect timing** — GetWindowRect may read unsettled rect after restore-from-maximized

## Hotkeys

- [ ] **Conflict audit** — check default modifiers against: Windows (Win+Ctrl+Arrow = virtual desktops!), Rider/ReSharper, Word, VS Code, Sublime, Terminal/PowerShell, Edge, Chrome, Bitwarden
- [ ] **Ergonomic defaults** — Win+Alt+Shift is bad; Win+Ctrl / Win+Ctrl+Shift are candidates but need audit first
- [ ] **Config system** — all hotkeys user-configurable; `Reload Config` menu item already stubbed

## Config system

- [ ] Config file format (XML or INI) for: hotkeys, ring ratios, sticky edge force
- [ ] Centered ring steps configurable
- [ ] `INCLUDE_THIRDS` toggle in config (currently a compile-time `#define`)
- [ ] Config editor / UI — planned, not started

## Code quality

- [ ] **Unit tests** — ring arithmetic, FindNearest, SamePosition, DWM offset math, sticky edge logic, autostart registry; consider SpecKit
- [ ] Pass all Rider code analysis warnings (already flagging issues)
- [ ] Rider default C++ code formatting/layout
- [ ] Solid, clean, well-structured code

## Features — next up

- [ ] **Throw to next monitor** — move window to adjacent monitor, proportionally scaled (was in v9)
- [ ] **Always-on-top toggle**
- [ ] **Undo last placement** — snap window back to pre-hotkey position
- [ ] **Auto-placement per app** — remember position keyed by process+class name (critical for home/work/laptop)
- [ ] **Startup toast** — tray balloon on first run so users know the tool is alive

## Features — later

- [ ] Corners — no clean solution with 4 arrow keys on mini keyboard; deferred
- [ ] Window fusion — tile two specific windows together (was in v9)
- [ ] Mosaic mode — native Windows tiling (was in v9)
- [ ] Config editor binary

## Polish

- [ ] **Tray icon** — replace IDI_APPLICATION with a real icon
- [ ] **Name** — find a good name (see brainstorm below)

## Ship

- [ ] **GitHub repo** — publish source
- [ ] **Portable signed release** — single `.exe`, no installer; code signing (SignPath.io = free for OSS)
- [ ] GitHub Actions CI — build on push, publish release on tag

---

## Name brainstorm

Need something short, memorable, distinct from "WinSplit" (which is taken/legacy).

Ideas — feel free to add:
- **Helm** — steering your windows, nautical control
- **Orbit** — the ring navigation
- **Pivot** — repositioning, clean
- **Reticle** — precision targeting/placement
- **Posto** — Italian for "place/position"
- **Keel** — stability, multi-monitor balance
- **Prism** — multi-monitor, direction

Criteria: ≤6 chars ideally, no existing Win32 tool with that name, GitHub repo available.

---

## Notes

- Older WinSplit codebases for reference: `../v9.0.2/`, `../v10.x/` (read-only)
- DMT source + config for sticky edges reference: `C:\Files\Portables\PortableApps\DualMonitorTools\`
- Solution file: `Mullion.slnx`
- Toolchain: VS 2026 Enterprise, MSVC v145, C++17, pure Win32
