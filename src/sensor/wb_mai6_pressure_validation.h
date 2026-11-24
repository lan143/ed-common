#pragma once

#include <cmath>
#include <cstdint>

// Pure, host-testable validation for raw WB-MAI6 calculated values.
//
// Incident: with no water pressure the WB-MAI6 channel read 0x7FFF, which the
// library scaled to 327.67 bar (32767 * 10 / 1000). That fake high pressure fed
// the bypass/pump automation as if it were real. The saved Wiren Board Modbus
// table documents only Error: 0x7FFF for the 16-bit calculated-value registers
// (0xX504/0xX505, signed int16); 0xFFFF and other high values are rejected
// defensively (they are out of the calibrated range anyway, and 0xFFFF matches
// the original bug report hypothesis). Everything outside the calibrated raw
// range is rejected too. No Arduino/ESP dependencies.
namespace EDCommon
{
    inline bool validateWBMAI6Pressure(
        uint16_t rawValue,
        uint16_t minCalculatedValue,
        uint16_t maxCalculatedValue,
        float_t maxPhysicalValue,
        float_t* physicalValue)
    {
        if (physicalValue == nullptr) {
            return false;
        }

        // Stay deterministic on rejection: callers never observe a stale value.
        *physicalValue = 0.0f;

        // WB-MAI6 channel error sentinels (0x7FFF caused the reported bug).
        if (rawValue == 0x7FFFu || rawValue == 0xFFFFu) {
            return false;
        }

        // Degenerate calibration would divide by zero.
        if (maxCalculatedValue <= minCalculatedValue) {
            return false;
        }

        // Outside the calibrated raw span: either sensor error or over-range.
        if (rawValue < minCalculatedValue || rawValue > maxCalculatedValue) {
            return false;
        }

        const float_t scaled = (static_cast<float_t>(rawValue - minCalculatedValue) * maxPhysicalValue)
            / static_cast<float_t>(maxCalculatedValue - minCalculatedValue);

        // Defensive: NaN/inf or an out-of-domain physical result is invalid.
        if (!(scaled >= 0.0f && scaled <= maxPhysicalValue)) {
            return false;
        }

        *physicalValue = scaled;
        return true;
    }
} // namespace EDCommon
