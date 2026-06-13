#include "light.h"

bool EDCommon::Automation::Light::init(std::initializer_list<Option> options)
{
    for (auto& opt : options) {
        opt(_config);
    }

    if (_config.hasMQTTSupport) {
        _config.name = "Light night mode";

        char mqttStateTopic[256] = {0};
        char mqttCommandTopic[256] = {0};

        std::string name = _config.name;
        std::replace(name.begin(), name.end(), ' ', '_');
        std::transform(name.begin(), name.end(), name.begin(), [](unsigned char c) {
            return std::tolower(c);
        });

        snprintf(mqttStateTopic, 64, "%s/%s/state", _config.topicPrefix.c_str(), name.c_str());
        snprintf(mqttCommandTopic, 64, "%s/%s/set", _config.topicPrefix.c_str(), name.c_str());

        _config.mqttStateTopic = mqttStateTopic;
        _config.mqttCommandTopic = mqttCommandTopic;
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

        std::string uniqueID = EDUtils::formatString("%s_%s_%s", discoveryObjectID.c_str(), controllerName.c_str(), EDUtils::getChipID());

        _config.discoveryMgr->addSwitch(
            _config.device,
            _config.name,
            discoveryObjectID,
            uniqueID
        )
            ->setCommandTemplate("{{ value }}")
            ->setCommandTopic(_config.mqttCommandTopic)
            ->setStateTopic(_config.mqttStateTopic)
            ->setValueTemplate("{{ value }}")
            ->setPayloadOn("true")
            ->setPayloadOff("false")
            ->setStateOn("true")
            ->setStateOff("false");
    }

    return true;
}

void EDCommon::Automation::Light::update()
{
    if ((_lastUpdateTime + 500000) < esp_timer_get_time()) {
        auto isEnabled = _mainLight->isEnabled();
        auto brightness = _mainLight->getBrightness();

        if (isEnabled.second && _state.enabled != isEnabled.first) {
            changeStateInternal(isEnabled.first, true);
        }

        if (
            !_state.nightMode
            && brightness.second && _state.brightness != brightness.first
        ) {
            _state.brightness = brightness.first;
            updateLight();
        }

        auto isHumanDetected = _humanDetector->isActive();
        if (isHumanDetected.second && isHumanDetected.first) {
            _lastHumanDetectTime = esp_timer_get_time();
        }

        auto lightLevel = _lightLevel->getValue();
        if (lightLevel.second && lightLevel.first < 150.0f) {
            _lightLowLevelCount++;
        } else {
            _lightLowLevelCount = 0;
        }

        // enable light if human detected, manual mode isnt active and light level is low
        if (!_manual) {
            if (isHumanDetected.second) {
                if (isHumanDetected.first && _lightLowLevelCount > 120) {
                    changeStateInternal(true, false);
                } else if (!isHumanDetected.first) {
                    changeStateInternal(false, false);
                }
            }
        }

        _lastUpdateTime = esp_timer_get_time();
    }

    if (
        _manual
        && (
            (_state.enabled && (_lastManualControlTime + 300000000) < esp_timer_get_time()) // manual mode turns off after 5 minutes if the light was turned on manually
            || (!_state.enabled && (_lastHumanDetectTime + 600000000) < esp_timer_get_time()) // manual mode turns off 10 minutes after a person leaves the room and light is disabled
        )
    ) {
        _manual = false;
    }
}

void EDCommon::Automation::Light::changeStateInternal(bool enabled, bool manual)
{
    if (_state.enabled == enabled) {
        return;
    }

    _state.enabled = enabled;
    _manual = manual;

    if (_manual) {
        _lastManualControlTime = esp_timer_get_time();
    }

    updateLight();
}

void EDCommon::Automation::Light::updateLight()
{
    if (_state.nightMode) {
        if (_backLight != nullptr) {
            if (_backLight->hasColorControl()) {
                _backLight->setColor(0xff0000);
            }

            if (_backLight->hasBrightnessControl()) {
                _backLight->setBrightness(10); // 10% of max
            }
        } else {
            if (_mainLight->hasColorControl()) {
                _mainLight->setColor(0xff0000);
            }

            if (_mainLight->hasTemperatureControl()) {
                _mainLight->setTemperature(2700);
            }

            if (_mainLight->hasBrightnessControl()) {
                _mainLight->setBrightness(2); // 2% of max
            }
        }
    } else {
        if (_mainLight->hasTemperatureControl()) {
            _mainLight->setTemperature(_state.temperature);
        }

        if (_mainLight->hasBrightnessControl()) {
            _mainLight->setBrightness(_state.brightness);
        }

        if (_backLight != nullptr) {
            if (_backLight->hasColorControl()) {
                _backLight->setColor(_state.color);
            }

            if (_backLight->hasBrightnessControl()) {
                _backLight->setBrightness(_state.brightness);
            }
        }
    }

    if (_backLight != nullptr) {
        _mainLight->setState(!_state.nightMode ? _state.enabled : false);
        _backLight->setState(_state.enabled);
    } else {
        _mainLight->setState(_state.enabled);
    }
}
