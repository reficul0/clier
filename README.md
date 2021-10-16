Cliver
======

[![Build status](https://ci.appveyor.com/api/projects/status/vltix4u8lv2mvk32?svg=true)](https://ci.appveyor.com/project/reficul0/cliver)
[![Codacy Badge](https://api.codacy.com/project/badge/Grade/304d5a2963804ee5be401a90453272a9)](https://app.codacy.com/gh/reficul0/cliver?utm_source=github.com&utm_medium=referral&utm_content=reficul0/cliver&utm_campaign=Badge_Grade_Settings)
[![CodeFactor](https://www.codefactor.io/repository/github/reficul0/cliver/badge)](https://www.codefactor.io/repository/github/reficul0/cliver)
[![MIT License](https://img.shields.io/badge/license-MIT-blue.svg?style=flat)](https://github.com/RocketChat/Rocket.Chat/raw/master/LICENSE)
[![FOSSA Status](https://app.fossa.com/api/projects/git%2Bgithub.com%2Freficul0%2Fcliver.svg?type=shield)](https://app.fossa.com/projects/git%2Bgithub.com%2Freficul0%2Fcliver?ref=badge_shield)<br>
![language](https://img.shields.io/badge/language-c++-blue.svg)
![c++](https://img.shields.io/badge/std-c++14-blue.svg)

## Overview

Cliver is a C++ network client-server framework.

Builds upon [Boost.ASIO](http://www.boost.org/) to provide a simple API for developers.

### Features
Features:
* [x] Server can send packets to clients
* [x] Clients can handle server packets
* [x] Server can accept many clients connections
* [x] Cross-platform compatible

## Installation

### Requirements

* [Required] - [Conan](https://conan.io/).
* [Required] - pthread(Linux only).
* [Installing by conan] - [Boost 1.70](http://www.boost.org/).

### Building on Windows

```shell
- git clone git@github.com:reficul0/cliver.git
- mkdir build && cd build
- conan install ..
- cmake -A %platform% -G "Visual Studio 15 2017" -DBUILD_TESTS=OFF ..
- cmake --build .
```
### Building on Linux

```bash
$ git clone git@github.com:reficul0/cliver.git
$ mkdir build && cd build
$ conan install ..
$ cmake -DBUILD_TESTS=OFF ..
$ cmake --build .
```
### Getting Started

Create server, start connections acception and send to all connected clients specific packet:
```cpp
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <tools/asio/ip/tcp_server.hpp>

int main(int argc, char const* argv[]) {
  if (argc < 3)
  {
    std::cout << "Usage <port> <wait_for_first_connection_timout_milliseconds>" << std::endl;
    return EXIT_SUCCESS;
  }
  
  auto const port = std::stoul(argv[1]);
  boost::chrono::milliseconds const wait_for_first_connection_timout_ms{std::stoul(argv[2])};
  
  boost::asio::io_service io_service;
  boost::asio::io_service::work work{ io_service };
  std::thread io_thread{
    [&io_service]()
    {
      io_service.run();
    }
  };

  ip::tcp::server server{ io_service, port_number };
  
  server.start_acception();
  std::vector<uint8_t> packet = {1, 2, 3, 4};

  while (true)
  {
    if (!server.get_connections_count())
    {
      boost::this_thread::sleep_for(wait_for_first_connection_timout_ms);
      continue;
    }

    server.write(packet.data(), packet.size());
  }
  
  return (EXIT_SUCCESS);
}
```

Create client, connect to server and read its packet:
```cpp
#include <iostream>
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <tools/asio/ip/tcp_server.hpp>

int main(int argc, char const* argv[]) {
  if (argc < 3)
  {
    std::cout << "Usage <host_ip> <port>" << std::endl;
    return EXIT_SUCCESS;
  }
  
  auto host_ip = argv[1];
  unsigned short port_number = std::stoul(argv[2]);
  
  boost::asio::io_service io_service;
  ip::tcp::client client{ io_service };
  
  auto connect_to_server = [host_ip, port_number](decltype(client) &connect_me)
  {
    while (true)
    {
      connect_me.connect(host_ip, std::to_string(port_number));
      if (connect_me.is_connected())
        break;

      // todo customize wait timeout
      boost::this_thread::sleep_for(boost::chrono::milliseconds(50));
    }
  };
  std::vector<uint8_t> packet;
  packet.resize(4);
  
  while (true)
  {
    if (!client.is_connected())
      connect_to_server(client);
		
    if(client.read(packet.data(), packet.size()) != packet.size())
      std::cout << "Accepted packet is less than expected" << std:: endl;
      
    // process packet...
  }
  
  return (EXIT_SUCCESS);
}
```

## System Compatibility

OS           | Compiler      | Status
------------ | ------------- | -------------
Windows      | msvc15        | :white_check_mark: Working
Linux        | gcc           | :white_check_mark: Working

## Contributing

Pull requests are welcome.

## Authors

* [@reficul0](https://github.com/reficul0)

## License
[![FOSSA Status](https://app.fossa.com/api/projects/git%2Bgithub.com%2Freficul0%2Fcliver.svg?type=large)](https://app.fossa.com/projects/git%2Bgithub.com%2Freficul0%2Fcliver?ref=badge_large)
