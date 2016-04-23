#include <memory>
#include <gtest/gtest.h>
#include "../src/Crypto.h"
#include "../src/UniqueAddress.hpp"

TEST(crypto, address_crypto) {

    std::cout << "Generating 500 addresses to see if they're unique." << std::endl;

    std::set<Address> addresses;
    for (int i = 0; i < 500; i++) {
        
        if (i % 10 == 0) {
            std::cout << '\r' << (i/5) << "% " << std::flush;
        }

        auto keys = KeyPair::generateNewKeypair(2048);

        Address addr = Address::generateFromKeys(*keys);

        ASSERT_EQ(addresses.end(), addresses.find(addr));
        addresses.insert(addr);
    }

    std::cout << std::endl;

}

TEST(crypto, sha256) {

    std::string testString = "The quick brown fox jumped over the lazy dog.";

    std::string expected = "68b1282b91de2c054c36629cb8dd447f12f096d3e3c587978dc2248444633483";

    auto hash = simpleSHA256(testString.data(), testString.length());

    std::stringstream result;

result << std::hex;

    for (int i = 0; i < 32; i++) {
        result << (hash[i] >> 4);
        result << (hash[i] & 15);
    }

    EXPECT_EQ(expected, result.str());
}

TEST(crypto, sign_and_verify) {

   auto keys = KeyPair::generateNewKeypair(2048);
   
   std::string testString = "The quick brown fox jumped over the lazy dog.";

   auto signature = keys->sign((uint8_t*)testString.c_str(), testString.length());

   EXPECT_TRUE(keys->verify((uint8_t*)testString.c_str(), testString.length(), (uint8_t*) signature.data(), signature.size()));

}

