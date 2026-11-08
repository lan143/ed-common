#pragma once

#include <device/wb_map12e.h>

#include "./sensor.h"

namespace EDCommon
{
    namespace Sensor
    {
        class WBMAP12EFrequency : public Sensor
        {
        public:
            WBMAP12EFrequency(EDWB::MAP12E* device) : _device(device) {}

        protected:
            bool preInit() override
            {
                _config.unitOfMeasurement = "Hz";
                _config.deviceClass = "frequency";
                _config.sensorStateClass = EDHA::SENSOR_STATE_CLASS_MEASUREMENT;

                return true;
            }

            std::pair<float_t, bool> getValueInternal() override
            {
                return _device->getFrequency();
            }

        private:
            EDWB::MAP12E* _device = nullptr;
        };
    }
}
