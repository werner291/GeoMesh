#include "Crypto.h"

std::array<uint8_t,32> simpleSHA256(const void* input, unsigned long length)
{
    std::array<uint8_t,32> hash;

        SHA256_CTX context;
        if(!SHA256_Init(&context))
            throw std::runtime_error("Error while initializing SHA256 context.");

        if(!SHA256_Update(&context, (unsigned char*)input, length))
            throw std::runtime_error("Error while initializing SHA256 context.");

        if(!SHA256_Final(hash.data(), &context))
            throw std::runtime_error("Error while initializing SHA256 context.");
        
        return hash;
}
