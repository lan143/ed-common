#pragma once

#include <algorithm>
#include <cctype>
#include <Arduino.h>
#include <discovery.h>
#include <mqtt.h>

#include "./storage.h"

namespace EDCommon
{
    namespace Counter
    {
        struct CounterConfig
        {
            bool hasMQTTSupport = false;
            EDMQTT::MQTT* mqtt = nullptr;
            std::string topicPrefix;
            std::string controllerName;
            std::string name;
            std::string mqttStateTopic;

            bool hasDiscovery = false;
            EDHA::DiscoveryMgr* discoveryMgr = nullptr;
            EDHA::Device* device = nullptr;
            std::string objectID;
            std::string unitOfMeasurement = "m³";
            std::string deviceClass = "water";

            uint32_t cost = 1;

            CounterPersistence* persistence = nullptr;
        };

        using CounterOption = std::function<void(CounterConfig&)>;

        inline CounterOption withMQTT(EDMQTT::MQTT* mqtt, std::string topicPrefix, std::string controllerName, std::string name)
        {
            return [mqtt, topicPrefix, controllerName, name](CounterConfig& c) {
                c.hasMQTTSupport = true;
                c.mqtt = mqtt;
                c.topicPrefix = topicPrefix;
                c.controllerName = controllerName;
                c.name = name;

                std::string sanitized = name;
                std::replace(sanitized.begin(), sanitized.end(), ' ', '_');
                std::transform(sanitized.begin(), sanitized.end(), sanitized.begin(), [](unsigned char ch) {
                    return std::tolower(ch);
                });

                c.mqttStateTopic = topicPrefix + "/" + sanitized + "/state";
            };
        }

        inline CounterOption withDiscovery(EDHA::DiscoveryMgr* discoveryMgr, EDHA::Device* device)
        {
            return [discoveryMgr, device](CounterConfig& c) {
                c.hasDiscovery = true;
                c.discoveryMgr = discoveryMgr;
                c.device = device;
            };
        }

        inline CounterOption withObjectID(std::string objectID)
        {
            return [objectID](CounterConfig& c) {
                c.objectID = objectID;
            };
        }

        inline CounterOption withCost(uint32_t cost)
        {
            return [cost](CounterConfig& c) {
                c.cost = cost;
            };
        }

        inline CounterOption withPersistence(CounterPersistence* persistence)
        {
            return [persistence](CounterConfig& c) {
                c.persistence = persistence;
            };
        }

        class Counter
        {
        public:
            bool init(std::initializer_list<CounterOption> options);
            void update();

            void setInitialValue(float_t value);
            float_t getCurrentValue() const
            {
                return (float_t)_currentValue * (float_t)_config.cost / 1000.0f;
            }
            bool isFlowOfWaterActive() const { return _lastFlowOfWaterActive; }

        protected:
            virtual int readPin() { return HIGH; }

            CounterConfig _config;

        private:
            void publishState();

        private:
            uint32_t _currentValue = 0;
            bool _lock = false;
            bool _ponentialLockUnlock = false;
            uint64_t _lastCheckTime = 0;
            uint64_t _lastFlowOfWaterActiveTime = 0;
            bool _lastFlowOfWaterActive = false;
            bool _dirty = false;
        };
    }
}
