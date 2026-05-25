#pragma once

#include <Arduino.h>
#include <device/wb_mr6c.h>
#include <discovery.h>
#include <mqtt.h>

namespace EDCommon
{
    namespace BinarySensor
    {
        struct WBMR6CConfig
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
        };

        using WBMR6COption = std::function<void(WBMR6CConfig&)>;

        inline WBMR6COption withMQTT(EDMQTT::MQTT* mqtt, std::string topicPrefix, std::string controllerName, std::string name)
        {
            return [mqtt, topicPrefix, controllerName, name](WBMR6CConfig& c) {
                c.hasMQTTSupport = true;
                c.mqtt = mqtt;
                c.topicPrefix = topicPrefix;
                c.controllerName = controllerName;
                c.name = name;
            };
        }

        inline WBMR6COption withDiscovery(EDHA::DiscoveryMgr* discoveryMgr, EDHA::Device* device, EDHA::DeviceClass* deviceClass)
        {
            return [discoveryMgr, device, deviceClass](WBMR6CConfig& c) {
                c.hasDiscovery = true;
                c.discoveryMgr = discoveryMgr;
                c.device = device;
                c.deviceClass = deviceClass;
            };
        }

        class WBMR6C
        {
        public:
            WBMR6C(EDWB::MR6C* mr6c) : _mr6c(mr6c) {}
            bool init(uint8_t channel, std::initializer_list<WBMR6COption> options);
            void update();
            std::pair<bool, bool> getContact() { return _contact; }

        private:
            EDWB::MR6C* _mr6c = nullptr;

        private:
            WBMR6CConfig _config;
            int64_t _lastUpdateTime = 0;
            std::pair<bool, bool> _contact = {false, false};
        };
    }
}
