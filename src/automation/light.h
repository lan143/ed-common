#pragma once

#include <Arduino.h>
#include <data_mgr.h>
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
                EDCommon::BinarySensor::BinarySensor* humanDetector,
                EDCommon::Sensor::Sensor* lightLevel
            ) : _mainLight(mainLight), _backLight(backLight), _humanDetector(humanDetector),
                _lightLevel(lightLevel)
            {
                _commandQueue = xQueueCreate(10, sizeof(bool));
            }

            bool init(std::string stateFileName, std::initializer_list<Option> options);
            void update();

            bool changeNightModeState(bool enable);

        private:
            void changeStateInternal(bool enabled, bool manual);
            void updateLight();
            void publishState();

        private:
            Config _config;
            QueueHandle_t _commandQueue;
            LightState _state;
            bool _manual = false;
            int64_t _lastUpdateTime = 0;
            int64_t _lastHumanDetectTime = 0;
            int64_t _lastManualControlTime = 0;
            int64_t _lastPublishStateTime = 0;
            int64_t _lastStoreStateTime = 0;
            uint64_t _lightLowLevelCount = 0;

        private:
            EDCommon::Light::Light* _mainLight = nullptr;
            EDCommon::Light::Light* _backLight = nullptr;
            EDCommon::BinarySensor::BinarySensor* _humanDetector = nullptr;
            EDCommon::Sensor::Sensor* _lightLevel = nullptr;
            EDConfig::DataMgr<LightState>* _stateMgr = nullptr;
        };
    }
}