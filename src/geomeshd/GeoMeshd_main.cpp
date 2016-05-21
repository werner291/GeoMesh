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

#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>
#include <regex>
#include <unistd.h>
#include <boost/program_options.hpp>
#include <syslog.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>

#include <cryptopp/base64.h>
#include <cryptopp/eccrypto.h>
#include <cryptopp/filters.h>
#include <cryptopp/osrng.h>
#include <cryptopp/asn.h>
#include <cryptopp/oids.h>

using namespace std;
using namespace CryptoPP;
namespace po = boost::program_options;

#include "../Crypto.h"

#include "genconf.h"

#ifdef __linux__
#include "TunnelDeliveryInterface_Linux.hpp"
#endif
#ifdef __APPLE__
#include "TunnelDeliveryInterface_Apple.hpp"
#endif

#include "../UniqueAddress.hpp"
#include "../Logger.hpp"
#include "UDPManager.hpp"
#include "../Router.hpp"
#include "../Scheduler.hpp"

#include "FileDescriptorNotifier.hpp"

#include "http/HTTPServer.hpp"
#include "RESTHandler.hpp"
#include "RESTResourceAdapters.hpp"

#define DEFAULT_CONFIG_NODAEMON "geomesh.json"
#define DEFAULT_CONFIG_DAEMON "/etc/geomesh.json"

// TUN devices work differently on certain hosts, select the right one.
#ifdef __APPLE__
typedef TunnelDeliveryInterface_Apple TunnelIface;
#endif
#ifdef __linux__
typedef TunnelDeliveryInterface_Linux TunnelIface;
#endif
// TODO Windows

// Whether we are using log files that need to be closed.
bool usingSyslog = false;

// Set to false to start the graceful shutdown process.
// Will close log files and notify contacts.
bool running = true;

void enableSyslogStrategy() {
    setlogmask(LOG_UPTO (LOG_NOTICE));
    openlog("geomeshd", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL1);

    Logger::setLogStrategy([](const LogLevel level, const std::string &string) {
        switch (level) {
            default:
                syslog(LOG_NOTICE, "%s", string.c_str());
            break;
        }
    });

    usingSyslog = true;
}

void signalReceived(int signal) 
{
    std::cout << "Received signal: " << signal << std::endl;
    running = false;
}


void daemonize() 
{
    pid_t pid = fork();

    if (pid > 0) {
        // Parent process

        std::cout << "Daemon forked to background successfully!" << std::endl;
        std::cout << "Child process ad PID: " << pid << std::endl;

        exit(0);
    } else if (pid < 0) {
        std::cout << "Fork failed!" << std::endl;
        exit(1);
    } else {
    // This is the child process, continue daemonization (TODO)

        enableSyslogStrategy();
    }

    umask(0);

    /* Create a new SID for the child process */
    pid_t sid = setsid();
    if (sid < 0) {

            Logger::log(LogLevel::ERROR, "Cannot set session ID of daemon.");
            closelog();

            exit(EXIT_FAILURE);
    }
}

