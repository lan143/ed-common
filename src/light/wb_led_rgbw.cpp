#include <math.h>

#include <log/log.h>

#include "wb_led_rgbw.h"

static constexpr float WHITE_SATURATION_CUTOFF = 0.6f; // whites fully off when saturation >= this

CRGB EDCommon::Light::WBLedRGBW::kelvinToRGB(uint16_t kelvin)
{
    float t = (float)constrain((int)kelvin, 1000, 40000) / 100.0f;

    float r = (t <= 66.0f) ? 255.0f : 329.698727446f * powf(t - 60.0f, -0.1332047592f);

    float g;
    if (t <= 66.0f) {
        g = 99.4708025861f * logf(t) - 161.1195681661f;
    } else {
        g = 288.1221695283f * powf(t - 60.0f, -0.0755148492f);
    }

    float b;
    if (t >= 66.0f) {
        b = 255.0f;
    } else if (t <= 19.0f) {
        b = 0.0f;
    } else {
        b = 138.5177312231f * logf(t - 10.0f) - 305.0447927307f;
    }

    return CRGB((uint8_t)constrain((int)(r + 0.5f), 0, 255),
                (uint8_t)constrain((int)(g + 0.5f), 0, 255),
                (uint8_t)constrain((int)(b + 0.5f), 0, 255));
}

bool EDCommon::Light::WBLedRGBW::init(uint8_t switchChannel /* = 0 */, std::initializer_list<LightOption> options /* = {} */)
{
    Light::init(options);

    _switchChannel = switchChannel;

    if (!_led->setMode(EDWB::LED_MODE_RGBW)) {
        LOGE("WBLedCCT::init", "failed to change WB-LED mode");
        return false;
    }

    if (_switchChannel > 0) {
        if (!_led->setInputMode(_switchChannel, true)) {
            LOGE("WBLedCCT::init", "failed to set input mode");
            return false;
        }

        if (!_led->setSafeMode(_switchChannel, EDWB::SAFE_MODE_DONT_BLOCK_INPUT)) {
            LOGE("WBLedCCT::init", "failed to set safe mode");
            return false;
        }

        if (!_led->setInputActionRaw(_switchChannel, EDWB::INPUT_TYPE_SHORT_CLICK, 0x3007)) { // switch cct @todo: add support cct2
            LOGE("WBLedCCT::init", "failed to set input action raw");
            return false;
        }

        if (!_led->setInputActionRaw(_switchChannel, EDWB::INPUT_TYPE_LONG_CLICK, 0xB008)) { // change cct brightness @todo: add support cct2
            LOGE("WBLedCCT::init", "failed to set input action raw");
            return false;
        }
    }


    return true;
}

bool EDCommon::Light::WBLedRGBW::applyOutput()
{
    uint8_t whiteLevel;
    CRGB rgbOut;

    if (_temperature > 0) {
        CRGB tgt = kelvinToRGB(_temperature);
        CRGB nat = kelvinToRGB(_nativeCCT);

        float tR = tgt.r / 255.0f; float tG = tgt.g / 255.0f; float tB = tgt.b / 255.0f;
        float nR = (nat.r / 255.0f) * _whiteStrength;
        float nG = (nat.g / 255.0f) * _whiteStrength;
        float nB = (nat.b / 255.0f) * _whiteStrength;

        float w = 1.0f;
        if (nR > 0.0f) { w = fminf(w, tR / nR); }
        if (nG > 0.0f) { w = fminf(w, tG / nG); }
        if (nB > 0.0f) { w = fminf(w, tB / nB); }
        w = constrain(w, 0.0f, 1.0f);

        float cR = constrain(tR - w * nR, 0.0f, 1.0f);
        float cG = constrain(tG - w * nG, 0.0f, 1.0f);
        float cB = constrain(tB - w * nB, 0.0f, 1.0f);

        uint8_t B = (uint8_t)constrain(_brightness, 0, 100);
        whiteLevel = (uint8_t)(w * (float)B + 0.5f);
        rgbOut = CRGB((uint8_t)(cR * 255.0f + 0.5f), (uint8_t)(cG * 255.0f + 0.5f), (uint8_t)(cB * 255.0f + 0.5f));
        rgbOut.nscale8_video((uint8_t)map(B, 0, 100, 0, 255));
    } else {
        CRGB scaled = _lastColor;
        scaled.nscale8_video(map(constrain(_brightness, 0, 100), 0, 100, 0, 255));

        CHSV hsv = rgb2hsv_approximate(scaled);
        float S = hsv.s / 255.0f;
        float V = hsv.v / 255.0f;
        float whiteScale = 1.0f - S / WHITE_SATURATION_CUTOFF;
        if (whiteScale < 0.0f) {
            whiteScale = 0.0f;
        }

        whiteLevel = (uint8_t)(V * whiteScale * 100.0f + 0.5f);

        float vColor = V * S;
        CHSV hsvColor(hsv.h, 255, (uint8_t)(vColor * 255.0f + 0.5f));

        hsv2rgb_rainbow(hsvColor, rgbOut);
    }

    if (!_led->setChannelBrightness(4, whiteLevel)) {
        return false;
    }

    auto powered = isEnabled();

    if (!powered.second) {
        return false;
    }

    if (!_led->enableChannel(4, powered.first && whiteLevel > 0)) {
        return false;
    }

    return _led->setRGBColor(rgbOut.as_uint32_t());
}

bool EDCommon::Light::WBLedRGBW::setStateInternal(bool enable)
{
    auto result = isEnabled();

    if (!result.second) {
        return false;
    }

    if (enable) {
        if (result.first) {
            return true;
        }

        if (!_led->enableRGB(true)) {
            return false;
        }

        return applyOutput();
    }

    if (!_led->enableRGB(false)) {
        return false;
    }

    return _led->enableChannel(4, false);
}

std::pair<bool, bool> EDCommon::Light::WBLedRGBW::isEnabled()
{
    EDWB::Result<bool> result(false, false);

    result = _led->isEnabledRGB();

    return {result._value, result._success};
}

bool EDCommon::Light::WBLedRGBW::setBrightnessInternal(uint8_t brightness)
{
    _brightness = constrain(brightness, 0, 100);

    if (!_led->enableRGB(true)) {
        return false;
    }

    return applyOutput();
}

std::pair<uint8_t, bool> EDCommon::Light::WBLedRGBW::getBrightness()
{
    return {_brightness, true};
}

bool EDCommon::Light::WBLedRGBW::setColorInternal(CRGB color)
{
    _temperature = 0;
    _lastColor = color;
    return applyOutput();
}

std::pair<CRGB, bool> EDCommon::Light::WBLedRGBW::getColor()
{
    return {_lastColor, true};
}

bool EDCommon::Light::WBLedRGBW::setTemperatureInternal(uint16_t temperature)
{
    _temperature = (uint16_t)constrain((int)temperature, 2700, 6000);
    _lastColor = kelvinToRGB(_temperature);

    if (!_led->enableRGB(true)) {
        return false;
    }

    return applyOutput();
}

std::pair<uint16_t, bool> EDCommon::Light::WBLedRGBW::getTemperature()
{
    return {_temperature, _temperature > 0};
}
