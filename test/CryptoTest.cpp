#include "../src/Logger.hpp"
#include <cryptopp/base64.h>
#include <cryptopp/hex.h>
#include <cryptopp/eccrypto.h>
#include <cryptopp/filters.h>
#include <cryptopp/osrng.h>
#include <cryptopp/asn.h>
#include <cryptopp/oids.h>
#include <iostream>
#include <fstream>
#include <exception>
#include <gtest/gtest.h>
#include "../src/Crypto.h"
#include "../src/UniqueAddress.hpp"


using namespace CryptoPP;
using namespace std;


TEST(crypto, encode_decode_cycle_1) {

    AutoSeededRandomPool rng;

    AddressPrivateKey privateKey = generateAddressKey();

    std::string keyString = privateKeyToString(privateKey);
    
    std::cout << keyString << std::endl;

    AddressPrivateKey key = privateKeyFromString(keyString);

   EXPECT_EQ(privateKey.GetPrivateExponent(), key.GetPrivateExponent());

}

TEST(crypto, encode_decode_cycle) {

    AutoSeededRandomPool rng;

    AddressPrivateKey privateKey = generateAddressKey();

    std::string keyString = privateKeyToString(privateKey);
    
    std::cout << keyString << std::endl;

    AddressPrivateKey key = privateKeyFromString(keyString);

   EXPECT_EQ(privateKey.GetPrivateExponent(), key.GetPrivateExponent());

}
