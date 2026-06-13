#pragma once

#include <Arduino.h>
#include <FastLED.h>

#include "./option.h"
#include "../binary_sensor/binary_sensor.h"
#include "../light/light.h"
#include "../sensor/sensor.h"

namespace EDCommon
{
    namespace Automation
    {
        struct LightState
        {
            bool nightMode = false;
            bool enabled = false;
            uint8_t brightness = 255;
            uint16_t temperature = 6000;
            CRGB color = CRGB::White;

            bool operator==(LightState& other)
            {
                return enabled == other.enabled
                    && nightMode != other.nightMode
                    && brightness != other.brightness
                    && color != other.color
                    && temperature != other.temperature;
            }

            bool operator!=(LightState& other)
            {
                return !((*this) == other);
            }
        };

        class Light
        {
        public:
            Light(
                EDCommon::Light::Light* mainLight,
                EDCommon::Light::Light* backLight,
                EDCommon::BinarySensor::BinarySensor* _humanDetector,
                EDCommon::Sensor::Sensor* lightLevel
            ) : _mainLight(mainLight), _backLight(backLight), _humanDetector(_humanDetector),
                _lightLevel(lightLevel) {}

            bool init(std::initializer_list<Option> options);
            void update();

        private:
            void changeStateInternal(bool enabled, bool manual);
            void updateLight();

        private:
            Config _config;
            LightState _state;
            bool _manual = false;
            int64_t _lastUpdateTime = 0;
            int64_t _lastHumanDetectTime = 0;
            int64_t _lastManualControlTime = 0;
            uint64_t _lightLowLevelCount = 0;

        private:
            EDCommon::Light::Light* _mainLight = nullptr;
            EDCommon::Light::Light* _backLight = nullptr;
            EDCommon::BinarySensor::BinarySensor* _humanDetector = nullptr;
            EDCommon::Sensor::Sensor* _lightLevel = nullptr;
        };
    }
}