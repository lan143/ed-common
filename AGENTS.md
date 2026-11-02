# AGENTS.md — ed-common

Read this first. It orients agents in the repo; `README.md` has the full user-facing docs,
`RTK.md` describes condensed command output.

## What this is

`ed-common` is an Arduino/ESP32 **library** (namespace `EDCommon`, version 0.3.x), not a
firmware app. It provides Home Assistant-oriented entities: relay, light, sensor,
binary_sensor, counter, automation, and a header-only FastLED-compatible color module.
It is consumed as a git `lib_deps` by sibling firmware projects — there is no
`platformio.ini`, CI, linter config, or license here.

## Layout

```
src/<module>/            one directory per module, no umbrella header; include e.g. <relay/relay.h>
  ├── *.h / *.cpp        base class + hardware backend subclasses (wb_mr6c.h, pcf8574.h, ...)
  └── mqtt/              per-module MQTT plumbing: command/command_consumer/state/state_producer
src/color/color.h        header-only, the ONLY module compilable on host
src/automation/          light.cpp — presence/night-mode automation (main light + backlight)
tests/host/              color tests only; single entrypoint run_tests.sh
library.json/.properties PlatformIO/Arduino manifests (deps: ArduinoJSON, PCF8574, GyverFilters)
```

## Core pattern (every runtime module follows it)

- A `*Config` struct with `has*` flags; `using <X>Option = std::function<void(<X>Config&)>`;
  inline option factories `withMQTT / withDiscovery / withTimeout / ...` (see `src/relay/relay.h`).
- `bool init(std::initializer_list<Option>)` applies options; `update()` is called from `loop()`.
- Hardware backends override `protected virtual` `*Internal` hooks (e.g. `setStateInternal`);
  the base class keeps logical state in memory so it is usable without hardware.
- MQTT state is published as JSON; commands are plain `on`/`off` or JSON objects.
  Topics: `{prefix}/{name_slug}/state` and `.../set` (name lowercased, spaces → `_`).
- ESP32-only (`architectures=esp32`); uses FreeRTOS primitives (`xQueueCreate`).

## Dependencies you must supply

Beyond `library.json`, sources include headers from companion libs **not declared** anywhere:
EDMQTT `<mqtt.h>`, EDHA `<discovery.h>`, EDWB `<device/wb_*.h>`, EDConfig `<data_mgr.h>`,
EDUtils `<Utils.h>`, log `<log/log.h>`. Their repos sit as siblings: `../ed-mqtt`, `../ed-ha`,
`../ed-wb`, `../ed-config`, `../ed-utils` (relative to this repo's parent dir).

## Validation (how to actually check your change)

1. Color module: `bash tests/host/run_tests.sh`
   (compiles `tests/host/test_color.cpp` with `-std=gnu++11 -Wall -Wextra -Werror -I src`).
   This is the only test suite in the repo.
2. Everything else: **cannot compile on host** (Arduino + FreeRTOS + companions).
   Real compile check = build a consumer with PlatformIO, e.g.
   `pio run` in `../neptunus` or `../alfred` (both pin ed-common `#0.3.1`).
   State explicitly in your report which path you verified.
3. New/changed behavior otherwise: review against the pattern above; there is no mock harness.

## Impact radius

Edits to `src/` affect all four downstream firmware projects: `../evelyn`, `../newton`,
`../neptunus`, `../alfred` (they consume this repo by git URL/tag, so a pushed tag can break
them — public API changes need extra care and should be called out).

## Conventions

- 4-space indent, Allman braces, `#pragma once`, `EDCommon::<Module>::<Class>` namespaces.
- Functions/classes PascalCase or camelCase as in existing code; private members `_camelCase`.
- Commit messages: single long imperative summary line, no body, no prefixes ("Add …", "Fix …",
  "Gate …"). Tags are bare semver (`0.3.0`, `0.3.1`).
- Version drift to know: HEAD is tagged `0.3.1` but `library.json`/`library.properties`
  still say `0.3.0`. When bumping, sync manifests + README "Notes and status".
- `ANENJI ...modbus.pdf` is an untracked local datasheet; `.DS_Store`/`.vscode` are ignored.
  Do not commit either unless asked.

## Non-goals / don't

- Don't add an umbrella header, examples dir, license, or CI without being asked.
- Don't declare the companion libs in `library.json` — upstream deliberately keeps them out.
- Don't refactor across modules opportunistically; the modules share only the pattern, not code.
