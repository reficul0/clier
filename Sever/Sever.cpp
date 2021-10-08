#include "pch.h"

#include <iostream>
#include <random>
#include <thread>
#include <mutex>

#include <boost/asio.hpp>
#include <boost/thread.hpp>

#include <cliver/asio/ip/tcp_server.hpp>
#include "protocol/connections_guard.hpp"
#include "protocol/packet_builder.hpp"
#include "program_options.hpp"


int main(int argc, char *argv[])
{
	configuration::Configuration cfg;
	try
	{
		if (auto optional_cfg = configuration::get_configuration(argc, argv))
			cfg = *optional_cfg;
		else
			return EXIT_SUCCESS;
		
	}
	catch(std::exception const &e)
	{
		std::cout << e.what() << std::endl;
		return EXIT_FAILURE;
	}
	
	srand(time(NULL));

	boost::asio::io_service io_service;
	boost::asio::io_service::work work{ io_service };
	std::thread io_thread{
		[&io_service]()
		{
			io_service.run();
		}
	};

	ip::tcp::server server{ io_service, cfg.port_number };

	protocol::cei::connections_guard connections_guard;
	server.on_connected.connect(
		[&connections_guard](ip::tcp::connection *c) { connections_guard.on_connected(c); }
	);
	server.on_disconnected.connect(
		[&connections_guard](ip::tcp::connection *c) { connections_guard.on_disconnected(c); }
	);
	server.start_acception();

	auto get_packet_for_connection = [&connections_guard](ip::tcp::connection *connection)
		-> std::vector<uint8_t>
	{
		auto packet_bytes = connections_guard.get_packet(
			connection,
			protocol::cei::payload{
				rand() % std::numeric_limits<decltype(protocol::cei::payload::payload)>::max()
			}
		);
		auto *packet = (protocol::cei::packet*)packet_bytes.data();
		
		std::cout << "Outgoung frame" << std::endl
			<< "\tHeader {" << std::endl
				<< "\t\tVersion " << std::to_string(packet->header.version) << std::endl
				<< "\t\tLength " << std::to_string(packet->header.length) << std::endl
				<< "\t\tPacketId " << std::to_string(packet->header.packet_id) << std::endl
			<< "\t}" << std::endl
			<< "\tPayload " << std::to_string(packet->payload.payload) << std::endl
			<< "\tCrc " << std::to_string(packet->crc) << std::endl;
		
		return std::move(packet_bytes);
	};
	while (true)
	{
		if (!server.get_connections_count())
		{
			boost::this_thread::sleep_for(cfg.wait_for_first_connection_timeout_ms);
			continue;
		}

		server.write(get_packet_for_connection);
	}
	
	return EXIT_SUCCESS;
}