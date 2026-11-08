#pragma once

#include <device/wb_map12e.h>

#include "./sensor.h"

namespace EDCommon
{
    namespace Sensor
    {
        class WBMAP12ECurrent : public Sensor
        {
        public:
            WBMAP12ECurrent(EDWB::MAP12E* device, uint8_t channel, EDWB::MAP12EPhase phase)
                : _device(device)
                , _channel(channel)
                , _phase(phase)
            {
            }

        protected:
            bool preInit() override
            {
                _config.unitOfMeasurement = "A";
                _config.deviceClass = "current";
                _config.sensorStateClass = EDHA::SENSOR_STATE_CLASS_MEASUREMENT;

                return true;
            }

            std::pair<float_t, bool> getValueInternal() override
            {
                return _device->getCurrent(_channel, _phase);
            }

        private:
            EDWB::MAP12E* _device = nullptr;
            uint8_t _channel = 0;
            EDWB::MAP12EPhase _phase = EDWB::MAP12E_PHASE_L1;
        };
    }
}
