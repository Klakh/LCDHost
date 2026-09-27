# Changelog

All notable changes to this project are documented in this file. The format
is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/).

Versions up to 0.0.38 were released by the original author; see the
[original repository](https://github.com/linkdata/LCDHost).

## [Unreleased]

## [0.0.41] - 2026-09-27

### Removed

- The automatic update mechanism. It downloaded plugins and installers over
  plain HTTP without verifying them, and its server no longer exists.
- Monitoring sources for discontinued software: Fraps, classic RivaTuner (and
  the RivaTuner shared memory writer), ATITrayTools, HWMonitor and Logitech
  battery Sidebar gadgets, and the EVEREST fallback of the AIDA64 source.
- Online lyrics lookup in Now Playing (LyricWiki, LyrDB and Letras are gone).
  The `{lyrics}` token is kept but stays empty.
- Unused sources, build outputs and obsolete scripts.

### Changed

- The About and Welcome dialogs point to the GitHub repository instead of
  the defunct forum.
- Line endings are normalized to LF.

### Added

- Contributor documentation, code of conduct, security policy, issue and
  pull request templates, roadmap and architecture overview.

## [0.0.40] - 2026-09-27

### Fixed

- Windows builds with current MSVC and Windows SDK (Qt 4 API leftovers,
  struct packing around Windows headers, missing `user32` link).
- The fallback to the default layout directory looked for the wrong path.

### Added

- Bundled layouts are installed on first run.
- Windows CI build with the Qt runtime and layouts included, and a startup test.

## [0.0.39] - 2026-09-27

### Fixed

- Crash at startup and when the class or instance selection is cleared.
- Linux: plugins now load regardless of the working directory.

### Added

- Plugin load failures are logged.
- Build instructions, headless smoke test and Linux CI.

[Unreleased]: https://github.com/LokLakh-s/LCDHost/compare/v0.0.41...HEAD
[0.0.41]: https://github.com/LokLakh-s/LCDHost/releases/tag/v0.0.41
[0.0.40]: https://github.com/LokLakh-s/LCDHost/releases/tag/v0.0.40
[0.0.39]: https://github.com/LokLakh-s/LCDHost/releases/tag/v0.0.39
