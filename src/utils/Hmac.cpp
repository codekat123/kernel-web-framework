#include "../../include/utils/Hmac.hpp"
#include "../../include/utils/Base64.hpp"
#include <openssl/hmac.h>
#include <openssl/evp.h>

std::string Hmac::sha256(const std::string& message, const std::string& secret) {
    unsigned char digest[EVP_MAX_MD_SIZE];
    unsigned int digest_len = 0;

    HMAC(
        EVP_sha256(),
        secret.c_str(),
        secret.size(),
        (const unsigned char*)message.c_str(),
        message.size(),
        digest,
        &digest_len
    );

    return Base64::encode(digest, digest_len);
}
