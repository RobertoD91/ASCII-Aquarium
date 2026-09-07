# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## What this is

ASCII Aquarium CYD is an Arduino/ESP32 firmware sketch: an animated ASCII fish-tank
screensaver for the ESP32-2432S028R "Cheap Yellow Display" (CYD) and related boards
(ILI9341/XPT2046 for CYD & CYD2USB, ST7796U for the JC3248w535, ESP32-C3 + ST7789 for
the CyberSaiyan WHY2025/EMF2026 and RHC22 conference badges). There is no
app/server/test-suite layer — the entire product is a single `.ino` sketch rendered
live on-device, plus a static GitHub Pages web flasher under `docs/`.

## Repository layout

- `ASCII_Aquarium_CYD/ASCII_Aquarium_CYD_2.39.ino` — the current sketch (single file,
  ~7.6k lines). `ASCII_Aquarium_CYD_2.20.ino` is the previous release, kept for
  reference/rollback; **edit the highest-numbered version**, and when cutting a new
  release, copy it to a new `_X.Y.ino` file rather than overwriting history.
- `User_Setup.h`, `User_Setup_Select_CYD.h` — TFT_eSPI library configuration snippets.
  These are *not* compiled as part of the sketch; they must be copied into the
  installed `TFT_eSPI` library folder (see comments in each file) before building for
  the base CYD board. Keep the rest of TFT_eSPI's stock `User_Setup_Select.h` intact.
- `User_Setup_CyberSaiyanC3.h` — the same kind of TFT_eSPI setup snippet, for the
  CyberSaiyan ESP32-C3 badges (see the board profile note below).
- `docs/` — the GitHub Pages web flasher (`index.html`, using `esp-web-install-button` /
  ESP Web Tools) plus per-board `manifest*.json` files and the merged firmware `.bin`
  binaries it serves. `docs/firmware/*-merged.bin` are the release artifacts produced
  by Arduino IDE's "Export Compiled Binary".
- `ASCII_Aquarium_Release_Notes_v*.md` — per-version changelogs; add a new one when
  bumping the sketch version.
- `.github/workflows/build-firmware.yml` — CI: compiles the sketch with `arduino-cli`
  for the CYD and CyberSaiyan ESP32-C3 board profiles on every push/PR that touches
  the sketch or a `User_Setup*.h`. This only proves the firmware builds; it does not
  produce the release `.bin` artifacts under `docs/firmware/` (still done manually via
  Arduino IDE's "Export Compiled Binary", per the workflow below).

## Build / flash workflow

There is no CLI build system (no Makefile, PlatformIO config, or CI) — building
happens in the Arduino IDE:

1. Install the ESP32 board package and the `TFT_eSPI`, `XPT2046_Touchscreen` libraries.
2. For the base CYD board, copy `User_Setup.h` into the `TFT_eSPI` library's root
   folder, and make sure `User_Setup_Select.h` in that library still includes it
   (see the instructions inside `User_Setup_Select_CYD.h`).
3. Open the current `ASCII_Aquarium_CYD_*.ino` in Arduino IDE.
4. Select the ESP32 board matching the target hardware. Board variant is selected at
   compile time via a preprocessor define, not by editing source:
   - Base CYD (ILI9341, default): no define needed.
   - ST7796U-based boards (e.g. JC3248w535): define `AQUARIUM_BOARD_ST7796U35`
     (board-specific `#if defined(AQUARIUM_BOARD_ST7796U35)` blocks near the top of
     the sketch swap pin mappings, screen geometry, and touch bus behavior).
   - CyberSaiyan ESP32-C3 badges (WHY2025/EMF2026, RHC22): board "ESP32C3 Dev
     Module", partition scheme **"Minimal SPIFFS (1.9MB APP with OTA/128KB
     SPIFFS)"** (the default 1.2MB-APP scheme leaves too little headroom — see
     below), define `AQUARIUM_BOARD_CYBERSAIYAN_C3`, and copy
     `User_Setup_CyberSaiyanC3.h` (not `User_Setup.h`) into `TFT_eSPI`. This
     profile has no electrically-wired touch controller (the panel's TSC2007 is
     I2C-only and unused) and no SD card slot, so touch and BMP capture are
     always inactive; the backlight lives behind an AW9523 I2C GPIO expander
     (`initCyberSaiyanBacklight()`) instead of a plain PWM pin. WHY2025/EMF2026
     and RHC22 share one profile — same ESP32-C3 + ST7789 240x320 wiring.
5. Compile and upload, or use **Sketch → Export Compiled Binary** and place/update
   the resulting merged `.bin` under `docs/firmware/` (and bump the matching
   `docs/manifest-*.json` `version`/`path`) to ship it through the web flasher.

