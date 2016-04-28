/*
 * (c) Copyright 2016 Werner Kroneman
 *
 * This file is part of GeoMesh.
 * 
 * GeoMesh is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * GeoMesh is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with GeoMesh.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef GEOMESH_CRYPTO_H
#define GEOMESH_CRYPTO_H

#include <openssl/rsa.h>
#include <openssl/bio.h>
#include <openssl/sha.h>
#include <openssl/evp.h>
#include <openssl/pem.h>

#include <exception>
#include <assert.h>
#include <vector>
#include <array>
#include <memory>
#include <string>

/**
 * Calculate the SHA256 of the {@code input}
 *
 * @param input Pointer to a char buffer containing of at least {@code length}
 *     bytes.
 * @param length The length of the message to be hashed.
 *
 * @param md Buffer of at least 32 bytes to store thedigest in.
 */
std::array<uint8_t,32> simpleSHA256(const void* input, unsigned long length);

const int RSA_BITS = 2048;
const int RSA_EXPONENT = 65537;
const int HASH_ALGO = NID_sha256;

/**
 * Simplified interface to the OpenSSL RSA struct.
 */
class KeyPair {
    RSA* rsa;
    BIGNUM* ex;

    // Copy constructor banned due to problematic copying of internal C structs.
    // Need to look into how to make this work.
    KeyPair(const KeyPair& kp);
    KeyPair& operator=(const KeyPair);

    public:

    KeyPair() {
      rsa = RSA_new();
      ex = BN_new();
      BN_set_word(ex,RSA_EXPONENT);
    }

    ~KeyPair() {
      RSA_free(rsa);  
      BN_clear_free(ex);
    }

    std::vector<uint8_t> sign(const uint8_t* toSign, size_t length) {

        auto digest = simpleSHA256(toSign, length);

        std::vector<uint8_t> signature(RSA_size(rsa));

        unsigned int sigLength = 0;

        int result = RSA_sign(HASH_ALGO, digest.data(), 32, signature.data(), &sigLength, rsa);

        assert(result == 1);

        signature.resize(sigLength);

        return signature;
    }

    bool verify(const uint8_t* toVerify, size_t length, const uint8_t* signature, size_t sigLength) {
        
        auto digest = simpleSHA256(toVerify, length);

        return RSA_verify(HASH_ALGO, digest.data(), 32, signature, sigLength, rsa) == 1;

    }

    std::vector<uint8_t> getPublicKeyAsBytes() const {
        
        std::vector<uint8_t> buffer(BN_num_bytes(rsa->n));

        BN_bn2bin(rsa->n, buffer.data());

        return buffer;
    }

    /**
     * Generate a new RSA keypair
     *
     * @param length The key length in bits. 2048
     */
    static std::unique_ptr<KeyPair> generateNewKeypair(int length = RSA_BITS) {

        std::unique_ptr<KeyPair> keys(new KeyPair());

        RSA_generate_key_ex(keys->rsa, length, keys->ex, nullptr);

        return keys;
    }

    static std::unique_ptr<KeyPair> fromPemString(const std::string &pemString) {
        std::unique_ptr<KeyPair> keys(new KeyPair());

        BIO *bio = BIO_new(BIO_s_mem());

        BIO_puts(bio, pemString.c_str());

        RSA_free(keys->rsa);

        keys->rsa = PEM_read_bio_RSAPrivateKey(bio, nullptr, nullptr, nullptr);

        return keys;
    }
};


#endif // GEOMESH_CRYPTO_H