int main(int argc, char **argv) {

    // Cover our asses
    
    std::cout << "(c) Copyright 2016 Werner Kroneman. This program was written"
        << " in the hope that it would be useful, but comes with ABSOLUTELY NO"
        << " WARRANTY! This program is free software licensed under the GNU"
        << " GPLv3 license. See the LICENSE file you should have received with"
        << " the software." << std::endl;

    // Register signal handlers
    
    signal(SIGINT, &signalReceived);

    po::options_description config_options("GeoMesh configuration");
    config_options.add_options()
            ("udp_peers",
                 po::value<std::vector<std::string> >()->multitoken(),
                 "List of UDP peers in IPAddress:Port format.")
            ("udp_port",
                 po::value<int>()->default_value(10976),
                 "UDP port on which to listen for UDP bridge packets.")
            ("udp_bridge_enable", 
                 po::value<bool>()->default_value(true),
                 "Whether to enable the UDP bridge.")
            ("rpc_enable",
                 po::value<bool>()->default_value(true),
                 "Whether to enable administration and querying through RPC."
                 " Otherwise, the only way to communicate with the process is"
                 " through signals.")
            ("rpc_port",
                 po::value<int>()->default_value(10976),
                 "The TCP port to listen on for incoming RPC requests.")
            ("rpc_password",
                 po::value<std::string>()->required(),
                 "The password used to authenticate with RPC. Typically also"
                 " the password to any web interfaces that might exist.")
            ("address_key", 
                 "A base64-encoded private key used to generate a public key"
                 " and an address. The generated address will be printed at"
                 " startup. Note: keep this key secure, or others will be able"
                 " to hijack your address. Also: GeoMesh only uses keys for a"
                 " very limited form of authentication, messages are sent in"
                 " cleartext! Use software like SSL to keep your data secure!")
            ("ethernet_autoconnect", 
                 po::value<bool>()->default_value(true),
                 "Whether to automatically connect to other GeoMesh nodes that"
                 " are connected through Ethernet, Wifi, etc...")
            ("local_interface_enable", 
                 po::value<bool>()->default_value(true),
                 "Whether to make GeoMesh accessible to the host such that"
                 " processes on the host can access GeoMesh hosts as if it"
                 " were IPv6. This is done through a TUN device on Linux, or a"
                 " utun device on OSX. An entry will also be added to the"
                 " routing table to redirect all traffic bound for fcf4::/16"
                 " into that tunnel device.")
            ("tun_device_name", 
                 po::value<std::string>()->default_value("tunGeo"),
                 "The name of the tunnel device.");

    po::options_description cli_options;
    cli_options.add_options()
            ("help,h", "Display help.")
            ("config,c",
             po::value<std::string>()->default_value("/etc/geomesh.conf"),
             "Config file location.")
            ("genconf",
             po::value<std::string>(), 
             "Generate a new config file at a specified path.")
            ("version,v",
             "Show version and exit.")
            ("verbose",
             "Enable DEBUG-level logging.")
            ("daemon,d",
             po::value<bool>()->default_value(true),
             "Whether to fork into the background, or remain in"
             " the foreground.");

    //cli_options.add(config_options);

    // Parse command line arguments into program options
    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, cli_options), vm);
    po::notify(vm);

    if (vm.count("help")) {
        // Print help and exit.
        std::cout << cli_options << std::endl;
        return 0;
    }

    if (vm.count("version")) {
        // Display version and exit.
        std::cout << "Compiled on " << __DATE__ << ". Early development"
            " version. Refer to git for exact version information."
            << std::endl;
        return 0;
    }

    if (vm.count("verbose")) {
        Logger::setLogLevel(LogLevel::DEBUG);
        Logger::log(LogLevel::DEBUG, "Verbose logging enabled.");
    }

    if (vm.count("genconf")) {
        // If requested, generate a default configuration file at the specified
        // location and then exit.

        std::string path = vm["genconf"].as<std::string>();
        std::cout << "Generating default config file at path: " << path
            << std::endl;

        generateConfigFile(path);

        std::cout << "Done!" << std::endl;

        return 0;
    }

    // Check for any errors
    po::notify(vm);

    ifstream configFile(vm["config"].as<std::string>());

    if (!configFile.is_open()) {
        std::cout << "Config file " << vm["config"].as<std::string>() << 
            " not found. Exiting." << std::endl;
        std::cout << "Use " << argv[0] << 
            " --genconf [filepath] to generate a default configuration file." << std::endl;

        return 1;
    }

    po::store(po::parse_config_file(configFile, config_options), vm);

    if (vm["daemon"].as<bool>()) {
        daemonize();
    }

    // TODO validation

    std::string stringKey = vm["address_key"].as<std::string>();

    AddressPrivateKey key = privateKeyFromString(stringKey);

    AddressPublicKey pubKey;
    key.MakePublicKey(pubKey);

    Address addr = Address::generateFromKey(pubKey);

    std::cout << "Using address: " << addr.toString() << std::endl;

    GreedyRoutingTable table;
    ContactsSet contacts;

    Router router(addr, Location(0, 0), table);

    LocationLookupManager llm(router.getLocalHandler(),
                              addr,
                              router.getLocationMgr(),
                              contacts);

    // Allocate and initialize the scheduler
    Scheduler scheduler(false);

    FDNotifier fdlistener(false);

    std::unique_ptr<LocalInterface> ipAdapter;
    std::unique_ptr<TunnelIface> tunnel;

    // Start the tunnel interface (platform-specific)
    // Only if requested, otherwise GeoMesh will not be able to provide
    // services to the host. This may be useful for relay-only nodes.
    // Does not affect RPC.
    if (vm["local_interface_enable"].as<bool>()) {

        ipAdapter.reset(new LocalInterface(router.getLocalHandler(), llm));
        tunnel.reset(new TunnelIface(ipAdapter.get(), addr, fdlistener));
    }

    std::unique_ptr<UDPManager> udpManager;

    // If UDP bridge is enabled, create the UDPManager and configure it
    if (vm["udp_bridge_enable"].as<bool>()) {
        // Create a new UDP bridge manager
        udpManager.reset(new UDPManager(router.getLinkManager(),
                                        vm["udp_port"].as<int>(),
                                        fdlistener
        ));

        // Iterate over all the peers specified in the onfig file and try to connect to them.
        for (const std::string& peer : vm["udp_peers"].as<std::vector<std::string> >()) {

            if (! peer.empty()) {

                std::regex regex("([0-9]+.[0-9]+.[0-9]+.[0-9]+):([0-9]+)");

                std::smatch results;

                if (regex_match(peer, results, regex)) {
                    std::string address = results[1];
                    int port = std::stoi(results[2].str());

                    udpManager->connectTo(address,port);
                }
                else 
                {
                    std::cout << "Invalid peer address: " << peer
                              << ", must be in x.x.x.x:port format for IPv4,"
                                 " and in [...]:port format for IPv6"
                                 " (unimplemented)." << std::endl;

                    exit(1); // TODO Problematic if already daemonized, need to
                             // do validation early on
                }
            }
        }
    }

    std::unique_ptr<HTTPServer> httpserver;
    std::unique_ptr<RESTRequestHandler> rest;
    std::unique_ptr<ContactsResource> contactsResource;

    if (vm["rpc_enable"].as<bool>()) {

        rest.reset(new RESTRequestHandler());

        contactsResource.reset(new ContactsResource(contacts));
        rest->addResource("contacts",*contactsResource);

        httpserver.reset(new HTTPServer(9999,
                                        "Hello!",
                                        *rest,
                                        fdlistener
                                       )
                        );
    }

    // For the whole duration of the program, poll for messages and handle time-related things.
    while (running) {

        scheduler.update();

        usleep(10000); // Sleep 0.01 seconds = 10000 microseconds
    }

    Logger::log(LogLevel::INFO, "Shutting down...");

    if (usingSyslog) {
        closelog();
    }
}
