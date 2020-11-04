#pragma once

#include <inttypes.h>

namespace protocol
{
	namespace cei
	{
#pragma pack(push, 1)
		struct header
		{
			uint32_t version;
			uint32_t length;
			uint16_t packet_id;
		};
		struct payload
		{
			int64_t payload;
		};
		struct packet
		{
			header header;
			payload payload;
			uint16_t crc;
		};
#pragma pack(pop) 
	}
}