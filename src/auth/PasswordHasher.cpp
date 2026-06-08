#include "../../include/auth/PasswordHasher.hpp"
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <stdexcept>
#include <sstream>
#include <iomanip>

static const int SALT_BYTES = 16;
static const int HASH_BYTES = 32;
static const int ITERATIONS = 10000;

static std::string toHex(const unsigned char* bytes, int len) {
    std::ostringstream oss;
    for (int i = 0; i < len; i++) {
        oss << std::hex
            << std::setw(2)
            << std::setfill('0')
            << (int)bytes[i];
    }
    return oss.str();
}

std::string PasswordHasher::hash(const std::string& password) {
    unsigned char salt[SALT_BYTES];
    unsigned char hash[HASH_BYTES];

    if (RAND_bytes(salt, SALT_BYTES) != 1) {
        throw std::runtime_error("Failed to generate salt");
    }

    int result = PKCS5_PBKDF2_HMAC(
        password.c_str(),
        password.size(),
        salt,
        SALT_BYTES,
        ITERATIONS,
        EVP_sha256(),
        HASH_BYTES,
        hash
    );

    if (result != 1) {
        throw std::runtime_error("PBKDF2 failed");
    }

    return toHex(salt, SALT_BYTES) + "$" + toHex(hash, HASH_BYTES);
}

bool PasswordHasher::verify(const std::string& password, const std::string& stored) {
    size_t separator = stored.find('$');

    if (separator == std::string::npos) {
        return false;
    }

    std::string salt_hex = stored.substr(0, separator);
    std::string hash_hex = stored.substr(separator + 1);

    unsigned char salt[SALT_BYTES];

    for (int i = 0; i < SALT_BYTES; i++) {
        salt[i] = (unsigned char)std::stoi(
            salt_hex.substr(i * 2, 2),
            nullptr,
            16
        );
    }

    unsigned char hash[HASH_BYTES];

    int result = PKCS5_PBKDF2_HMAC(
        password.c_str(),
        password.size(),
        salt,
        SALT_BYTES,
        ITERATIONS,
        EVP_sha256(),
        HASH_BYTES,
        hash
    );

    if (result != 1) {
        return false;
    }

    return toHex(hash, HASH_BYTES) == hash_hex;
}
