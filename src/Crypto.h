
#ifndef CRYPTO_H
#define CRYPTO_H

#include <cryptopp/base64.h>
#include <cryptopp/eccrypto.h>
#include <cryptopp/filters.h>
#include <cryptopp/osrng.h>
#include <cryptopp/asn.h>
#include <cryptopp/oids.h>

typedef CryptoPP::ECDSA<CryptoPP::ECP, CryptoPP::SHA256>::PrivateKey AddressPrivateKey;
typedef CryptoPP::ECDSA<CryptoPP::ECP, CryptoPP::SHA256>::PublicKey AddressPublicKey;

/**
 * Generate an ECDSA key.
 */
AddressPrivateKey generateAddressKey();

/**
 * Generate a string representing the Base64 encoding
 * of the private key. (The exponent in big-endian format)
 */
std::string privateKeyToString(const AddressPrivateKey& pKey);

/**
 * Returns a private ECDSA key, interpreting keyString
 * as a Base64-encoded private exponent in big-endian
 * format.
 */
AddressPrivateKey privateKeyFromString(const std::string& keyString);

#endif // CRYPTO_H
