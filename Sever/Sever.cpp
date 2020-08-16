#include "pch.h"

#include <iostream>
#include <random>
#include <thread>
#include <mutex>

#include <boost/asio.hpp>
#include <boost/thread.hpp>

#include "../communication/tcp_server.h"
#include "../protocol/packet_builder.h"

namespace protocol
{
	namespace cei
	{
		class connections_guard
		{
			std::unordered_map<ip::tcp::connection*, packet_builder<packet>> _connections_guard;
		public:
			void on_connected(ip::tcp::connection *connection)
			{
				_connections_guard[connection];
			}
			void on_disconnected(ip::tcp::connection *connection)
			{
				auto const connection_iter = _connections_guard.find(connection);
				if(connection_iter!= _connections_guard.end())
					_connections_guard.erase(connection_iter);
			}
			std::vector<uint8_t> get_packet(ip::tcp::connection *connection, payload const &packet_payload)
			{
				return _connections_guard[connection].get_packet(packet_payload);
			}
		};
	}
}

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
		auto* packet = (protocol::cei::packet*)&packet_bytes[0];
		
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
			// todo: конфигурирование таймаута ожидания первого соединения
			boost::this_thread::sleep_for(boost::chrono::milliseconds(50));
			continue;
		}
		server.write(get_packet_for_connection);
	}
	
	return EXIT_SUCCESS;
}