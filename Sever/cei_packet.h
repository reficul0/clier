#pragma once

#include <inttypes.h>

namespace specification
{
#pragma pack(push, 1)
	struct CEIHeader
	{
		uint32_t version;
		uint32_t length;
		uint16_t packet_id;
	};
	struct CEIPayload
	{
		int64_t payload;
	};
	struct CEIPacket
	{
		CEIHeader header;
		CEIPayload payload;
		uint16_t crc;
	};
#pragma pack(pop)
}