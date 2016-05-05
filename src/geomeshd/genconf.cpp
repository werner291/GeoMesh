#include "../Crypto.h"
#include "../Logger.hpp"
#include <boost/algorithm/string/replace.hpp>
#include <cryptopp/base64.h>
#include <cryptopp/eccrypto.h>
#include <cryptopp/filters.h>
#include <cryptopp/osrng.h>
#include <cryptopp/asn.h>
#include <cryptopp/oids.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <exception>

using namespace std;
using namespace CryptoPP;

// A template for the configuration file.
const std::string CONFIG_TEMPLATE = R"(
# Whether to enable the UDP bridge such that links can be established over IP or
# IP-compatible networks like the Internet.
# A note about security: This connection is not encrypted by design.
# To secure the link you could send it through an encrypted tunnel.
udp_bridge_enable = 1

# The UDP port on which to accept incoming connections.
udp_port = 10976

# List of UDP bridge peers in x.x.x.x:port format (separated by spaces)
# For example: udp_peers = 192.168.1.1:10976 192.0.2.0:9854
udp_peers = 


# The private key that the address is derived from. DO NOT SHARE!
address_key = $address_key


# Whether to allow an exchange of datagrams between the host and
# GeoMesh. Usually you want this, otherwise this node will just
# act as a relay.
local_interface_enable = 1

# The name of the tun or utun device
tun_device_name = tunGeo


# Whether to enable RPC
rpc_enable = 1

# The TCP port number for the RPC interface.
rpc_port = 10976

# The password for RPC.
rpc_password = $rpc_password
)";

void generateConfigFile(const std::string& path) {
    
    // Generate a new private key and encode it as a string
    std::string keyString = privateKeyToString(generateAddressKey());

    // Generate a random RPC password
    std::stringstream rpc_password;

    AutoSeededRandomPool rng;
    for (int i = 0; i < 10; ++i) {
        rpc_password << std::hex << (int) rng.GenerateByte();  
    }

    // Make a copy of the config file template and fill it with the values
    std::string config = CONFIG_TEMPLATE;
    boost::replace_all(config, "$address_key", keyString);
    boost::replace_all(config, "$rpc_password", rpc_password.str());

    // Open file stream to specified config file location.
    ofstream genCfg(path);

    if (!genCfg.is_open()) {
        throw runtime_error("Cannot write to file: " + path);
    }

    // Send it all to the file
    genCfg << config;
    
    // Finalize and close
    genCfg.flush();
    genCfg.close();
}
