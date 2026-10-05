#include <ArduinoJson.h>
#include <Json.h>
#include <ExtStrings.h>
#include "command.h"

static bool parseColorString(const char* value, std::pair<CRGB, bool>& out)
{
    std::vector<std::string> rgb = EDUtils::split(value, ",");

    if (rgb.size() != 3) {
        return false;
    }

    CRGB color = 0;

    for (int i = 0; i < 3; i++) {
        int c = 0;
        if (EDUtils::str2int(&c, rgb[i].c_str(), 10) != EDUtils::STR2INT_SUCCESS) {
            return false;
        }

        color = color.as_uint32_t() | (c << (16 - i * 8));
    }

    out = {color, true};
    return true;
}

bool EDCommon::Light::MQTTCommand::unmarshalJSON(const char* data)
{
    return EDUtils::parseJson(data, [this](JsonObject root) {
        if (root.containsKey(F("brightness"))) {
            _brightness = {root[F("brightness")].as<uint8_t>(), true};
        }

        if (root.containsKey(F("tempColor"))) {
            _tempColor = {root[F("tempColor")].as<uint16_t>(), true};
        }

        if (root.containsKey(F("color"))) {
            parseColorString(root[F("color")].as<const char*>(), _color);
        } else if (root.containsKey(F("lightColor"))) {
            parseColorString(root[F("lightColor")].as<const char*>(), _color);
        }

        return true;
    });
}
