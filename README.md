# EDCommon

Some objects for home automations — an Arduino/ESP32 library with Home Assistant-oriented relays, lights, sensors, binary sensors, pulse counters, and a FastLED-compatible color module.

## Features

- **Relay** — MQTT-controlled relays with Home Assistant discovery, optional timeout, open/closed semantics, and state-change listeners. Backends for Wiren Board MR6C and PCF8574.
- **Light** — CCT and RGBW light control for Wiren Board LED devices, plus a wrapper around `Relay`. Supports on/off, brightness, color temperature, and RGB color over MQTT.
- **Sensor** — MQTT/Home Assistant sensors with optional Kalman filtering. Wrappers for Wiren Board MS/MSW measurements, a QDY30A water-level sensor, and WB MAI6 pressure.
- **Binary sensor** — Wiren Board MR6C digital inputs, MTD262MB occupancy detection, and WB-MSW motion detection.
- **Counter** — pulse water counters with cost scaling and optional persistence.
- **Automation** — presence/light-level/night-mode smart light automation with LittleFS-persisted state.
- **Color** — header-only, FastLED-compatible `CRGB`/`CHSV` port that is bit-for-bit compatible with FastLED 3.10.3 on non-AVR builds.

Runtime modules follow a similar pattern: a config struct (`RelayConfig`, `SensorConfig`, or `Config`), option factories such as `withMQTT` and `withDiscovery`, an `init(...)` call, and an `update()` method called from `loop()`. There is no umbrella header; include the module headers you use directly.

## Supported hardware

ESP32 only (`architectures=esp32`).

| Hardware | Provided by |
| --- | --- |
| Wiren Board MR6C | `Relay::WBMR6C`, `BinarySensor::WBMR6C` |
| Wiren Board MS | `Sensor::WBMS*` |
| Wiren Board MSW | `Sensor::WBMSW*`, `BinarySensor::WBMSW` |
| Wiren Board MAI6 | `Sensor::WBMAI6Pressure` |
| Wiren Board LED | `Light::WBLedCCT`, `Light::WBLedRGBW` |
| PCF8574 I/O expander | `Relay::PCF8574P`, `Counter::PCF8574P` |
| QDY30A water level | `Sensor::WaterLevel` |
| MTD262MB occupancy | `BinarySensor::MTD262MB` |
| DHT22 temperature/humidity | `Sensor::DHT22Temperature`, `Sensor::DHT22Humidity` (Adafruit DHT library) |
| GPIO relay | `Relay::GPIO` |
| GPIO binary sensor | `BinarySensor::GPIO` |

## Installation

### Arduino IDE

Install through Library Manager if the library is listed, or clone it into your Arduino `libraries` directory:

```bash
cd ~/Documents/Arduino/libraries
git clone https://github.com/lan143/ed-common.git
```

Restart the Arduino IDE afterwards and include only the module headers you need, for example `#include <relay/relay.h>`.

### PlatformIO

Add the repository to `lib_deps` in `platformio.ini`:

```ini
lib_deps =
    https://github.com/lan143/ed-common.git
```

`library.json` declares the Arduino framework, the `espressif32` platform, and the dependencies ArduinoJSON, PCF8574, GyverFilters, and the Adafruit DHT libraries.

## Dependencies

### Declared in `library.json`

| Library (manifest name) | Used for |
| --- | --- |
| `ArduinoJSON` | JSON parsing and serialization |
| `PCF8574` | PCF8574 I/O expander driver |
| `GyverFilters` | `GKalman` filter in `Sensor::withKalmanFilter` |
| `DHT sensor library` | DHT22 temperature/humidity driver used by `Sensor::DHT22Sensor` |
| `Adafruit Unified Sensor` | Dependency of the Adafruit DHT library |

### Companion libraries (not declared in `library.json`)

The source includes headers from additional libraries in the author's home-automation project. They are **not** declared in `library.json`, and their repositories and versions are not documented here. You must provide them yourself.

