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
			_write(payload, std::move(completion_callback));
		}
	private:
		connection(boost::asio::io_service &io_service)
			: _socket(io_service)
		{
		}

		template<typename CompletionCallbackType>
		void _write(specification::CEIPayload const &payload, CompletionCallbackType completion_callback)
		{
			specification::CEIPacket packet;

			packet.header.version = 0;
			packet.header.packet_id = _iteration++;
			packet.header.length = sizeof(specification::CEIPacket);

			packet.payload = payload;

			packet.crc = 0x31415;

			boost::system::error_code ec;
			size_t bytes_transerred;
			bytes_transerred = boost::asio::write(_socket, boost::asio::buffer(&packet, packet.header.length), ec);
			completion_callback(shared_from_this(), bytes_transerred, ec);
		}
	};
}