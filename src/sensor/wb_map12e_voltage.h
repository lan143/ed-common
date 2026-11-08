#pragma once

#include <device/wb_map12e.h>

#include "./sensor.h"

namespace EDCommon
{
    namespace Sensor
    {
        class WBMAP12EVoltage : public Sensor
        {
        public:
            WBMAP12EVoltage(EDWB::MAP12E* device, EDWB::MAP12EPhase phase) : _device(device), _phase(phase) {}

        protected:
            bool preInit() override
            {
                _config.unitOfMeasurement = "V";
                _config.deviceClass = "voltage";
                _config.sensorStateClass = EDHA::SENSOR_STATE_CLASS_MEASUREMENT;

                return true;
            }

            std::pair<float_t, bool> getValueInternal() override
            {
                return _device->getVoltage(_phase);
            }

        private:
            EDWB::MAP12E* _device = nullptr;
            EDWB::MAP12EPhase _phase = EDWB::MAP12E_PHASE_L1;
        };
    }
}
