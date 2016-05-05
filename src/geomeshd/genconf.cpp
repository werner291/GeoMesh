#include "../Crypto.h"
#include "../Logger.hpp"
#include <cryptopp/base64.h>
#include <cryptopp/eccrypto.h>
#include <cryptopp/filters.h>
#include <cryptopp/osrng.h>
#include <cryptopp/asn.h>
#include <cryptopp/oids.h>
#include <iostream>
#include <fstream>
#include <exception>

using namespace std;
using namespace CryptoPP;

void generateConfigFile(const std::string& path) {
        // Open file stream to specified config file location.
        ofstream genCfg(path);

        if (!genCfg.is_open()) {
            throw runtime_error("Cannot write to file: " + path);
        }

        ////////////////////////////////////////////////////////////
        // Write the block with default UDP bridge settings
        ////////////////////////////////////////////////////////////
        
        genCfg << "# Whether to enable the UDP bridge such that links can be"
                   " established over IP or" << std::endl;
        genCfg << "# IP-compatible networks like the Internet." << std::endl;
        genCfg << "# A note about security: This connection is not encrypted"
                   " by design." << std::endl;
        genCfg << "# To secure the link you could send it through an encrypted"
                   " tunnel." << std::endl;
        genCfg << "udp_bridge_enable = 1" << std::endl;
        genCfg << std::endl;
        genCfg << "# The UDP port on which to accept incoming connections." 
               << std::endl;
        genCfg << "udp_port = 10976" << std::endl;
        genCfg << std::endl;
        genCfg << "# List of UDP bridge peers in x.x.x.x:port format"
                   " (separated by spaces)" << std::endl;
        genCfg << "# For example: udp_peers = 192.168.1.1:10976 192.0.2.0:9854"
               << std::endl;
        genCfg << "udp_peers = []" << std::endl;
        genCfg << std::endl << std::endl;

        ////////////////////////////////////////////////////////////
        // Write the block about Ethernet links (unimplemented)
        ////////////////////////////////////////////////////////////
        //genCfg << "ethernet_autoconnect = 1" << std::endl;

        //////////////////////////////////////////////////////////// 
        // Generate an RSA keypair and write it into the configuration
        ////////////////////////////////////////////////////////////

        AutoSeededRandomPool rng;
        Logger::log(LogLevel::DEBUG, "RNG seeded.");

        AddressPrivateKey privateKey;
        // TODO Try so understand what secp60r1() exactly means.
        privateKey.Initialize( rng, ASN1::secp256r1()  );
        bool result = privateKey.Validate( rng, 3  );
        if( !result  )
            throw runtime_error("Private Key validation error."
                    " Retry or file bug report.");
                
        Logger::log(LogLevel::DEBUG, "Private key created.");

        std::string keyString;
        Base64Encoder encoder(new StringSink(keyString));
        privateKey.DEREncodePrivateKey(encoder);
        encoder.MessageEnd();

        genCfg << "# The private key that the address is derived from. DO NOT SHARE!" << std::endl;
        genCfg << "address_key = " << keyString << std::endl;
        genCfg << std::endl << std::endl;

        ////////////////////////////////////////////////////////////
        // Write the block about the local interface and tunnel interface
        ////////////////////////////////////////////////////////////
        
        genCfg << "# Whether to allow an exchange of datagrams between the host and" << std::endl;
        genCfg << "# GeoMesh. Usually you want this, otherwise this node will just" << std::endl;
        genCfg << "# act as a relay." << std::endl;
        genCfg << "local_interface_enable = 1" << std::endl;
        genCfg << std::endl;
        genCfg << "# The name of the tun or utun device" << std::endl;
        genCfg << "tun_device_name = tunGeo" << std::endl;
        genCfg << std::endl << std::endl;

        ////////////////////////////////////////////////////////////
        // Write the block about the local interface and tunnel interface
        ////////////////////////////////////////////////////////////
        
        genCfg << "# Whether to enable RPC" << std::endl;
        genCfg << "rpc_enable = 1" << std::endl;
        genCfg << std::endl;
        genCfg << "# The TCP port number for the RPC interface." << std::endl;
        genCfg << "rpc_port = 10976" << std::endl;
        genCfg << std::endl;
        genCfg << "# The password for RPC." << std::endl;
        genCfg << "rpc_password = ";

        for (int i = 0; i < 10; ++i) {
            genCfg << std::hex << (int) rng.GenerateByte();  
        }
        
        genCfg << std::endl << std::endl;

        //////////////
        // Finalize //
        //////////////
        
        genCfg.flush();
        genCfg.close();
}
