#pragma once

#include <thread>
#include <unordered_map>

#include <boost/asio.hpp>
#include <boost/thread/shared_mutex.hpp>
#include <boost/thread/locks.hpp>
#include <boost/bind.hpp>
#include <boost/signals2.hpp>

#include "tools/locks.h"
#include "tcp_connection.h"

namespace ip
{
	namespace tcp
	{
		class client
		{
			boost::asio::io_service &_io_service;
			boost::asio::ip::tcp::resolver _resolver;
			// todo std::atomic_bool?
			std::pair<boost::shared_ptr<tcp::connection>, bool> _connection;
			mutable boost::shared_mutex _connection_change;
		public:
			boost::signals2::signal<void(connection*)> on_connected;
			boost::signals2::signal<void(connection*)> on_disconnected;
			
			client(decltype(_io_service) io_context)
				: _io_service(io_context)
				, _resolver(io_context)
				, _connection(std::make_pair(nullptr, false))
			{
			}

			void connect(std::string const &host_ip, std::string const &host_port)
			{
				boost::asio::ip::tcp::resolver::query query(host_ip, host_port);
				auto endpoint = *_resolver.resolve(query);

				auto connection = connection::create(_io_service);
				boost::system::error_code ec;
				connection->get_socket().connect(endpoint, ec);
				_handle_connection(std::move(connection), ec);
			}
			void disconnect()
			{
				_handle_disconnection();
			}

			bool is_connected() const
			{
				TOOLS_SHARED_LOCK(_connection_change);
				return _is_connected();
			}

			size_t read(uint8_t *bytes, size_t count)
			{
				TOOLS_UNIQUE_LOCK(_connection_change);
				if (!_is_connected())
					return 0;

				auto const bytes_were_read = _connection.first->read(
					bytes,
					count,
					boost::bind(
						&client::_handle_reading_completion, this,
						boost::placeholders::_1, boost::placeholders::_2, boost::placeholders::_3
					)
				);

				// disconnected while reading
				if (!_is_connected())
				{
					_notify_about_disconnection_and_free_connection_resources();
					return 0;
				}
				
				return bytes_were_read;
			}

			// handler must return false if disconnection required
			void read(size_t count, std::function<bool(connection*, std::vector<uint8_t>)> were_read_from_connection)
			{
				TOOLS_UNIQUE_LOCK(_connection_change);
				if (!_is_connected())
					return;

				std::vector<uint8_t> data;
				data.resize(count);

				auto const bytes_were_read = _connection.first->read(
					data.data(),
					count,
					boost::bind(
						&client::_handle_reading_completion, this,
						boost::placeholders::_1, boost::placeholders::_2, boost::placeholders::_3
					)
				);

				data.resize(bytes_were_read);
				bool const is_disconnection_required = !were_read_from_connection(_connection.first.get(), std::move(data));
				
				// disconnection may has started(this is so if this is marked as disconnected)
				// while reading(reason of it is reading error appearing).
				if (_is_connected())
				{
					if (is_disconnection_required)
						_handle_disconnection_unsafe();
					return;
				}
				
				// disconnection already begin(then this is marked as disconnected),
				// while reading(reason is reading error appearing).
				// disconnection requires the completion
				_notify_about_disconnection_and_free_connection_resources();
			}
		private:
			bool _is_connected() const
			{
				return _connection.second;
			}
			
			void _handle_reading_completion(
				boost::shared_ptr<connection> connection,
				std::size_t bytes_transferred,
				boost::system::error_code error
			) {
				if(error)
					_set_connection_state_to_disconnected(connection);
			}

			void _handle_connection(
				boost::shared_ptr<connection> connection,
				boost::system::error_code error
			) {
				if (!error)
				{
					TOOLS_UNIQUE_LOCK(_connection_change);
					_connection = std::make_pair(std::move(connection), true);
					on_connected(_connection.first.get());
				}
			}

			void _handle_disconnection()
			{
				TOOLS_UNIQUE_LOCK(_connection_change);
				if (_connection.first)
					_handle_disconnection_unsafe();
			}
			void _handle_disconnection_unsafe()
			{
				_set_connection_state_to_disconnected(_connection.first);
				_notify_about_disconnection_and_free_connection_resources();
			}
			void _set_connection_state_to_disconnected(boost::shared_ptr<connection>)
			{
				_connection.second = false;
			}
			void _notify_about_disconnection_and_free_connection_resources()
			{
				on_disconnected(_connection.first.get());
				_connection.first.reset();
			}
		};
	}
}