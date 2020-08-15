#include "pch.h"

#include <iostream>
#include <random>
#include <thread>
#include <mutex>

#include <boost/asio.hpp>
#include <boost/thread.hpp>

#include "../communication/tcp_server.h"
#include "../protocol/packet_builder.h"

int main()
{
	unsigned short port_number{ 12345 };
	srand(time(NULL));

	boost::asio::io_service io_service;
	boost::asio::io_service::work work{ io_service };
	std::thread io_thread{
		[&io_service]()
		{
			try
			{
				io_service.run();
			}
			catch (boost::thread_interrupted const&)
			{
			}
		}
	};

	ip::tcp::server server{ io_service, port_number };

	protocol::packet_builder<protocol::cei::packet> builder;
	while (true)
	{
		if (!server.get_connections_count())
		{
			boost::this_thread::sleep_for(boost::chrono::milliseconds(50));
			continue;
		}
		auto packet = builder.get_packet(
			protocol::cei::payload{
				rand() % std::numeric_limits<decltype(protocol::cei::payload::payload)>::max()
			}
		);
		server.write((uint8_t*)&packet, sizeof(packet));
			std::cout << "Write" << std::endl
				<< "\tHeader {" << std::endl
					<< "\t\tVersion " << std::to_string(packet.header.version) << std::endl
					<< "\t\tLength " << std::to_string(packet.header.length) << std::endl
					<< "\t\tPacketId " << std::to_string(packet.header.packet_id) << std::endl
				<< "\t}" << std::endl
				<< "\tPayload " << std::to_string(packet.payload.payload) << std::endl
				<< "\tCrc " << std::to_string(packet.crc) << std::endl;
	}
	
	return EXIT_SUCCESS;
}