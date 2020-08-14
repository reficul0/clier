#include "pch.h"

#include <fstream>
#include <iostream>
#include <random>
#include <thread>

#include <boost/asio.hpp>
#include <boost/log/trivial.hpp>

#include "tcp_server.h"
#include "tcp_client.h"

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

	tcp::server server{ io_service, port_number };
	tcp::client client{ io_service };

	auto host_ip = "127.0.0.1";
	
	auto connect_to_client = [host_ip, port_number](tcp::client &client)
	{
		while(!client.is_connected())
			client.connect(host_ip, std::to_string(port_number));
	};
	
	std::thread client_thread{
		[&client, &connect_to_client]()
		{
			std::ofstream output{"client_output.txt"};
			while (true)
				if (client.is_connected())
				{
					specification::CEIPacket packet;
					if (client.read((uint8_t*)&packet, sizeof(packet)))
						output << "Read " << std::endl
							<< "Header {" << std::endl
								<< "\tVersion " << std::to_string(packet.header.version) << std::endl
								<< "\tLength " << std::to_string(packet.header.length) << std::endl
								<< "\tPacketId " << std::to_string(packet.header.packet_id) << std::endl
							<< "}" << std::endl
							<< "Payload " << std::to_string(packet.payload.payload) << std::endl
							<< "Crc " << std::to_string(packet.crc) << std::endl
						<< std::endl;
				}
				else
					connect_to_client(client);
		}
	};

	std::thread server_thread{
		[&server]()
		{
			while (true)
				if (server.get_connections_count())
				{
					auto payload = specification::CEIPayload{
							rand() % std::numeric_limits<decltype(specification::CEIPayload::payload)>::max()
					};
					server.write(payload);
				}
		}
	};

	getchar();

	return EXIT_SUCCESS;
}