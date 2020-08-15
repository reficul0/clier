// Client.cpp : Этот файл содержит функцию "main". Здесь начинается и заканчивается выполнение программы.
//

#include "pch.h"
#include <iostream>

#include <boost/asio.hpp>
#include <boost/thread.hpp>

#include "../communication/tcp_client.h"
#include "../protocol/packet_builder.h"

int main()
{
	unsigned short port_number{ 12345 };
	auto host_ip = "127.0.0.1";
	
	boost::asio::io_service io_service;

	ip::tcp::client client{ io_service };

	auto connect_to_server = [host_ip, port_number](decltype(client) &connect_me)
	{
		while (!connect_me.is_connected())
			connect_me.connect(host_ip, std::to_string(port_number));
	};

	protocol::packet_builder<protocol::cei::packet> builder;
	while (true)
	{
		if (!client.is_connected())
		{
			connect_to_server(client);
			boost::this_thread::sleep_for(boost::chrono::milliseconds(50));
			continue;
		}
		
		protocol::cei::packet packet;
		size_t const bytes_to_read = sizeof(packet),
					bytes_were_read = client.read((uint8_t*)&packet, bytes_to_read);
		if (bytes_to_read != bytes_were_read)
		{
			std::cout << "Accepted " << bytes_were_read << ", but expected " << bytes_to_read << std::endl
					<< ". Client will disconnect." << std::endl;
			client.disconnect();
			continue;
		}

		try
		{
			builder.check_accepted_packet(packet);
		}
		catch (std::logic_error const &e)
		{
			std::cout << "Error: " << e.what() << std::endl
					<< ". Client will disconnect." << std::endl;
			client.disconnect();
			continue;
		}
		
		std::cout << "Read" << std::endl
			<< "\tHeader {" << std::endl
				<< "\t\tVersion " << std::to_string(packet.header.version) << std::endl
				<< "\t\tLength " << std::to_string(packet.header.length) << std::endl
				<< "\t\tPacketId " << std::to_string(packet.header.packet_id) << std::endl
			<< "\t}" << std::endl
			<< "\tPayload " << std::to_string(packet.payload.payload) << std::endl
			<< "\tCrc " << std::to_string(packet.crc) << std::endl;

	}
}