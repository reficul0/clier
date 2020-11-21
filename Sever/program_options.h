#pragma once

#include <boost/program_options.hpp>
#include <boost/format.hpp>

#define OPTION_PORT "port"
#define SHORT_OPTION_PORT "p"
#define OPTION_CONNECTION_TIMEOUT "connection_timeout"
#define SHORT_OPTION_CONNECTION_TIMEOUT "t"

namespace configuration
{
	struct Configuration
	{
		unsigned short port_number;
		boost::chrono::milliseconds wait_for_first_connection_timeout_ms;
	};

	boost::program_options::variables_map get_program_options(int argc, char *argv[])
	{
		namespace po = boost::program_options;

		auto full_and_short_option_fmt = boost::format{ "%1%,%2%" };

		po::options_description options_descr{ "Optinos" };
		
		options_descr.add_options()
			("help,h", "Help.")
			((full_and_short_option_fmt % OPTION_PORT % SHORT_OPTION_PORT).str().c_str(),
				po::value<decltype(Configuration::port_number)>()->required(),
				"Port where server will accept connections."
			)
			((full_and_short_option_fmt % OPTION_CONNECTION_TIMEOUT % SHORT_OPTION_CONNECTION_TIMEOUT).str().c_str(),
				po::value<uint64_t>()->required(),
				"Wait for first connection timeout in milliseconds."
			);

		po::variables_map options;
		po::store(po::parse_command_line(argc, argv, options_descr), options);

		if (options.count("help"))
		{
			std::cout << options_descr << std::endl;
			return {};
		}

		return std::move(options);
	}

	boost::optional<Configuration> get_configuration(int argc, char *argv[])
	{
		auto const options = get_program_options(argc, argv);
		if (!options.size())
			return {};

		auto port_number = options[OPTION_PORT].as<unsigned short>();
		auto wait_for_first_connection_timeout_ms = boost::chrono::milliseconds(options[OPTION_CONNECTION_TIMEOUT].as<uint64_t>());

		return Configuration{ port_number, wait_for_first_connection_timeout_ms };
	}
}