There is no automated test suite; validation is manual, on real hardware (or by
reading the serial monitor boot diagnostics printed in `setup()`).

## Sketch architecture

The `.ino` file is organized top-to-bottom into clearly marked `// ---- Section ----`
banners; when navigating, search for these rather than assuming file layout:

1. **Board Touch Pins / Button/SD Capture / Display Geometry / Settings UI /
   Aquarium Controls** — compile-time constants and tunables (screen size, default
   fish/bubble counts, UI layout coordinates, color tables). Most "how many fish",
   "what's the min/max", "where is this button" questions are answered here.
2. **Objects** — `struct`/`enum` definitions for every simulated entity: `Fish`,
   `Bubble`, `Flake` (food), `Octopus`, `Seahorse`, `Snail`, `Jellyfish`, `Squid`, plus
   UI enums (`SettingsTab`, `WifiPanelMode`, `ClockDisplayStyle`, `BackgroundStyle`,
   etc.) and `AsciiClockFont`/`AsciiClockGlyph` for the selectable ASCII clock fonts.
3. **Globals** — the live simulation/UI state (fish arrays, settings values, Wi-Fi
   state machine flags, capture/SD state, lighting state). Nearly everything is a
   global; there's no per-object encapsulation beyond the structs above.
4. **Utility** — string/safety helpers, capture (BMP screenshot/sequence) helpers,
   frequency-normalization helpers for creature spawn rates.
5. **Aquarium Logic** — the `update*()` functions that step the simulation each frame
   (fish movement/schooling/avoidance, bubbles, flakes, visiting creatures, clock,
   Wi-Fi service loop, settings persistence, light schedule/Auto Sky).
6. **Drawing** — everything rendered into the offscreen `TFT_eSprite` canvas each
   frame (background gradients, creatures, HUD, settings panels, clock).
7. **Input Handling** — touch hit-testing and the settings/Wi-Fi/keyboard panel state
   machines.
8. **Setup / Loop** — `setup()` allocates the render sprite first (before Wi-Fi/NVS
   can fragment heap), initializes display/touch/lighting, loads persisted settings,
   and seeds the initial creature population; `loop()` is a fixed pipeline of
   `service*()`/`update*()` calls each frame followed by `renderFrame()`.

Key patterns to know before making changes:

- **Rendering** happens entirely into an offscreen `TFT_eSprite` ("canvas"), then
  pushed to the physical `tft`; there's a low-memory fallback path that renders in
  horizontal strips (`stripRenderActive`) when the full-frame sprite can't be
  allocated — check `mainCanvasActualColorDepth`/`spriteReady` before assuming a
  full-color, full-height canvas is available.
- **Settings persistence** uses the ESP32 `Preferences` (NVS) API and is
  debounced/batched via `markSettingsDirty()` + `serviceSettingsPersistence()`, not
  written synchronously on every change — follow that pattern for new settings so
  NVS isn't hammered every frame.
- **Frequency-style settings** (octopus/seahorse/snail/jellyfish/auto-feed spawn
  rates) are stored as normalized ints and go through
  `normalize*Frequency()`/`cycle*Frequency()` helper pairs — mirror this for any new
  spawn-rate setting rather than inventing a new scheme.
- **Time** is decoupled from wall-clock `millis()`: `aquariumNowMs` advances by a
  clamped step each loop (and is slowed deliberately during BMP sequence capture via
  `CAPTURE_RECORD_FRAME_MS`) so recording a capture sequence doesn't speed up or
  freeze the simulation's apparent motion.
- **SD capture** (BMP screenshots/sequences) temporarily takes over the shared SPI
  bus (`beginCaptureSdBus()`/`restoreTftBus()`); on low memory it frees render
  buffers first — anything touching capture must restore the TFT bus state
  afterward or the display will corrupt.
- **RISC-V ESP32 targets** (C3/S2/S3) only expose one general-purpose SPI host to
  Arduino (`FSPI`), not the classic ESP32's separate `HSPI`/`VSPI` — the sketch
  maps the old names onto `FSPI` near the top of the file when the core doesn't
  define them, since `SPIClass touchSPI(HSPI)`/`sdSPI(VSPI)` are declared
  unconditionally. Keep this in mind before adding another `HSPI`/`VSPI` use.
- **Board-specific `arduino-cli` defines** must go through `compiler.cpp.extra_flags`
  / `compiler.c.extra_flags`, never `build.extra_flags` — the esp32 core's
  `platform.txt` stuffs its own required defines (notably `-DESP32=ESP32`) into
  `build.extra_flags`'s *default value*, and a `--build-property
  build.extra_flags=...` override silently replaces that default instead of
  appending to it, which breaks TFT_eSPI's processor detection. See
  `.github/workflows/build-firmware.yml` for the working pattern.
