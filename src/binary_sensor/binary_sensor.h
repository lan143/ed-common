#pragma once

#include <Arduino.h>
#include <discovery.h>
#include <mqtt.h>

namespace EDCommon
{
    namespace BinarySensor
    {
        struct Config
        {
            uint8_t channel = 0;

            bool hasMQTTSupport = false;
            EDMQTT::MQTT* mqtt = nullptr;
            std::string topicPrefix;
            std::string controllerName;
            std::string name;
            std::string mqttStateTopic;

            bool hasDiscovery = false;
            EDHA::DiscoveryMgr* discoveryMgr = nullptr;
            EDHA::Device* device = nullptr;
            EDHA::DeviceClass* deviceClass = nullptr;

            bool reverse = false;
        };

        using Option = std::function<void(Config&)>;

        inline Option withMQTT(EDMQTT::MQTT* mqtt, std::string topicPrefix, std::string controllerName, std::string name)
        {
            return [mqtt, topicPrefix, controllerName, name](Config& c) {
                c.hasMQTTSupport = true;
                c.mqtt = mqtt;
                c.topicPrefix = topicPrefix;
                c.controllerName = controllerName;
                c.name = name;
            };
        }

        inline Option withDiscovery(EDHA::DiscoveryMgr* discoveryMgr, EDHA::Device* device)
        {
            return [discoveryMgr, device](Config& c) {
                c.hasDiscovery = true;
                c.discoveryMgr = discoveryMgr;
                c.device = device;
            };
        }

        class BinarySensor
        {
        public:
            bool init(int64_t updateInterval, std::initializer_list<Option> options);
            void update();
            std::pair<bool, bool> isActive() { return _isActive; }

        protected:
            virtual bool preInit() { return true; }
            virtual bool postInit() { return true; }
            virtual std::pair<bool, bool> isActiveInternal() { return {false, false}; }

        protected:
            Config _config;

        private:
            std::pair<bool, bool> _isActive;
            int64_t _lastUpdateTime = 0;
            int64_t _updateInterval = 0;
        };
    }
}
