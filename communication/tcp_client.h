#pragma once

#include <unordered_map>

#include <boost/asio.hpp>
#include <boost/thread/shared_mutex.hpp>
#include <boost/thread/locks.hpp>
#include <boost/bind.hpp>

#include "tcp_connection.h"

namespace ip
{
	namespace tcp
	{
		class client
		{
			boost::asio::io_service &_io_service;
			boost::asio::ip::tcp::resolver _resolver;
			boost::shared_ptr<tcp::connection> _connection;
			mutable boost::shared_mutex _connection_change;
		public:
			client(decltype(_io_service) io_context)
				: _io_service(io_context)
				, _resolver(io_context)
			{
			}

			void connect(std::string const &host_ip, std::string const &host_port)
			{
				boost::asio::ip::tcp::resolver::query query(host_ip, host_port);
				auto endpoint = *_resolver.resolve(query);

				{
					boost::unique_lock<decltype(_connection_change)> connection_change_lock{ _connection_change };
					_connection = connection::create(_io_service);
				}
				boost::system::error_code ec;
				_connection->get_socket().connect(endpoint, ec);
				if (ec)
					_connection.reset();
			}


			void disconnect()
			{
				boost::unique_lock<decltype(_connection_change)> connection_change_lock{ _connection_change };
				_connection.reset();
			}

			bool is_connected() const
			{
				boost::shared_lock<decltype(_connection_change)> connection_change_lock{ _connection_change };
				return (bool)_connection;
			}

			size_t read(uint8_t *bytes, size_t count)
			{
				boost::unique_lock<decltype(_connection_change)> connection_change_lock{ _connection_change };
				if (!_connection)
					return 0;

				return _connection->read(
					bytes,
					count,
					boost::bind(
						&client::_handle_reading_completion, this,
						boost::placeholders::_1, boost::placeholders::_2, boost::placeholders::_3
					)
				);
			}
		private:
			void _handle_reading_completion(
				boost::shared_ptr<connection> connection,
				std::size_t bytes_transferred,
				const boost::system::error_code& error
			) {
				if (error)
				{
					boost::unique_lock<decltype(_connection_change)> connection_change_lock{ _connection_change };
					if (_connection.get() == connection.get())
						_connection.reset();
				}
			}
		};
	}
}