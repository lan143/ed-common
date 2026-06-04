#pragma once

#include <Arduino.h>
#include <FastLED.h>
#include <discovery.h>
#include <mqtt.h>

#include "light/mqtt/state_producer.h"
#include "light/mqtt/state.h"

namespace EDCommon
{
    namespace Light
    {
        struct LightCommand
        {
            std::pair<bool, bool> enable;
            std::pair<uint8_t, bool> brightness;
            std::pair<uint16_t, bool> tempColor;
            std::pair<CRGB, bool> color;
        };

        struct LightConfig
        {
            bool hasMQTTSupport = false;
            EDMQTT::MQTT* mqtt = nullptr;
            std::string topicPrefix;
            std::string controllerName;
            std::string name;
            std::string mqttCommandTopic;
            std::string mqttStateTopic;

            bool hasDiscovery = false;
            EDHA::DiscoveryMgr* discoveryMgr = nullptr;
            EDHA::Device* device = nullptr;
        };

        using LightOption = std::function<void(LightConfig&)>;

        inline LightOption withMQTT(EDMQTT::MQTT* mqtt, std::string topicPrefix, std::string controllerName, std::string name)
        {
            return [mqtt, topicPrefix, controllerName, name](LightConfig& c) {
                c.hasMQTTSupport = true;
                c.mqtt = mqtt;
                c.topicPrefix = topicPrefix;
                c.controllerName = controllerName;
                c.name = name;
            };
        }

        inline LightOption withDiscovery(EDHA::DiscoveryMgr* discoveryMgr, EDHA::Device* device)
        {
            return [discoveryMgr, device](LightConfig& c) {
                c.hasDiscovery = true;
                c.discoveryMgr = discoveryMgr;
                c.device = device;
            };
        }

        class Light
        {
        public:
            Light()
            {
                _commandQueue = xQueueCreate(10, sizeof(LightCommand));
            }

            bool init(std::initializer_list<LightOption> options);

            void update();

            bool setState(bool enable);
            virtual std::pair<bool, bool> isEnabled() { return {false, false}; }

            bool setBrightness(uint8_t brightness);
            virtual std::pair<uint8_t, bool> getBrightness() { return {0, false}; }

            bool setTemperature(uint16_t temperature);
            virtual std::pair<uint16_t, bool> getTemperature() { return {0, false}; }

            bool setColor(CRGB color);
            virtual std::pair<CRGB, bool> getColor() { return {0, false}; }

            virtual bool hasBrightnessControl() const { return false; };
            virtual bool hasTemperatureControl() const { return false; };
            virtual bool hasColorControl() const { return false; };

        protected:
            virtual bool setStateInternal(bool enable) {}
            virtual bool setBrightnessInternal(uint8_t brightness) {}
            virtual bool setTemperatureInternal(uint16_t temperature) {}
            virtual bool setColorInternal(CRGB color) {}

        private:
            bool publishState();

        private:
            LightConfig _config;
            MQTTState _mqttState;
            QueueHandle_t _commandQueue;
            int64_t _lastPublishStateTime = 0;

        private:
            StateProducer* _stateProducer = nullptr;
        };
    }
}
