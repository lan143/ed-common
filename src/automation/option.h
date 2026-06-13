#pragma once

#include <Arduino.h>
#include <discovery.h>
#include <mqtt.h>

namespace EDCommon
{
    namespace Automation
    {
        struct Config
        {
            bool hasMQTTSupport = false;
            EDMQTT::MQTT* mqtt = nullptr;
            std::string topicPrefix;
            std::string controllerName;
            std::string name;
            std::string mqttStateTopic;
            std::string mqttCommandTopic;

            bool hasDiscovery = false;
            EDHA::DiscoveryMgr* discoveryMgr = nullptr;
            EDHA::Device* device = nullptr;
            EDHA::DeviceClass* deviceClass = nullptr;
        };

        using Option = std::function<void(Config&)>;

        inline Option withMQTT(EDMQTT::MQTT* mqtt, std::string topicPrefix, std::string controllerName)
        {
            return [mqtt, topicPrefix, controllerName](Config& c) {
                c.hasMQTTSupport = true;
                c.mqtt = mqtt;
                c.topicPrefix = topicPrefix;
                c.controllerName = controllerName;
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
    }
}