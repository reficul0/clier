#pragma once

#include <boost/asio.hpp>
#include <boost/enable_shared_from_this.hpp>

namespace ip
{
	namespace tcp
	{
		class connection
			: public boost::enable_shared_from_this<connection>
		{
			boost::asio::ip::tcp::socket _socket;
		public:
			static boost::shared_ptr<connection> create(boost::asio::io_service &io_service)
			{
				return boost::shared_ptr<connection>(new connection{ io_service });
			}
			decltype(_socket)& get_socket()
			{
				return _socket;
			}
			
			size_t write(
				uint8_t const *bytes, 
				size_t size, 
				std::function<void(boost::shared_ptr<connection>, size_t, boost::system::error_code)> completion_callback
			) {
				boost::system::error_code ec;
				const auto bytes_transferred = boost::asio::write(_socket, boost::asio::buffer(bytes, size), ec);
				completion_callback(shared_from_this(), bytes_transferred, ec);
				return bytes_transferred;
			}

			size_t read(
				uint8_t* bytes, 
				size_t &size,
				std::function<void(boost::shared_ptr<connection>, size_t, boost::system::error_code)> completion_callback
			) {
				boost::system::error_code ec;
				const auto bytes_transferred = boost::asio::read(_socket, boost::asio::buffer(bytes, size), ec);
				completion_callback(shared_from_this(), bytes_transferred, ec);
				return bytes_transferred;
			}

		private:
			connection(boost::asio::io_service &io_service)
				: _socket(io_service)
			{
			}
		};
	}
}