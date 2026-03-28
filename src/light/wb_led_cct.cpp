#include <log/log.h>

#include "mqtt/state_producer.h"
#include "wb_led_cct.h"

bool EDCommon::Light::WBLedCCT::init(uint8_t cctChannel, std::initializer_list<WBLedCCTOption> options)
{
    WBLedCCTConfig config;

    for (auto& opt : options) {
        opt(config);
    }

    EDWB::LEDMode mode;
    switch (cctChannel) {
        case 1:
            mode = EDWB::LED_MODE_CCTWW;
            break;
        case 2:
            mode = EDWB::LED_MODE_2WCCT;
            break;
        default:
            LOGE("init", "unsupported cct channel");
            return false;
    }

    if (!_led->setMode(mode)) {
        LOGE("init", "failed to change WB-LED mode");
        return false;
    }

    if (config.switchChannel > 0) {
        _led->setInputMode(config.switchChannel, true);
        _led->setSafeMode(config.switchChannel, EDWB::SAFE_MODE_DONT_BLOCK_INPUT);
        _led->setInputActionRaw(config.switchChannel, EDWB::INPUT_TYPE_SHORT_CLICK, 0x3007); // switch cct @todo: add support cct2
        _led->setInputActionRaw(config.switchChannel, EDWB::INPUT_TYPE_LONG_CLICK, 0xB008); // change cct brightness @todo: add support cct2
    }

    if (config.hasMQTTSupport) {
        auto stateProducer = new StateProducer(config.mqtt);
        stateProducer->init(config.mqttStateTopic.c_str());
        _mqttStateMgr = new EDUtils::StateMgr<MQTTState>(stateProducer);
    }

    if (config.hasDiscovery && config.hasMQTTSupport) {
        config.discoveryMgr->addLight(
            config.device,
            config.discoveryName,
            config.discoveryObjectID,
            EDUtils::formatString("%s_%s_%s", config.discoveryObjectID, config.controllerName, EDUtils::getChipID())
        )
            ->setCommandTopic(config.mqttCommandTopic)
            ->setStateTopic(config.mqttStateTopic)
            ->setStateValueTemplate("{{ value_json.hallwayLightEnabled }}")
            ->setPayloadOn("stateHallwayLightOn")
            ->setPayloadOff("stateHallwayLightOff")
            ->setBrightnessCommandTopic(config.mqttCommandTopic)
            ->setBrightnessCommandTemplate("{\"hallwayLightBrightness\": {{ value }} }")
            ->setBrightnessStateTopic(config.mqttStateTopic)
            ->setBrightnessValueTemplate("{{ value_json.hallwayLightBrightness }}")
            ->setBrightnessScale(100)
            ->setColorTempKelvin(true)
            ->setColorTempCommandTemplate("{\"hallwayLightTempColor\": {{ value }} }")
            ->setColorTempCommandTopic(config.mqttCommandTopic)
            ->setColorTempStateTopic(config.mqttStateTopic)
            ->setColorTempValueTemplate("{{ value_json.hallwayLightTempColor }}")
            ->setMinKelvin(2700) // @todo: move to options
            ->setMaxKelvin(6000); // @todo: move to options
    }
}

bool EDCommon::Light::WBLedCCT::setState(bool enable)
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

bool EDCommon::Light::WBLedCCT::setBrightness(uint8_t brightness)
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

bool EDCommon::Light::WBLedCCT::setTemperature(uint16_t temperature)
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

void EDCommon::Light::WBLedCCT::update()
{
    _mqttStateMgr->loop();
}
