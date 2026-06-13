#pragma once

#include <Arduino.h>
#include <discovery.h>
#include <mqtt.h>

namespace EDCommon
{
    namespace Sensor
    {
        struct SensorConfig
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
            std::string unitOfMeasurement;
        };

        using SensorOption = std::function<void(SensorConfig&)>;

        inline SensorOption withMQTT(EDMQTT::MQTT* mqtt, std::string topicPrefix, std::string controllerName, std::string name)
        {
            return [mqtt, topicPrefix, controllerName, name](SensorConfig& c) {
                c.hasMQTTSupport = true;
                c.mqtt = mqtt;
                c.topicPrefix = topicPrefix;
                c.controllerName = controllerName;
                c.name = name;
            };
        }

        inline SensorOption withDiscovery(EDHA::DiscoveryMgr* discoveryMgr, EDHA::Device* device)
        {
            return [discoveryMgr, device](SensorConfig& c) {
                c.hasDiscovery = true;
                c.discoveryMgr = discoveryMgr;
                c.device = device;
            };
        }

        class Sensor
        {
        public:
            bool init(std::initializer_list<SensorOption> options);
            void update();
            std::pair<float_t, bool> getValue() { return _currentValue; }

        protected:
            virtual bool preInit() {}
            virtual bool postInit() {}
            virtual std::pair<float_t, bool> getValueInternal() { return {0.0f, false}; }

        protected:
            SensorConfig _config;

        private:
            std::pair<float_t, bool> _currentValue;
            int64_t _lastUpdateTime = 0;
        };
    }
}
