#include <log/log.h>

#include "wb_led_rgbw.h"

static constexpr float WHITE_SATURATION_CUTOFF = 0.6f; // whites fully off when saturation >= this

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
    CRGB scaled = _lastColor;
    scaled.nscale8_video(map(constrain(_brightness, 0, 100), 0, 100, 0, 255));

    CHSV hsv = rgb2hsv_approximate(scaled);
    float S = hsv.s / 255.0f;
    float V = hsv.v / 255.0f;
    float whiteScale = 1.0f - S / WHITE_SATURATION_CUTOFF;
    if (whiteScale < 0.0f) {
        whiteScale = 0.0f;
    }

    uint8_t whiteLevel = (uint8_t)(V * whiteScale * 100.0f + 0.5f);

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

    float vColor = V * S;
    CHSV hsvColor(hsv.h, 255, (uint8_t)(vColor * 255.0f + 0.5f));

    CRGB rgbColor;
    hsv2rgb_rainbow(hsvColor, rgbColor);

    return _led->setRGBColor(rgbColor.as_uint32_t());
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
    _lastColor = color;
    return applyOutput();
}

std::pair<CRGB, bool> EDCommon::Light::WBLedRGBW::getColor()
{
    return {_lastColor, true};
}
