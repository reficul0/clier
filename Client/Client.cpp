// Client.cpp : Этот файл содержит функцию "main". Здесь начинается и заканчивается выполнение программы.
//

#include "pch.h"
#include <iostream>

#include <boost/asio.hpp>
#include <boost/thread.hpp>

#include "tools/asio/ip/tcp_client.hpp"
#include "../protocol/connections_guard.hpp"
#include "../protocol/packet_builder.hpp"

int main(int argc, char *argv[])
{
	if (argc < 3)
	{
		std::cout << "Usage <host_ip> <port>" << std::endl;
		return EXIT_SUCCESS;
	}
	
	auto host_ip = argv[1];
	unsigned short port_number = 0;
	{
		auto port_as_string = std::string{ argv[2] };
		try
		{
			port_number = std::stoul(port_as_string);
		}
		catch (std::exception const &)
		{
			std::cout << "Error: can't cast console arg 2 \"" + port_as_string + "\" to unsigned long." << std::endl;
			return EXIT_FAILURE;
		}
	}
	
	boost::asio::io_service io_service;

	ip::tcp::client client{ io_service };
	protocol::cei::connections_guard connections_guard;
	client.on_connected.connect(
		[&connections_guard](ip::tcp::connection *c) { connections_guard.on_connected(c); }
	);
	client.on_disconnected.connect(
		[&connections_guard](ip::tcp::connection *c) { connections_guard.on_disconnected(c); }
	);

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

	auto were_read_from_connection = [&connections_guard, &client](ip::tcp::connection *connection, std::vector<uint8_t> packet_bytes)
	{
		protocol::cei::packet *packet = nullptr;
		try
		{
			static size_t constexpr bytes_to_read = sizeof(protocol::cei::packet);
			size_t const bytes_were_read = packet_bytes.size();
			
			// todo если  будет таймаут на ожидание окончания считывания, то это перестанет считаться ошибкой.
			if (bytes_to_read > bytes_were_read)
				throw std::logic_error{
					"Accepted " + std::to_string(bytes_were_read)
					+ ", but expected " + std::to_string(bytes_to_read)
				};

			packet = (protocol::cei::packet*)packet_bytes.data();
			connections_guard.check_accepted_packet(
				connection,
				*packet
			);
		}
		catch (std::logic_error const &e)
		{
			std::cout << "Error: " << e.what() << std::endl
				<< ". Client will disconnect." << std::endl;
			return false;
		}

		std::cout << "Incoming frame" << std::endl
			<< "\tHeader {" << std::endl
				<< "\t\tVersion " << std::to_string(packet->header.version) << std::endl
				<< "\t\tLength " << std::to_string(packet->header.length) << std::endl
				<< "\t\tPacketId " << std::to_string(packet->header.packet_id) << std::endl
			<< "\t}" << std::endl
			<< "\tPayload " << std::to_string(packet->payload.payload) << std::endl
			<< "\tCrc " << std::to_string(packet->crc) << std::endl;
		return true;
	};
	
	while (true)
	{
		if (!client.is_connected())
			connect_to_server(client);
		
		client.read(sizeof(protocol::cei::packet), were_read_from_connection);
	}
}