| Library | Headers used | Role |
| --- | --- | --- |
| EDMQTT | `<mqtt.h>`, `<producer.h>`, `<consumer.h>` | MQTT client, producer, and consumer base classes |
| EDHA | `<discovery.h>` | Home Assistant discovery (`EDHA::DiscoveryMgr`, `EDHA::Device`) |
| EDWB | `<device/wb_*.h>` | Wiren Board device drivers (MR6C, MS, MSW, LED, MAI6, QDY30A, MTD262MB) |
| EDConfig | `<data_mgr.h>`, `<storage/littlefs_storage.hpp>` | Persisted state for `Automation::Light` |
| EDUtils | `<Utils.h>`, `<ExtStrings.h>`, `<Json.h>`, `<state/state_producer.h>` | Formatting, JSON, and state-producer helpers |
| log | `<log/log.h>` | `LOG*` macros |

## Quick start

The example below registers a relay with MQTT and Home Assistant discovery. It requires the companion EDMQTT and EDHA libraries; ed-common itself does not declare them.

```cpp
#include <relay/relay.h>

EDCommon::Relay::Relay relay;

// Call after your EDMQTT and EDHA objects have been created.
void setupRelay(EDMQTT::MQTT* mqtt, EDHA::DiscoveryMgr* discovery, EDHA::Device* haDevice)
{
    relay.init({
        EDCommon::Relay::withMQTT(mqtt, "home", "controller", "Pump"),
        EDCommon::Relay::withDiscovery(discovery, haDevice),
    });
}

void loop()
{
    relay.update();
}
```

With the name `Pump`, the topics become `home/pump/state` and `home/pump/set` (names are lowercased and spaces become underscores). The command payload is plain `on` or `off`; the published state is JSON such as `{"enabled":"on"}`.

`EDCommon::Relay::Relay` keeps its logical state in memory, so the example is usable without a hardware backend. Use `EDCommon::Relay::WBMR6C` or `EDCommon::Relay::PCF8574P` to drive physical outputs.

## Modules

### Relay

Headers: `<relay/relay.h>`, `<relay/wb_mr6c.h>`, `<relay/pcf8574.h>`, `<relay/gpio.h>`

`Relay` publishes state as JSON and subscribes to `{prefix}/{name}/set` with `on`/`off` commands. Options: `withMQTT`, `withDiscovery`, `withTimeout`, `withOpenSemantics`, `withStateChangeListener`. Methods: `init`, `setState`, `isEnabled`, `update`.

`WBMR6C` adds `init(channel, switchChannel, options)`; `PCF8574P` adds `init(channel, options)`; `GPIO` drives a plain ESP32 output pin with `init(pin, activeHigh, options)`.

### Light

Headers: `<light/light.h>`, `<light/wb_led_cct.h>`, `<light/wb_led_rgbw.h>`, `<light/relay_light.h>`

`Light` provides `setState`, `setBrightness`, `setTemperature` (Kelvin), and `setColor` (`CRGB`), gated by `hasBrightnessControl`, `hasTemperatureControl`, and `hasColorControl`. Options: `withMQTT`, `withDiscovery`.

`WBLedCCT` uses `init(cctChannel, switchChannel, options)`; `WBLedRGBW` uses `init(switchChannel, options)` and accepts `nativeCCT` and `whiteStrength` constructor arguments. `Light::Relay` wraps an `EDCommon::Relay::Relay` and forwards on/off state.

Light MQTT commands accept plain `on`/`off` or a JSON object with `brightness`, `tempColor`, or `color` (`"r,g,b"`). State is published as JSON and can contain `enabled`, `brightness`, `tempColor`, and `color` fields.

### Sensor

Headers: `<sensor/sensor.h>` plus a device wrapper such as `<sensor/wbms_temperature.h>`

`Sensor::init(precision, updateInterval, options)` and `update()`; `getValue()` returns a value/valid pair. Options: `withMQTT`, `withDiscovery`, `withKalmanFilter`.

Available wrappers:

- MS: `WBMSTemperature`, `WBMSHumidity`, `WBMSLightLevel`, `WBMSOneWireTemperature`
- MSW: `WBMSWTemperature`, `WBMSWHumidity`, `WBMSWLightLevel`, `WBMSWCO2`, `WBMSWAirQuality`
- DHT22: `DHT22Temperature`, `DHT22Humidity`
- `WaterLevel` (QDY30A), `WBMAI6Pressure` (WB MAI6)

