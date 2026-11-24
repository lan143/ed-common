// Host-only scenarios for src/sensor/wb_mai6_pressure_validation.h.
//
// Build and run via the suite entrypoint:
//   bash tests/host/run_tests.sh
//
// This file lives outside src/ so PlatformIO never compiles it into firmware.

#include <cassert>
#include <cmath>
#include <cstdint>
#include <cstdio>

#include "../src/sensor/wb_mai6_pressure_validation.h"

static bool isValid(uint16_t raw, uint16_t min, uint16_t max, float* out)
{
    return EDCommon::validateWBMAI6Pressure(raw, min, max, 10.0f, out);
}

static void testErrorSentinel0x7FFF()
{
    // Regression for the incident: 0x7FFF used to be published as 327.67 bar.
    float value = 123.0f;
    assert(!isValid(0x7FFF, 0, 1000, &value));
    assert(value == 0.0f);
}

static void testErrorSentinel0xFFFF()
{
    float value = 123.0f;
    assert(!isValid(0xFFFF, 0, 1000, &value));
    assert(value == 0.0f);
}

static void testZeroIsValid()
{
    float value = -1.0f;
    assert(isValid(0, 0, 1000, &value));
    assert(value == 0.0f);
}

static void testMaxBoundaryIsValid()
{
    float value = 0.0f;
    assert(isValid(1000, 0, 1000, &value));
    assert(std::fabs(value - 10.0f) < 0.000001f);
}

static void testAboveMaxIsInvalid()
{
    float value = 123.0f;
    assert(!isValid(1001, 0, 1000, &value));
    assert(value == 0.0f);
}

static void testRaw32767IsInvalid()
{
    float value = 123.0f;
    assert(!isValid(32767, 0, 1000, &value));
    assert(value == 0.0f);
}

static void testDegenerateCalibrationIsInvalid()
{
    float value = 123.0f;
    assert(!isValid(1000, 1000, 1000, &value));
    assert(value == 0.0f);
}

static void testMidValueIsValid()
{
    float value = 0.0f;
    assert(isValid(500, 0, 1000, &value));
    assert(std::fabs(value - 5.0f) < 0.000001f);
}

static void testBelowMinIsInvalid()
{
    float value = 123.0f;
    assert(!isValid(99, 100, 1000, &value));
    assert(value == 0.0f);
}

static void testMinBoundaryWithOffsetIsValid()
{
    float value = -1.0f;
    assert(isValid(100, 100, 1000, &value));
    assert(value == 0.0f);
}

static void testSentinelIsolatedWithinWideCalibration()
{
    // With max=0xFFFF the range check alone would accept 0x7FFF, so an
    // invalid result here isolates the explicit sentinel branch.
    float value = 123.0f;
    assert(!isValid(0x7FFF, 0, 0xFFFF, &value));
    assert(value == 0.0f);

    // Positive control: the same wide calibration accepts a near-sentinel
    // raw value, proving only the sentinel branch rejected 0x7FFF.
    assert(isValid(0x7FFE, 0, 0xFFFF, &value));
    assert(std::fabs(value - (0x7FFE * 10.0f / 0xFFFF)) < 0.000001f);
}

static void testNegativeWraparoundIsInvalid()
{
    // 0xFFFB is signed -5 in the vendor's int16 format, read as unsigned
    // by the library; it must not wrap around into a valid reading.
    float value = 123.0f;
    assert(!isValid(0xFFFB, 0, 1000, &value));
    assert(value == 0.0f);
}

int main()
{
    testErrorSentinel0x7FFF();
    testErrorSentinel0xFFFF();
    testZeroIsValid();
    testMaxBoundaryIsValid();
    testAboveMaxIsInvalid();
    testRaw32767IsInvalid();
    testDegenerateCalibrationIsInvalid();
    testMidValueIsValid();
    testBelowMinIsInvalid();
    testMinBoundaryWithOffsetIsValid();
    testSentinelIsolatedWithinWideCalibration();
    testNegativeWraparoundIsInvalid();

    std::printf("wb_mai6_pressure_validation scenarios passed: 12\n");
    return 0;
}
