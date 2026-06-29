#ifndef LIBDSPAPI_H_INCLUDED
#define LIBDSPAPI_H_INCLUDED

#include <stdint.h>
#include <stdio.h>
#include <sys/types.h>

#pragma pack( push, 1 )

typedef struct
{
    int physAddr;
    off_t physSize;
} _physPart;

typedef struct
{
    void *pData;
    int size;
    int nPhysParts;
    _physPart physParts[ 0 ];
} unii_buffer;

#pragma pack( pop )

using IRQ_callback = void ( * )( uint8_t *data, ssize_t size );
using STATUS_callback = void ( * )( uint8_t *data, ssize_t size );

unii_buffer *unii_alloc( int size );
void unii_free( unii_buffer *pbuf );
void *unii_data( unii_buffer *pbuf, uint32_t vmeAddr );

int unii_init( const char *local_ip, const char *server_ip, bool debug = false );

int unii_open( char *path );
int unii_reset( int fd );

int unii_read( int fd, uint32_t addr, int count, void *buffer, uint32_t opflags );
int unii_read32a32( int fd, uint32_t addr32, int count32, void *buffer );
int unii_read32a32ex( int fd, uint32_t addr32, int count32, void *buffer, int opflags );

int unii_write( int fd, uint32_t addr, int count, void *buffer, uint32_t opflags );
int unii_write32a32( int fd, uint32_t addr32, int count32, void *buffer );
int unii_write32a32ex( int fd, uint32_t addr32, int count32, void *buffer, int opflags );

int unii_dma_read( int fd, uint32_t addr, int count, unii_buffer *pbuf, uint32_t opflags );
int unii_dma_read32a32( int fd, uint32_t addr32, int count32, unii_buffer *pbuf );
int unii_dma_read32a32ex( int fd, uint32_t addr32, int count32, unii_buffer *pbuf, int opflags );

int unii_dma_write( int fd, uint32_t addr, int count, unii_buffer *pbuf, uint32_t opflags );
int unii_dma_write32a32( int fd, uint32_t addr32, int count32, unii_buffer *pbuf );
int unii_dma_write32a32ex( int fd, uint32_t addr32, int count32, unii_buffer *pbuf, int opflags );

int unii_read_reg( int fd, uint32_t addr, uint32_t *value );
int unii_write_reg( int fd, uint32_t addr, uint32_t value );

int unii_register_irq( int fd, IRQ_callback icb );
int unii_unregister_irq( int fd );

int unii_register_status( int fd, STATUS_callback scb );
int unii_unregister_status( int fd );

#endif // LIBDSPAPI_H_INCLUDED
