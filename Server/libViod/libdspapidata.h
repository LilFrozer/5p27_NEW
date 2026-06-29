#ifndef LIBDSPAPIDATA_H_INCLUDED
#define LIBDSPAPIDATA_H_INCLUDED

#include <cstdint>

#pragma pack( push, 1 )

struct VIOD_HEADER
{
	uint16_t slid : 2,
		spid	  : 2,
		smid	  : 4,
		dlid	  : 2,
		dpid	  : 2,
		dmid	  : 4;
	uint16_t prmbl;
	uint16_t reserved1 : 2,
		offset		   : 14;
	uint16_t length;
	uint16_t full_length;
	uint16_t chid : 4,
		e		  : 1,
		f		  : 1,
		reserved2 : 10;
	uint16_t ip_port;
	uint8_t id;
	uint8_t res	  : 6,
		far		  : 1,
		broadcast : 1;
};

struct VIOD_PACKET
{
	VIOD_HEADER header;
	uint32_t data[ 360 ];
};

#pragma pack( pop )

using ClientPtr = void *;
using ClientID = uint8_t;

#ifdef __cplusplus
extern "C"
{
#endif

	ClientPtr dspapi_init( ClientID clientID, const char *server_ip, bool debug = false );
	void dspapi_close( ClientPtr client );
	int dspapi_recv( ClientPtr client, VIOD_PACKET *packet );
	int dspapi_send( ClientPtr client, const VIOD_PACKET &packet );

#ifdef __cplusplus
}
#endif

#endif // LIBDSPAPIDATA_H_INCLUDED
