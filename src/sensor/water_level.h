#pragma once

#include <Arduino.h>
#include <device/qdy30a.h>
#include <discovery.h>
#include <mqtt.h>

namespace EDCommon
{
    namespace Sensor
    {
        struct WaterLevelConfig
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
        };

        using WaterLevelOption = std::function<void(WaterLevelConfig&)>;

        inline WaterLevelOption withMQTT(EDMQTT::MQTT* mqtt, std::string topicPrefix, std::string controllerName, std::string name)
        {
            return [mqtt, topicPrefix, controllerName, name](WaterLevelConfig& c) {
                c.hasMQTTSupport = true;
                c.mqtt = mqtt;
                c.topicPrefix = topicPrefix;
                c.controllerName = controllerName;
                c.name = name;
            };
        }

        inline WaterLevelOption withDiscovery(EDHA::DiscoveryMgr* discoveryMgr, EDHA::Device* device)
        {
            return [discoveryMgr, device](WaterLevelConfig& c) {
                c.hasDiscovery = true;
                c.discoveryMgr = discoveryMgr;
                c.device = device;
            };
        }

        class WaterLevel
        {
        public:
            WaterLevel(EDWB::QDY30A* qdy30a) : _qdy30a(qdy30a) {}
            bool init(std::initializer_list<WaterLevelOption> options);
            void update();
            std::pair<float_t, bool> getLevel() { return _currentValue; }

        private:
            bool loadConstants();
            std::pair<float_t, bool> getWaterLevel();

        private:
            EDWB::QDY30A* _qdy30a = nullptr;

        private:
            WaterLevelConfig _config;
            int32_t _unitOfMeasurement = 0;
            int32_t _dotPosition = 0;
            bool _constantsLoaded = false;
            std::pair<float_t, bool> _currentValue;

            int64_t _lastUpdateTime = 0;
        };
    }
}