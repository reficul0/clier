#pragma once

#include <unordered_map>

#include <boost/asio.hpp>
#include <boost/thread/shared_mutex.hpp>
#include <boost/thread/locks.hpp>
#include <boost/bind.hpp>

#include "tcp_connection.h"

namespace tcp
{
	class server
	{
		boost::asio::io_service &_io_service;
		boost::asio::ip::tcp::acceptor _acceptor;
		std::unordered_map<boost::shared_ptr<connection>::element_type*, boost::shared_ptr<connection>> _connections;
		boost::shared_mutex _connections_change;
	public:
		server(decltype(_io_service) io_service, unsigned short port)
			: _io_service(io_service)
			, _acceptor(io_service, boost::asio::ip::tcp::endpoint{ boost::asio::ip::tcp::v4(), port})
		{
			start_acception();
		}

		void write(specification::CEIPayload const &payload)
		{
			boost::shared_lock<decltype(_connections_change)> connections_change_lock{ _connections_change };
			for (auto &connected : _connections)
				connected.second->write(
					payload, 
					boost::bind(
						&server::handle_writing_completion, this,
						boost::placeholders::_1, boost::placeholders::_2, boost::placeholders::_3 
					)
				);
		}
		size_t get_connections_count()
		{
			boost::shared_lock<decltype(_connections_change)> connections_change_lock{ _connections_change };
			return _connections.size();
		}
	private:
		void start_acception()
		{
			auto connection = tcp::connection::create(_io_service);

			_acceptor.async_accept(
				connection->get_socket(),
				boost::bind(&server::handle_connection, this, connection, boost::asio::placeholders::error)
			);
		}

		void handle_connection(
			boost::shared_ptr<connection> connection,
			boost::system::error_code const &error
		) {
			if (!error)
			{
				boost::unique_lock<decltype(_connections_change)> connections_change_lock{ _connections_change };
				_connections.emplace(connection.get(), std::move(connection));
			}

			start_acception();
		}

		void handle_writing_completion(
			boost::shared_ptr<connection> connection,
			std::size_t bytes_transferred,
			const boost::system::error_code& error
		) {
			if (error)
			{
				boost::unique_lock<decltype(_connections_change)> connections_change_lock{ _connections_change };
				auto connection_iter = _connections.find(connection.get());
				if (connection_iter != _connections.end())
					_connections.erase(connection_iter);
			}
		}
	};
}