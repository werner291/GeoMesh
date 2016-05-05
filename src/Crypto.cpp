#include "Crypto.h"

using namespace std;
using namespace CryptoPP;

AddressPrivateKey generateAddressKey() {
    AutoSeededRandomPool rng;

    AddressPrivateKey privateKey;
    // TODO Try so understand what secp60r1() exactly means.
    privateKey.Initialize( rng, ASN1::secp256r1()  );
    bool result = privateKey.Validate( rng, 3  );

    if (! result) {
        throw runtime_error("Error while generating private address key: failed validation.");
    }
    return privateKey;
}


std::string privateKeyToString(const AddressPrivateKey& pKey) {
    std::string keyString;
    Base64Encoder encoder(new StringSink(keyString));

    const Integer& exponent = pKey.GetPrivateExponent();

    exponent.Encode(encoder, 32, Integer::UNSIGNED);

    encoder.MessageEnd();

    return keyString;
}

AddressPrivateKey privateKeyFromString(const std::string& keyString){
    Base64Decoder decoder;
    
    decoder.Put((byte*)keyString.c_str(),keyString.length());
    decoder.MessageEnd();

    Integer privateExponent;
    privateExponent.Decode(decoder, 32);
    AddressPrivateKey key;
    key.AccessGroupParameters().Initialize(CryptoPP::ASN1::secp256k1());
    key.SetPrivateExponent(privateExponent);

    return key;
}
