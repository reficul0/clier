#pragma once

#include <future>
#include <unordered_map>

#include <boost/asio.hpp>
#include <boost/thread/shared_mutex.hpp>
#include <boost/thread/locks.hpp>
#include <boost/bind.hpp>
#include <boost/signals2.hpp>

#include "tcp_connection.h"

namespace ip
{
	namespace tcp
	{
		class server
		{
			boost::asio::io_service &_io_service;
			boost::asio::ip::tcp::acceptor _acceptor;
			std::unordered_map<boost::shared_ptr<connection>::element_type*, boost::shared_ptr<connection>> _connections;
			mutable boost::shared_mutex _connections_change;
		public:
			boost::signals2::signal<void(connection*)> on_connected;
			boost::signals2::signal<void(connection*)> on_disconnected;
			
			server(decltype(_io_service) io_service, unsigned short port)
				: _io_service(io_service)
				, _acceptor(io_service, boost::asio::ip::tcp::endpoint{ boost::asio::ip::tcp::v4(), port })
			{
			}

			void write(uint8_t const *bytes, size_t size)
			{
				boost::shared_lock<decltype(_connections_change)> connections_change_lock{ _connections_change };
				for (auto &connected : _connections)
					connected.second->write(
						bytes,
						size,
						boost::bind(
							&server::_handle_writing_completion, this,
							boost::placeholders::_1, boost::placeholders::_2, boost::placeholders::_3
						)
					);
			}
			void write(std::function<std::vector<uint8_t>(connection*)> get_packet_for_connection)
			{
				boost::shared_lock<decltype(_connections_change)> connections_change_lock{ _connections_change };
				for (auto &connection : _connections)
				{
					auto packet = get_packet_for_connection(connection.second.get());
					connection.second->write(
						&packet[0],
						packet.size(),
						boost::bind(
							&server::_handle_writing_completion, this,
							boost::placeholders::_1, boost::placeholders::_2, boost::placeholders::_3
						)
					);

				}
			}
			size_t get_connections_count() const
			{
				boost::shared_lock<decltype(_connections_change)> connections_change_lock{ _connections_change };
				return _connections.size();
			}

			void start_acception()
			{
				auto connection = connection::create(_io_service);

				_acceptor.async_accept(
					connection->get_socket(),
					boost::bind(&server::_handle_connection, this, connection, boost::asio::placeholders::error)
				);
			}
		private:
			void _handle_connection(
				boost::shared_ptr<connection> connection,
				boost::system::error_code error
			) {
				if (!error)
				{
					auto &socket = connection->get_socket();
					socket.set_option(boost::asio::ip::tcp::socket::keep_alive(true));
#ifdef _DEBUG
					socket.set_option(boost::asio::ip::tcp::socket::debug(true));
#endif
					auto connection_raw_ptr = connection.get();
					boost::unique_lock<decltype(_connections_change)> connections_change_lock{ _connections_change };
					_connections.emplace(connection_raw_ptr, std::move(connection));

					// todo: есть вероятность зависания, если коллбэк будет слишком долгим
					on_connected(connection_raw_ptr);
				}

				start_acception();
			}

			void _handle_writing_completion(
				boost::shared_ptr<connection> connection,
				std::size_t bytes_transferred,
				boost::system::error_code error
			) {
				if (error)
					_handle_disconnection(connection, error);
			}

			void _handle_disconnection(
				boost::shared_ptr<connection> connection,
				boost::system::error_code error
			) {
				// todo: может быть опасным
				// асинхронный вызов нужен, чтобы не инвалидировать итераторы, потому что этот дисконнект вызывается во время отправки по всем связям
				// и даже если отдетаченный поток не сразу удалит связь, то это не смертельно, ибо в запись в неприконнеченную связь только приведет к повторному вызову этого метода
				// и создаст отдетаченный поток заново, но метод, который мы передаём в поток учитывает, что связь может быть уже удалена.
				// Да, это тратит лишние ресурсы, но как хотфикс вполне допустим.
				std::thread(
					[connection, this]() 
					{
						auto connection_raw_ptr = connection.get();
						boost::unique_lock<decltype(_connections_change)> connections_change_lock{ _connections_change };
						auto connection_iter = _connections.find(connection_raw_ptr);
						if (connection_iter != _connections.end())
						{
							// todo: есть вероятность зависания, если коллбэк будет слишком долгим
							on_disconnected(connection_raw_ptr);
							_connections.erase(connection_iter);
						}
					}
				).detach();
			}
		};
	}
}