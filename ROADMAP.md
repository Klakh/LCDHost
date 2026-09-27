# Roadmap

This roadmap describes where the LCDHost revival is heading. It is a plan,
not a promise: priorities can change, and contributions on any item are
welcome (open an issue first for larger items so that work is not
duplicated).

Legend: ✅ done · 🚧 in progress · ⬜ planned

## Phase 0: Revival (done)

- ✅ Build again on Windows (current MSVC and Windows SDK) and Linux with Qt 5.15.
- ✅ Continuous integration on Linux and Windows, with a headless smoke test
  and downloadable builds.
- ✅ Fix startup crash and crashes when the selection is cleared.
- ✅ Fix plugin loading on Linux and install the bundled layouts on first run.
- ✅ Remove the insecure online update mechanism (unsigned downloads over HTTP).
- ✅ Remove integrations with discontinued software (Fraps, classic RivaTuner,
  EVEREST, ATITrayTools, Sidebar gadgets, online lyrics services).
- ✅ Contributor documentation, issue templates and this roadmap.

## Phase 1: Modern toolchain

- ✅ **Port to Qt 6** (6.8 or later, CI on 6.11) and drop Qt 4/5 code paths.
- ⬜ Replace the remaining deprecated Qt APIs reported by the compiler, and
  remove (or port to `QOpenGLWidget`) the OpenGL rendering path, which has
  been disabled at build time since the original project.
- ⬜ **Move the build from qmake to CMake**, the build system recommended for
  Qt 6, and remove the custom qmake feature machinery.
- ⬜ **Replace outdated bundled libraries** with current upstream versions,
  obtained through the system package manager on Linux and vcpkg on Windows:
  libusbx 1.0.15 → libusb, signal11 HIDAPI → libusb/hidapi, TagLib 1.7 →
  TagLib 2, and drop the bundled zlib 1.2.5 (known vulnerabilities).
- ⬜ Replace in-house helpers with Qt equivalents: `codeleap/json` →
  `QJsonDocument`.
- ⬜ DataViewer: stop using a layout-provided string as a `printf` format.
- ⬜ **Stop redistributing proprietary SDKs** (Logitech LCD SDK, iTunes COM
  SDK): let developers point the build to their own copy and make the
  dependent features optional.
- ⬜ Target Windows 10 or later (required by Qt 6) and remove Windows XP era
  workarounds.
- ⬜ Clean up compiler warnings and keep the build warning-free in CI.

## Phase 2: Repair broken features

- ✅ **Weather**: replace the defunct Yahoo Weather API with
  [Open-Meteo](https://open-meteo.com/) (free, no API key).
- ⬜ **RSS and web pages** (WebKit plugin): follow every kind of HTTP
  redirect, report network errors, resolve relative URLs, and make page
  capture reliable with Qt WebEngine (animated and JavaScript-driven pages).
- ⬜ **Now Playing**: use the operating system media APIs (Windows System
  Media Transport Controls, MPRIS on Linux) so that any modern player works,
  instead of per-player window hacks that broke over time (Spotify, iTunes /
  Apple Music). Support the VLC web interface password. Check whether the
  "MSN Now Playing" message receiver is still useful.
- ⬜ **TeamSpeak**: send the ClientQuery API key required since TeamSpeak
  3.1.3, and investigate TeamSpeak 5/6.
- ⬜ **Monitoring**: support the current HWiNFO shared memory interface
  (`HWiNFO_SENS_SM2`) and re-check the other sources (AIDA64, MSI Afterburner,
  CoreTemp, GPU-Z, SpeedFan).
- ⬜ **DataViewer**: support 64-bit processes.
- ⬜ Re-check Mailcount and DriveStats on current Windows versions.

## Phase 3: Hardware and platforms

- ⬜ **Hardware validation** with G13, G15, G19, G510 and Z10 owners (see the
  hardware test report issue form).
- ⬜ **G HUB**: find out whether the Logitech LCD SDK path still works with
  G HUB, alongside the kept Logitech Gaming Software support.
- ⬜ **Linux**: udev rules so that the direct USB/HID drivers work without
  root, and a distributable package (AppImage or Flatpak).
- ⬜ **Generic displays**: output drivers for non-Logitech screens, such as
  small USB or serial displays driven by an ESP32 or a Raspberry Pi, or frame
  streaming over the network.
- ⬜ Decide on macOS: the code has macOS support that nobody maintains or
  tests at the moment.

## Phase 4: Releases and quality

- ⬜ Tagged releases with a changelog, checksums and a Windows installer.
- ⬜ An optional, safe update notifier: check GitHub releases over HTTPS and
  tell the user, never download or run code automatically.
- ⬜ Automated tests for layout loading and saving, setup items and plugin
  loading.
- ⬜ Ask for confirmation before a layout launches a program (Cursor plugin
  actions).
- ⬜ User documentation: layout editor guide, plugin reference.
- ⬜ Translations of the user interface.