The DHT22 wrappers share one driver so a single physical read feeds both entities:

```cpp
#include <sensor/dht22_sensor.h>
#include <sensor/dht22_temperature.h>
#include <sensor/dht22_humidity.h>

EDCommon::Sensor::DHT22Sensor dht22(33);
EDCommon::Sensor::DHT22Temperature temperature(&dht22);
EDCommon::Sensor::DHT22Humidity humidity(&dht22);

void setupSensors(EDMQTT::MQTT* mqtt, EDHA::DiscoveryMgr* discovery, EDHA::Device* haDevice)
{
    dht22.begin();

    temperature.init(1, 60000, {
        EDCommon::Sensor::withMQTT(mqtt, "home", "controller", "Street temperature"),
        EDCommon::Sensor::withDiscovery(discovery, haDevice),
    });
    humidity.init(1, 60000, {
        EDCommon::Sensor::withMQTT(mqtt, "home", "controller", "Street humidity"),
        EDCommon::Sensor::withDiscovery(discovery, haDevice),
    });
}
```

### Binary sensor

Headers: `<binary_sensor/binary_sensor.h>`, `<binary_sensor/wb_mr6c.h>`, `<binary_sensor/mtd262mb.h>`, `<binary_sensor/wb_msw.h>`, `<binary_sensor/gpio.h>`

`BinarySensor::init(updateInterval, options)`, `update()`, `setValue()`, `isActive()`. Options: `withMQTT`, `withDiscovery`.

`WBMR6C` reads an MR6C input channel; `MTD262MB` exposes occupancy from the MTD262MB sensor; `WBMSW` exposes motion detection from the WB-MSW sensor. `GPIO` reads a plain ESP32 input pin: `GPIO(pin, deviceClass, reverse, samples = 1, sampleDelayUs = 20)` samples the pin and reports a majority vote, so `samples = 1` is a single read.

### Counter

Headers: `<counter/counter.h>`, `<counter/pcf8574.h>`

`Counter` counts pulses and reports cubic metres (`m³`) with the `water` device class. Methods: `init`, `update`, `setInitialValue`, `getCurrentValue`, `isFlowOfWaterActive`. Options: `withMQTT`, `withDiscovery`, `withObjectID`, `withCost`, `withPersistence`.

`PCF8574P` reads pulses from a PCF8574 pin. Persistence uses the `CounterPersistence` interface (`load`, `save`, `clear`).

### Automation

Headers: `<automation/light.h>` (`<automation/option.h>` is included by it)

`Automation::Light` combines a main light, an optional back light, and a human detector. It reacts to presence and ambient light level, supports a night mode, and persists state to LittleFS. Methods: `init(stateFileName, options)`, `update`, `changeNightModeState`. Options: `withMQTT`, `withDiscovery`, `withLightLevelSensor`, `withNightMode`.

When MQTT and night mode are enabled, the automation exposes a night-mode switch on `{prefix}/{name}/set` with `true`/`false` payloads.

## Color module

Header: `<color/color.h>`

The color module is header-only and provides `CRGB`, `CHSV`, `hsv2rgb_rainbow`, `rgb2hsv_approximate`, and FastLED-compatible 8-bit helpers. If `<FastLED.h>` is already available, `<color/color.h>` includes it and defines nothing else, so the real FastLED remains in effect. Otherwise a dependency-free port of FastLED 3.10.3 is used, with bit-for-bit compatible conversion routines on non-AVR builds.

Use it directly, for example:

```cpp
#include <color/color.h>

CRGB color = CRGB(255, 0, 0);
CHSV hsv = rgb2hsv_approximate(color);
```

## Testing

Only the color module has host tests. Run them from the repository root:

```bash
bash tests/host/run_tests.sh
```

The script compiles `tests/host/test_color.cpp` with `-std=gnu++11 -Wall -Wextra -Werror -I src` and executes it.

## Notes and status

- Current version: **0.5.0**.
- The `Relay` base class no longer requires EDWB; only Wiren Board-specific backend headers include `<device/...>`.
- This repository does not contain a license file, and no license is stated here.
- There is no `examples/` directory; the snippets in this README are illustrative.
