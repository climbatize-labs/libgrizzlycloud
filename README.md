# GrizzlyCloud library

GrizzlyCloud is a simplified VPN alternative for IoT (Internet of Things). Essentially it's just a client-server architecture that forwards your local TCP port requests to appropriate recipient. GrizzlyCloud library aims to provide a cross-platform cross-architecture support.

# Requirements

Compiled against:
- libev-dev 4.22
- libjson-c-dev 0.11
- libssl-dev 1.0.2g

# Guide

Client can be found in example/client. Feel free to modify both library and client to suit your needs. Configuration files are essential in order to have a client fully working.

Example client configuration:
```sh
{
    "user"     : "user1",
    "password" : "user1",
    "device"   : "DevName1",

    "tunnels"  : [
                   {"cloud"      : "user2",
                    "device"     : "DevName2",
                    "port"       : 22,
                    "portLocal"  : 1230 },

                   {"cloud"      : "user2",
                    "device"     : "DevName2",
                    "port"       : 22,
                    "portLocal"  : 1231 }
                 ]
}
```

Example server configuration:
```sh
{
    "user"     : "user2",
    "password" : "user2",
    "device"   : "DevName2",

    "allow"    : [ 22 ]
}
```

Don't forget to replace user and password parameters. Create your own account [here](https://grizzlycloud.com/signup.php).

Find out more about format of config file and available commands at [Wiki pages](https://grizzlycloud.com/wiki/doku.php?id=commands).

# Disclaimer

This version of library, although fully working, is not meant for a production environment just yet.
