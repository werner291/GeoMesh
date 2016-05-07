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
