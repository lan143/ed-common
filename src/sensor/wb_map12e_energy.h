#pragma once

#include <device/wb_map12e.h>

#include "./sensor.h"

namespace EDCommon
{
    namespace Sensor
    {
        // Published value = (*syncValue or 0) + (deviceEnergy - (*meterBase or 0)).
        // WB-MAP12E energy registers are read-only and never resettable, so this
        // software offset emulates a counter reset: syncValue is the supplier meter
        // reading captured at sync time, meterBase is the device counter at that
        // moment. Both pointers must point to stable storage (e.g. persisted Config)
        // that outlives the sensor. Float precision degrades above ~10^6 kWh.
        class WBMAP12EEnergy : public Sensor
        {
        public:
            WBMAP12EEnergy(
                EDWB::MAP12E* device,
                uint8_t channel,
                EDWB::MAP12EPhase phase,
                const float_t* syncValue = nullptr,
                const float_t* meterBase = nullptr
            )
                : _device(device)
                , _channel(channel)
                , _phase(phase)
                , _syncValue(syncValue)
                , _meterBase(meterBase)
            {
            }

        protected:
            bool preInit() override
            {
                _config.unitOfMeasurement = "kWh";
                _config.deviceClass = "energy";
                _config.sensorStateClass = EDHA::SENSOR_STATE_CLASS_TOTAL_INCREASING;

                return true;
            }

            std::pair<float_t, bool> getValueInternal() override
            {
                auto energy = _device->getForwardActiveEnergy(_channel, _phase);
                if (!energy.second) {
                    return {0.0f, false};
                }

                float_t base = _meterBase ? *_meterBase : 0.0f;
                float_t sync = _syncValue ? *_syncValue : 0.0f;

                return {sync + (energy.first - base), true};
            }

        private:
            EDWB::MAP12E* _device = nullptr;
            uint8_t _channel = 0;
            EDWB::MAP12EPhase _phase = EDWB::MAP12E_PHASE_L1;
            const float_t* _syncValue = nullptr;
            const float_t* _meterBase = nullptr;
        };
    }
}
