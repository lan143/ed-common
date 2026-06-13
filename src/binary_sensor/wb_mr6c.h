#pragma once

#include <Arduino.h>
#include <device/wb_mr6c.h>

#include "./binary_sensor.h"

namespace EDCommon
{
    namespace BinarySensor
    {
        class WBMR6C : public BinarySensor
        {
        public:
            WBMR6C(uint8_t channel, EDHA::DeviceClass* deviceClass, bool reverse, EDWB::MR6C* mr6c) : _deviceClass(deviceClass), _reverse(reverse), _mr6c(mr6c) {}

        protected:
            bool preInit() override
            {
                _config.deviceClass = _deviceClass;
                _config.reverse = _reverse;
                _config.channel = _channel;

                return true;
            }

            std::pair<bool, bool> isActiveInternal()
            {
                return _mr6c->getInputChannelState(_config.channel);
            }

        private:
            EDWB::MR6C* _mr6c = nullptr;
            EDHA::DeviceClass* _deviceClass;
            bool _reverse = false;
            uint8_t _channel = 0;
        };
    }
}
