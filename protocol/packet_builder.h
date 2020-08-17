#pragma once

#include <atomic>

#include "cei_packet.h"

namespace protocol
{
	template<typename PacketType>
	class packet_builder
	{};

	template<>
	class packet_builder<cei::packet>
	{
		std::atomic<size_t> _iteration{ 0 };
	public:
		std::vector<uint8_t> get_packet(cei::payload const &payload)
		{
			std::vector<uint8_t> packet_bytes;
			packet_bytes.resize(sizeof(cei::packet));
			cei::packet* packet = (cei::packet*)packet_bytes.data();
			
			packet->header.version = 0;
			packet->header.packet_id = _iteration++;
			packet->header.length = sizeof(cei::packet);

			packet->payload = payload;

			packet->crc = 0x314;

			return std::move(packet_bytes);
		}
		
		void check_accepted_packet(cei::packet const &packet)
		{
			if (packet.header.version != 0)
				throw std::logic_error{ "Unknown packet version value \"" + std::to_string(packet.header.version) + "\"" };
			
			if (packet.header.packet_id != _iteration)
				throw std::logic_error{
					"Uncorrect packet id value \"" + std::to_string(packet.header.version) + "\""
					+ ". Expected \"" + std::to_string(_iteration.load()) + "\""
				};
			
			if (packet.header.length != sizeof(cei::packet))
				throw std::logic_error{
					"Uncorrect packet length value \"" + std::to_string(packet.header.length) + "\""
					+ ". Expected \"" + std::to_string(sizeof(cei::packet)) + "\""
			};
			
			if(packet.crc != 0x314)
				throw std::logic_error{ "Uncorrect packet crc value \"" + std::to_string(packet.crc) + "\"" };
			
			++_iteration;
		}
	};
}