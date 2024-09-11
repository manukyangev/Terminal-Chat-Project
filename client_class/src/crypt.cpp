#include <openssl/rand.h>
#include <cstring>
#include <iostream>
#include <iomanip>
#include <openssl/hmac.h>
#include <openssl/md5.h>
#include <sstream>
std::string hmac_md5(const std::string& data) {
    const std::string& key = "TerminalChat";
    unsigned char* digest;
    unsigned int len = MD5_DIGEST_LENGTH;

    digest = HMAC(EVP_md5(), key.c_str(), key.length(), (unsigned char*)data.c_str(), data.length(), nullptr, nullptr);

    std::ostringstream oss;
    for (unsigned int i = 0; i < len; ++i) {
        oss << std::hex << std::setw(2) << std::setfill('0') << (int)digest[i];
    }

    return oss.str();
}


