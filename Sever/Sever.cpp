#include "pch.h"
#include <iostream>
#include <random>
#include <thread>

#include <boost/asio.hpp>

#include "tcp_server.h"

int main()
{
	unsigned short port_number{ 12345 };

	srand(time(NULL));

	boost::asio::io_service io_service;
	boost::asio::io_service::work work{ io_service };
	boost::asio::io_service::strand strand{ io_service };
	std::thread io_thread{
		[&strand_ref = strand]()
		{
			try
			{
				strand_ref.get_io_service().run();
			}
			catch (boost::thread_interrupted const&)
			{
			}
		}
	};

	tcp::server server{ io_service, port_number };

	while (true)
		if (server.get_connections_count())
			server.write(
				specification::CEIPayload{ 
					rand() % std::numeric_limits<decltype(specification::CEIPayload::payload)>::max() 
				}
			);

	return EXIT_SUCCESS;
}