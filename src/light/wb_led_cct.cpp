#include <log/log.h>

#include "wb_led_cct.h"

bool EDCommon::Light::WBLedCCT::init(uint8_t cctChannel, uint8_t switchChannel /* = 0 */, std::initializer_list<LightOption> options /* = {} */)
{
    Light::init(options);

    _cctChannel = cctChannel;
    _switchChannel = switchChannel;

    EDWB::LEDMode mode;
    switch (cctChannel) {
        case 1:
            mode = EDWB::LED_MODE_CCTWW;
            break;
        case 2:
            mode = EDWB::LED_MODE_2WCCT;
            break;
        default:
            LOGE("WBLedCCT::init", "unsupported cct channel");
            return false;
    }

    if (!_led->setMode(mode)) {
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

bool EDCommon::Light::WBLedCCT::setStateInternal(bool enable)
{
    auto result = isEnabled();

    if (!result.second) {
        return false;
    }

    if (result.first != enable) {
        bool enableResult = false;

        switch (_cctChannel) {
            case 1:
                return _led->enableCCT1(enable);
            case 2:
                return _led->enableCCT2(enable);
            default:
                LOGE("setStateInternal", "unsupported cct channel");
                return false;
        }
    }

    return true;
}

std::pair<bool, bool> EDCommon::Light::WBLedCCT::isEnabled()
{
    EDWB::Result<bool> result(false, false);

    switch (_cctChannel) {
        case 1:
            result = _led->isEnabledCCT1();
            break;
        case 2:
            result = _led->isEnabledCCT2();
            break;
        default:
            return {false, false};
    }

    return {result._value, result._success};
}

bool EDCommon::Light::WBLedCCT::setBrightnessInternal(uint8_t brightness)
{
    auto result = getBrightness();
    if (!result.second) {
        return false;
    }

    if (result.first != brightness) {
        switch (_cctChannel) {
            case 1:
                return _led->setBrightnessCCT1(brightness);
            case 2:
                return _led->setBrightnessCCT2(brightness);
            default:
                return false;
        }
    }

    return true;
}

std::pair<uint8_t, bool> EDCommon::Light::WBLedCCT::getBrightness()
{
    EDWB::Result<uint8_t> result(false, 0);

    switch (_cctChannel) {
        case 1:
            result = _led->getBrightnessCCT1();
            break;
        case 2:
            result = _led->getBrightnessCCT2();
            break;
        default:
            return {0, false};
    }

    return {result._value, result._success};
}

bool EDCommon::Light::WBLedCCT::setTemperatureInternal(uint16_t temperature)
{
    auto result = getTemperature();
    if (!result.second) {
        return false;
    }

    if (result.first != temperature) {
        auto val = map(temperature, 2700, 6000, 0, 100);

        switch (_cctChannel) {
            case 1:
                return _led->setTemperatureCCT1(val);
            case 2:
                return _led->setTemperatureCCT2(val);
            default:
                return false;
        }
    }

    return true;
}

std::pair<uint16_t, bool> EDCommon::Light::WBLedCCT::getTemperature()
{
    EDWB::Result<uint8_t> result(false, 0);
    switch (_cctChannel) {
        case 1:
            result = _led->getTemperatureCCT1();
            break;
        case 2:
            result = _led->getTemperatureCCT2();
            break;
        default:
            return {0, false};
    }

    return {map(result._value, 0, 100, 2700, 6000), result._success};
}
