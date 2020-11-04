#pragma once
#include <unordered_map>

#include "cei_packet.hpp"
#include "packet_builder.hpp"
#include "tools/asio/ip/tcp_connection.hpp"

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
				if (connection_iter != _connections_guard.end())
					_connections_guard.erase(connection_iter);
			}
			std::vector<uint8_t> get_packet(ip::tcp::connection *connection, payload const &packet_payload)
			{
				return _connections_guard[connection].get_packet(packet_payload);
			}
			void check_accepted_packet(ip::tcp::connection *connection, packet const &packet)
			{
				_connections_guard[connection].check_accepted_packet(packet);
			}
		};
	}
}
