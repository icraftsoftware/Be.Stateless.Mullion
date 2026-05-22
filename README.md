# Mullion

A lightweight native Win32 window manager for Windows 10/11, inspired by the original [WinSplit Revolution](https://github.com/x-vibe/winsplit-revolution-revived).

A *mullion* is the vertical or horizontal bar dividing panes in a window frame — fitting for a tool that divides... and conquers your screen.

## Features

- Snap windows to halves, thirds, and corners via keyboard shortcuts
- Stretch-snap with `Shift` held to resize without moving
- Mouse-driven snapping by dragging windows to screen edges
- System tray icon with autostart toggle
- Zero runtime dependencies — a single lean native executable

## Key Bindings

All shortcuts use `Win + Alt + ...`

| Key | Action |
|-----|--------|
| `←` | Snap left half |
| `→` | Snap right half |
| `↑` | Snap top half |
| `↓` | Snap bottom half |
| `C` | Center window |

Hold `Shift` with any arrow to stretch the window instead of moving it.

## Building

Requires Visual Studio 2022 or later with the C++ desktop workload.

Open `Mullion.slnx` and build. No dependencies, no package restore.

## License

Copyright © 2026 François Chabot

Licensed under the [Apache License, Version 2.0](LICENSE).
