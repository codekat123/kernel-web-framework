#include "../../include/utils/JsonParser.hpp"

std::unordered_map<std::string, std::string> parseJson(
    const std::string& json
) {
    std::unordered_map<std::string, std::string> result;

    size_t pos = 0;

    while (pos < json.size()) {

        size_t key_start = json.find('"', pos);
        if (key_start == std::string::npos) break;

        size_t key_end = json.find('"', key_start + 1);
        if (key_end == std::string::npos) break;

        std::string key = json.substr(key_start + 1, key_end - key_start - 1);

        size_t colon = json.find(':', key_end + 1);
        if (colon == std::string::npos) break;

        size_t val_start = json.find('"', colon + 1);
        if (val_start == std::string::npos) break;

        size_t val_end = json.find('"', val_start + 1);
        if (val_end == std::string::npos) break;

        std::string value = json.substr(val_start + 1, val_end - val_start - 1);

        result[key] = value;

        pos = val_end + 1;
    }

    return result;
}
