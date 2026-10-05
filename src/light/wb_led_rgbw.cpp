#include <log/log.h>

#include "wb_led_rgbw.h"

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

bool EDCommon::Light::WBLedRGBW::setStateInternal(bool enable)
{
    auto result = isEnabled();

    if (!result.second) {
        return false;
    }

    if (result.first != enable) {
        return _led->enableRGB(enable);
    }

    return true;
}

std::pair<bool, bool> EDCommon::Light::WBLedRGBW::isEnabled()
{
    EDWB::Result<bool> result(false, false);

    result = _led->isEnabledRGB();

    return {result._value, result._success};
}

bool EDCommon::Light::WBLedRGBW::setBrightnessInternal(uint8_t brightness)
{
    auto result = getBrightness();
    if (!result.second) {
        return false;
    }

    auto colorResult = _led->getRGBColor();
    if (!colorResult._success) {
        return false;
    }

    CRGB newColor = colorResult._value;
    uint8_t brightness = map(constrain(brightness, 0, 100), 0, 100, 0, 255);
    newColor.nscale8_video(brightness);
    CHSV hsv = rgb2hsv_approximate(newColor);

    float S = hsv.s / 255.0f;
    float V = hsv.v / 255.0f;
    float Wf = V * (1.0f - S);

    if (_led->setChannelBrightness(4, (uint8_t)(Wf * 100.0f + 0.5f))) {
        return false;
    }

    if (!_led->enableChannel(4, true)) {
        return false;
    }

    float vColor = V * S;
    CHSV hsvColor(hsv.h, 255, (uint8_t)(vColor * 255.0f + 0.5f));

    CRGB rgbColor;
    hsv2rgb_rainbow(hsvColor, rgbColor);

    if (!_led->setRGBColor(rgbColor.as_uint32_t())) {
        return false;
    }

    if (!_led->enableRGB(true)) {
        return false;
    }

    _brightness = brightness;
}

std::pair<uint8_t, bool> EDCommon::Light::WBLedRGBW::getBrightness()
{
    return {_brightness, true};
}

bool EDCommon::Light::WBLedRGBW::setColorInternal(CRGB color)
{
    return _led->setRGBColor(color.as_uint32_t());
}

std::pair<CRGB, bool> EDCommon::Light::WBLedRGBW::getColor()
{
    auto result = _led->getRGBColor();
    if (!result._success) {
        return {0, false};
    }

    return {result._value, true};
}
