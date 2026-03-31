#pragma once

#include <device/wb_led.h>
#include <discovery.h>
#include <state/state_mgr.h>

#include "light.h"
#include "mqtt/state.h"

namespace EDCommon
{
    namespace Light
    {
        struct WBLedCCTConfig
        {
            uint8_t switchChannel = 0;

            bool hasMQTTSupport = false;
            EDMQTT::MQTT* mqtt = nullptr;
            std::string controllerName;
            std::string name;
            std::string mqttCommandTopic;
            std::string mqttStateTopic;

            bool hasDiscovery = false;
            EDHA::DiscoveryMgr* discoveryMgr = nullptr;
            EDHA::Device* device = nullptr;

            uint16_t minTemperature = 2700;
            uint16_t maxTemperature = 6000;
        };

        using WBLedCCTOption = std::function<void(WBLedCCTConfig&)>;

        WBLedCCTOption withSwitchChannel(uint8_t channel)
        {
            return [channel](WBLedCCTConfig& c) { c.switchChannel = channel; };
        }

        WBLedCCTOption withMQTT(EDMQTT::MQTT* mqtt, std::string controllerName, std::string name)
        {
            return [mqtt, controllerName, name](WBLedCCTConfig& c) {
                c.hasMQTTSupport = true;
                c.mqtt = mqtt;
                c.controllerName = controllerName;
                c.name = name;
            };
        }

        WBLedCCTOption withDiscovery(EDHA::DiscoveryMgr* discoveryMgr, EDHA::Device* device)
        {
            return [discoveryMgr, device](WBLedCCTConfig& c) {
                c.hasDiscovery = true;
                c.discoveryMgr = discoveryMgr;
                c.device = device;
            };
        }

        WBLedCCTOption withTemperature(uint16_t minTemperature, uint16_t maxTemperature)
        {
            return [minTemperature, maxTemperature](WBLedCCTConfig& c) {
                c.minTemperature = minTemperature;
                c.maxTemperature = maxTemperature;
            };
        }

        class WBLedCCT : public Light, public Dimmer, public ColorTemperatureSetter
        {
        public:
            WBLedCCT(EDWB::LED* led) : _led(led) {}
            bool init(uint8_t cctChannel, std::initializer_list<WBLedCCTOption> options);

            bool setState(bool enable);
            std::pair<bool, bool> isEnabled();

            bool setBrightness(uint8_t brightness);
            std::pair<uint8_t, bool> getBrightness();

            bool setTemperature(uint16_t temperature);
            std::pair<uint16_t, bool> getTemperature();

            void update();

        private:
            WBLedCCTConfig _config; 
            uint8_t _cctChannel;

        private:
            EDWB::LED* _led = nullptr;
            EDUtils::StateMgr<MQTTState>* _mqttStateMgr = nullptr;
        };
    }
}
