#pragma once

#include <boost/asio.hpp>
#include <boost/enable_shared_from_this.hpp>

#include "cei_packet.h"

namespace tcp
{
	class connection
		: public boost::enable_shared_from_this<connection>
	{
		boost::asio::ip::tcp::socket _socket;
		std::atomic<size_t> _iteration{ 0 };
	public:
		static boost::shared_ptr<connection> create(boost::asio::io_service &io_service)
		{
			return boost::shared_ptr<connection>(new connection{ io_service });
		}
		decltype(_socket)& get_socket()
		{
			return _socket;
		}
		template<typename CompletionCallbackType>
		void write(specification::CEIPayload const &payload, CompletionCallbackType completion_callback)
		{
			auto packet = _get_packet(payload);

			boost::system::error_code ec;
			auto bytes_transferred = boost::asio::write(_socket, boost::asio::buffer(&packet, packet.header.length), ec);
			completion_callback(shared_from_this(), bytes_transferred, ec);

			std::cout << "Write " << std::endl
				<< "Header {" << std::endl
					<< "\tVersion " << std::to_string(packet.header.version) << std::endl
					<< "\tLength " << std::to_string(packet.header.length) << std::endl
					<< "\tPacketId " << std::to_string(packet.header.packet_id) << std::endl
				<< "}" << std::endl
				<< "Payload " << std::to_string(packet.payload.payload) << std::endl
				<< "Crc " << std::to_string(packet.crc) << std::endl
			<< std::endl;
		}
		
		template<typename CompletionCallbackType>
		size_t read(uint8_t* bytes, size_t &size, CompletionCallbackType completion_callback)
		{
			boost::system::error_code ec;
			auto bytes_transferred = boost::asio::read(_socket, boost::asio::buffer(bytes, size), ec);
			completion_callback(shared_from_this(), bytes_transferred, ec);
			return bytes_transferred;
		}

	private:
		connection(boost::asio::io_service &io_service)
			: _socket(io_service)
		{
		}

		specification::CEIPacket _get_packet(specification::CEIPayload const &payload)
		{
			specification::CEIPacket packet;
			packet.header.version = 0;
			packet.header.packet_id = _iteration++;
			packet.header.length = sizeof(specification::CEIPacket);

			packet.payload = payload;

			packet.crc = 0x31415;
			
			return packet;
		}
	};
}