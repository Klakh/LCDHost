# Third-party components

LCDHost is licensed under the GNU GPL v3 or later. This repository also
contains code from other projects, which keeps its own license. The table
below lists what is bundled, where, and under which terms.

| Component | Version | Location | License | Used by |
|---|---|---|---|---|
| [libusbx](https://libusb.info/) (now merged back into libusb) | 1.0.15 | `linkdata/lh_libusbx/` | LGPL 2.1 or later | `LH_Lg320x240` (G19 driver) |
| [HIDAPI](https://github.com/signal11/hidapi) (signal11) | 2009 | `applications/lh_hid/hidapi/` | GPL v3, BSD-style, or original HIDAPI license (at your choice) | `lh_hid`, Logitech HID drivers |
| [TagLib](https://taglib.org/) | 1.7.0 | `3rdParty/taglib/` | LGPL 2.1 or MPL 1.1 | `LH_NowPlaying` (cover art, tags) |
| [zlib](https://zlib.net/) | 1.2.5 | `3rdParty/zlib/` | zlib license | TagLib |
| Logitech LCD SDK (`lglcd.h`, `lglcd.lib`, `liblgLcd.a`) | 3.0 or later | `plugins/LH_LgLcdMan/win/`, `plugins/LH_LgLcdMan/mac/` | **Proprietary** (Logitech LCD SDK License Agreement) | `LH_LgLcdMan` |
| iTunes COM SDK (generated headers) | — | `plugins/LH_NowPlaying/SDKs/iTunes/` | **Proprietary** (Apple) | `LH_NowPlaying` |
| Winamp SDK headers (`wa_*.h`) | — | `plugins/LH_NowPlaying/SDKs/Winamp/` | Nullsoft SDK terms | `LH_NowPlaying` |
| Icons based on the Silk set (resized to 12, 14 and 16 px) | — | `layouts/g19-eos/Status Icons/` | [CC BY 2.5](https://creativecommons.org/licenses/by/2.5/), Silk by Mark James (famfamfam.com) | sample layout |

## Notes

- **Proprietary SDKs.** The Logitech LCD SDK and the iTunes COM SDK were
  shipped by the original project. Their license agreements are not included
  in this repository, and it has not been verified that they allow
  redistribution. Removing them from the repository (and asking developers to
  install the SDKs themselves) is tracked in the [roadmap](../ROADMAP.md).
- **Outdated copies.** The bundled libusbx, HIDAPI, TagLib and zlib are more
  than ten years old and miss security fixes (zlib 1.2.5 in particular).
  Replacing them with current upstream versions, ideally through a package
  manager, is part of the roadmap.
- **Sample layouts.** The artwork in `layouts/` comes from the original
  project and its community. If you hold rights on an image and want it
  removed or credited differently, please open an issue.

If you add a third-party component, add it to this table in the same pull
request.
