# LCDHost

[![build](https://github.com/LokLakh-s/LCDHost/actions/workflows/build.yml/badge.svg)](https://github.com/LokLakh-s/LCDHost/actions/workflows/build.yml)
[![License: GPL v3](https://img.shields.io/badge/License-GPLv3-blue.svg)](COPYING)

LCDHost is a compositing plugin manager for secondary displays. It renders
layouts (clocks, system monitors, media players, graphs, images…) and sends
them to small screens, originally the LCDs of Logitech G-series keyboards
(G15, G13, G510, G19).

This repository revives the project, which its original author
[Johan Lindh](https://github.com/linkdata/LCDHost) stopped maintaining in 2016.
The goal is to keep existing layouts working while bringing the code up to
date. See the [roadmap](ROADMAP.md).

> **Status:** early revival. The application builds and runs on Windows and
> Linux with Qt 5.15. The Qt 6 port is in progress. Real hardware has not been
> re-tested yet: reports from G15/G19 owners are very welcome.

## Features

- Layout editor with live preview, drag and drop, and per-element settings.
- Plugins for text, dials, bars, graphs, images, decorations, cursors and
  menus, logic, system monitoring (AIDA64, MSI Afterburner, CoreTemp, GPU-Z,
  HWiNFO, SpeedFan), now playing (iTunes, Winamp, foobar2000, VLC, Spotify),
  TeamSpeak 3, web pages and RSS feeds.
- Output drivers for Logitech 160×43 and 320×240 LCDs (direct HID/USB or
  through the Logitech LCD SDK) and a **virtual LCD** for testing without
  hardware.

## Getting LCDHost

There is no release yet. Every build of `master` produces ready-to-run
folders for Windows x64 and Linux x64: open the latest successful
[build run](https://github.com/LokLakh-s/LCDHost/actions/workflows/build.yml)
and download `LCDHost-windows-x64` or `LCDHost-linux-x64`.

On first start:

1. Plugins are disabled by default. Enable the ones you need in the
   **Plugins** tab (select a plugin, then **Load**).
2. Pick an output device in the **Output** tab. Without hardware, use one of
   the *Virtual* devices provided by the VirtualLCD plugin.
3. The bundled layouts are copied to `Documents/LCDHost/layouts/` the first
   time LCDHost starts.

## Building from source

Requirements: Qt **5.15** (with the `qtwebengine` module for the WebKit
plugin) and a C++ compiler.

### Linux

```sh
sudo apt-get install libudev-dev libgl1-mesa-dev
mkdir build && cd build
qmake ../LCDHost.pro -r
make -j"$(nproc)"
cp -r ../layouts LCDHost.app/
./LCDHost.app/bin/LCDHost
```

### Windows

With Qt 5.15 (`win64_msvc2019_64`) and Visual Studio 2019 or 2022, from an
"x64 Native Tools" command prompt:

```bat
mkdir build && cd build
qmake ..\LCDHost.pro -r
nmake
windeployqt --release --no-translations LCDHost.app\bin\LCDHost.exe
windeployqt --release --no-translations LCDHost.app\bin\WebKitServer.exe
xcopy /E /I ..\layouts LCDHost.app\layouts
```

### Smoke test

`tests/smoke/run.sh` starts LCDHost headless with the default layout and
checks that it does not crash and that the core plugins load:

```sh
tests/smoke/run.sh build/LCDHost.app/bin 20
```

## Documentation

- [Architecture overview](docs/ARCHITECTURE.md): how the host, plugins,
  layouts and devices fit together.
- [Contributing](CONTRIBUTING.md): how to report bugs, propose changes and
  submit pull requests.
- [Roadmap](ROADMAP.md) and [changelog](CHANGELOG.md).
- [Third-party components](docs/THIRD_PARTY.md) bundled in this repository.
- [Security policy](SECURITY.md).

## License

LCDHost is free software, released under the
[GNU General Public License v3](COPYING) or later. The plugin API header
(`linkdata/lh_api5plugin/lh_plugin.h`) is BSD-licensed so that plugins can
use any license. Bundled third-party code keeps its own license, see
[docs/THIRD_PARTY.md](docs/THIRD_PARTY.md).

Copyright © 2009–2016 Johan Lindh (Link Data Stockholm), Andy Bridges
(CodeLeap) and contributors.
