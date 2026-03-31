#include <algorithm>
#include <log/log.h>

#include "mqtt/state_producer.h"
#include "mqtt/command_consumer.h"
#include "wb_led_cct.h"

bool EDCommon::Light::WBLedCCT::init(uint8_t cctChannel, std::initializer_list<WBLedCCTOption> options)
{
    _cctChannel = cctChannel;

    for (auto& opt : options) {
        opt(_config);
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
            LOGE("WBLedCCT::init", "unsupported cct channel");
            return false;
    }

    if (!_led->setMode(mode)) {
        LOGE("WBLedCCT::init", "failed to change WB-LED mode");
        return false;
    }

    if (_config.switchChannel > 0) {
        if (!_led->setInputMode(_config.switchChannel, true)) {
            LOGE("WBLedCCT::init", "failed to set input mode");
            return false;
        }

        if (!_led->setSafeMode(_config.switchChannel, EDWB::SAFE_MODE_DONT_BLOCK_INPUT)) {
            LOGE("WBLedCCT::init", "failed to set safe mode");
            return false;
        }

        if (!_led->setInputActionRaw(_config.switchChannel, EDWB::INPUT_TYPE_SHORT_CLICK, 0x3007)) { // switch cct @todo: add support cct2
            LOGE("WBLedCCT::init", "failed to set input action raw");
            return false;
        }

        if (!_led->setInputActionRaw(_config.switchChannel, EDWB::INPUT_TYPE_LONG_CLICK, 0xB008)) { // change cct brightness @todo: add support cct2
            LOGE("WBLedCCT::init", "failed to set input action raw");
            return false;
        }
    }

    if (_config.hasMQTTSupport) {
        char mqttStateTopic[64] = {0};
        char mqttCommandTopic[64] = {0};
        std::string controllerName = _config.controllerName;
        std::string name = _config.name;

        std::replace(controllerName.begin(), controllerName.end(), ' ', '_');
        std::transform(controllerName.begin(), controllerName.end(), controllerName.begin(), [](unsigned char c) {
            return std::tolower(c);
        });

        std::replace(name.begin(), name.end(), ' ', '_');
        std::transform(name.begin(), name.end(), name.begin(), [](unsigned char c) {
            return std::tolower(c);
        });

        snprintf(mqttStateTopic, 64, "%s/%s/%s/state", controllerName, EDUtils::getChipID(), name);
        snprintf(mqttCommandTopic, 64, "%s/%s/%s/set", controllerName, EDUtils::getChipID(), name);

        _config.mqttStateTopic = mqttStateTopic;
        _config.mqttCommandTopic = mqttCommandTopic;

        LOGD("WBLedCCT::init", "command topic: %s, state topic: %s", _config.mqttStateTopic.c_str(), _config.mqttCommandTopic.c_str());

        auto stateProducer = new StateProducer(_config.mqtt);
        stateProducer->init(_config.mqttStateTopic.c_str());
        _mqttStateMgr = new EDUtils::StateMgr<MQTTState>(stateProducer);

        auto commandConsumer = new MQTTCommandConsumer(this);
        commandConsumer->init(_config.mqttCommandTopic.c_str());
        _config.mqtt->subscribe(commandConsumer);
    }

    if (_config.hasDiscovery && _config.hasMQTTSupport) {
        std::string discoveryObjectID = _config.name;
        std::string controllerName = _config.controllerName;

        std::replace(controllerName.begin(), controllerName.end(), ' ', '_');
        std::transform(controllerName.begin(), controllerName.end(), controllerName.begin(), [](unsigned char c) {
            return std::tolower(c);
        });

        std::replace(discoveryObjectID.begin(), discoveryObjectID.end(), ' ', '_');
        std::transform(discoveryObjectID.begin(), discoveryObjectID.end(), discoveryObjectID.begin(), [](unsigned char c) {
            return std::tolower(c);
        });

        std::string uniqueID = EDUtils::formatString("%s_%s_%s", discoveryObjectID, controllerName, EDUtils::getChipID());

        _config.discoveryMgr->addLight(
            _config.device,
            _config.name,
            discoveryObjectID,
            uniqueID
        )
            ->setCommandTopic(_config.mqttCommandTopic)
            ->setStateTopic(_config.mqttStateTopic)
            ->setStateValueTemplate("{{ value_json.enabled }}")
            ->setPayloadOn("on")
            ->setPayloadOff("off")
            ->setBrightnessCommandTopic(_config.mqttCommandTopic)
            ->setBrightnessCommandTemplate("{\"brightness\": {{ value }} }")
            ->setBrightnessStateTopic(_config.mqttStateTopic)
            ->setBrightnessValueTemplate("{{ value_json.brightness }}")
            ->setBrightnessScale(100)
            ->setColorTempKelvin(true)
            ->setColorTempCommandTemplate("{\"tempColor\": {{ value }} }")
            ->setColorTempCommandTopic(_config.mqttCommandTopic)
            ->setColorTempStateTopic(_config.mqttStateTopic)
            ->setColorTempValueTemplate("{{ value_json.tempColor }}")
            ->setMinKelvin(2700) // @todo: move to options
            ->setMaxKelvin(6000); // @todo: move to options
    }

    return true;
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
