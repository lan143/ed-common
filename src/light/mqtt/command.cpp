#include <ArduinoJson.h>
#include <Json.h>
#include <ExtStrings.h>
#include "command.h"

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
            const char* color = root[F("color")].as<const char*>();
            std::vector<std::string> rgb = EDUtils::split(color, ",");

            if (rgb.size() == 3) {
                bool hasColor = true;
                CRGB color = 0;

                for (int i = 0; i < 3; i++) {
                    int c = 0;
                    if (EDUtils::str2int(&c, rgb[i].c_str(), 10) != EDUtils::STR2INT_SUCCESS) {
                        hasColor = false;
                        break;
                    }

                    color = color.as_uint32_t() | (c << (16 - i * 8));
                }

                if (hasColor) {
                    _color = {color, true};
                }
            }
        }

        return true;
    });
}
