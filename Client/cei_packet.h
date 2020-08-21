#pragma once

#include <inttypes.h>

namespace protocol
{
	namespace cei
	{
#pragma pack(push, 1)
		struct infoPacket
		{
			uint32_t version;
			uint32_t length;
			uint16_t packet_id;
		};
		struct payload
		{
			int64_t data;
		};
		struct packet
		{
			infoPacket header;
			payload data_send;
			uint16_t crc;
		};
#pragma pack(pop) 
	}
}
