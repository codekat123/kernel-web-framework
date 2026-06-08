#include "../../include/utils/Base64.hpp"
#include <string>

static const std::string CHARS =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

std::string Base64::encode(const unsigned char* data, size_t len) {
    std::string result;

    int i = 0;
    unsigned char buf3[3];
    unsigned char buf4[4];

    while (len--) {
        buf3[i++] = *data++;

        if (i == 3) {
            buf4[0] = (buf3[0] & 0xfc) >> 2;
            buf4[1] = ((buf3[0] & 0x03) << 4) + ((buf3[1] & 0xf0) >> 4);
            buf4[2] = ((buf3[1] & 0x0f) << 2) + ((buf3[2] & 0xc0) >> 6);
            buf4[3] = buf3[2] & 0x3f;

            for (int j = 0; j < 4; j++)
                result += CHARS[buf4[j]];

            i = 0;
        }
    }

    if (i > 0) {
        for (int j = i; j < 3; j++)
            buf3[j] = 0;

        buf4[0] = (buf3[0] & 0xfc) >> 2;
        buf4[1] = ((buf3[0] & 0x03) << 4) + ((buf3[1] & 0xf0) >> 4);
        buf4[2] = ((buf3[1] & 0x0f) << 2) + ((buf3[2] & 0xc0) >> 6);

        for (int j = 0; j < i + 1; j++)
            result += CHARS[buf4[j]];
    }

    for (char& c : result) {
        if (c == '+') c = '-';
        if (c == '/') c = '_';
    }

    return result;
}

std::string Base64::decode(const std::string& input) {
    std::string b64 = input;

    for (char& c : b64) {
        if (c == '-') c = '+';
        if (c == '_') c = '/';
    }

    while (b64.size() % 4 != 0)
        b64 += '=';

    std::string result;
    unsigned char buf4[4], buf3[3];
    int i = 0;

    for (char c : b64) {
        if (c == '=')
            break;

        buf4[i++] = (unsigned char)CHARS.find(c);

        if (i == 4) {
            buf3[0] = (buf4[0] << 2) + ((buf4[1] & 0x30) >> 4);
            buf3[1] = ((buf4[1] & 0x0f) << 4) + ((buf4[2] & 0x3c) >> 2);
            buf3[2] = ((buf4[2] & 0x03) << 6) + buf4[3];

            for (int j = 0; j < 3; j++)
                result += buf3[j];

            i = 0;
        }
    }

    if (i > 0) {
        for (int j = i; j < 4; j++)
            buf4[j] = 0;

        buf3[0] = (buf4[0] << 2) + ((buf4[1] & 0x30) >> 4);
        buf3[1] = ((buf4[1] & 0x0f) << 4) + ((buf4[2] & 0x3c) >> 2);

        for (int j = 0; j < i - 1; j++)
            result += buf3[j];
    }

    return result;
}
