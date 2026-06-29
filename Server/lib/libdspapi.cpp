#include <errno.h>
#include <string.h>

#include "core.h"
#include "libdspapi.h"

#define UNUSED( x ) ( void )( x )

static Core *core = Core::getInstance();

//----------------------------------------------------------------------------

DSP_CELL fdToCell( int fd )
{
    return ( fd == 3 )	? DSP_CELL::CELL6 :
           ( fd == 4 )	? DSP_CELL::CELL5 :
           ( fd == 5 )	? DSP_CELL::CELL4 :
           ( fd == 6 )	? DSP_CELL::CELL3 :
           ( fd == 7 )	? DSP_CELL::CELL2 :
           ( fd == 8 )	? DSP_CELL::CELL1 :
           ( fd == 9 )	? DSP_CELL::CELL0 :
           ( fd == 10 ) ? DSP_CELL::CTRL :
                          DSP_CELL::NONE;
}
//----------------------------------------------------------------------------

DSP_CELL addrToCell( uint32_t vmeAddr )
{
    uint32_t maskDSP = vmeAddr & 0xF0000000;

    return ( maskDSP == 0x30000000 ) ? DSP_CELL::CELL6 :
           ( maskDSP == 0x40000000 ) ? DSP_CELL::CELL5 :
           ( maskDSP == 0x50000000 ) ? DSP_CELL::CELL4 :
           ( maskDSP == 0x60000000 ) ? DSP_CELL::CELL3 :
           ( maskDSP == 0x70000000 ) ? DSP_CELL::CELL2 :
           ( maskDSP == 0x80000000 ) ? DSP_CELL::CELL1 :
           ( maskDSP == 0x90000000 ) ? DSP_CELL::CELL0 :
           ( maskDSP == 0xA0000000 ) ? DSP_CELL::CTRL :
                                       DSP_CELL::NONE;
}
//----------------------------------------------------------------------------

uint32_t makeAddr( uint32_t vmeAddr )
{
    uint32_t addr = ( vmeAddr & 0x00FFFFFF ) / 4;
    uint32_t maskDSP = vmeAddr & 0x0F000000;

    return ( maskDSP == 0x07000000 ) ? BROADCAST_ADDR + addr :
           ( maskDSP == 0x08000000 ) ? DSP0_ADDR + addr :
           ( maskDSP == 0x09000000 ) ? DSP1_ADDR + addr :
           ( maskDSP == 0x0A000000 ) ? DSP2_ADDR + addr :
           ( maskDSP == 0x0B000000 ) ? DSP3_ADDR + addr :
                                       SDRAM_ADDR + addr;
}
//----------------------------------------------------------------------------

bool cellAvailable( DSP_CELL cell )
{
    static std::map< DSP_CELL, int > cellStatusPos = {
        {DSP_CELL::CELL0, 0},
        {DSP_CELL::CELL1, 1},
        {DSP_CELL::CELL2, 2},
        {DSP_CELL::CELL3, 3},
        {DSP_CELL::CELL4, 4},
        {DSP_CELL::CELL5, 5},
        {DSP_CELL::CELL6, 6}
    };

    if( cell == DSP_CELL::NONE )
    {
        errno = ENODEV;
        return false;
    }

    uint32_t value = 0;
    int ret = core->read( DSP_CELL::CTRL, DSP_CMD::READ_REG, 0x00000020, ( uint8_t * )&value, sizeof( uint32_t ) );
    if( ret < 0 )
        return false;

    if( cell == DSP_CELL::CTRL )
        return true;

    if( ( value & ( 1 << cellStatusPos[ cell ] ) ) > 0 )
        return true;

    errno = ENODEV;
    return false;
}
//----------------------------------------------------------------------------

unii_buffer *unii_alloc( int size )
{
    unii_buffer *ub = new unii_buffer;
    ub->size = size;
    ub->pData = new char[ size ];
    ub->nPhysParts = 0;
    return ub;
}
//----------------------------------------------------------------------------

void unii_free( unii_buffer *pbuf )
{
    delete[] reinterpret_cast< char * >( pbuf->pData );
    delete pbuf;
}
//----------------------------------------------------------------------------

void *unii_data( unii_buffer *pbuf, uint32_t vmeAddr )
{
    UNUSED( vmeAddr );
    return pbuf->pData;
}
//----------------------------------------------------------------------------

int unii_init( const char *local_ip, const char *server_ip, bool debug )
{
    return core->init( local_ip, server_ip, debug );
}
//----------------------------------------------------------------------------

int unii_open( char *path )
{
    const char *vme_path = "/devices/hl";
    if( strncmp( vme_path, path, strlen( vme_path ) ) == 0 )
    {
        char *end;
        int fd = strtol( &path[ strlen( vme_path ) ], &end, 16 );
        if( cellAvailable( fdToCell( fd ) ) )
            return fd;
    }
    errno = ENODEV;
    return -1;
}
//----------------------------------------------------------------------------

int unii_reset( int fd )
{
    if( fd == 0 )
        return 0;

    // reset CLBUS controller and all DSP
    uint32_t val = 0xFFFFFFFF;
    int res = unii_write_reg( fd, 0x00000161, val );
    if( res < 0 )
        return res;

    // reset FPGA1
    val = 0xFFFFFFFF;
    res = unii_write_reg( fd, 0x00000000, val );
    if( res < 0 )
        return res;

    // reset FPGA2
    val = 0xFFFFFFFF;
    res = unii_write_reg( fd, 0x00004000, val );
    if( res < 0 )
        return res;

    return res;
}
//----------------------------------------------------------------------------

int unii_read( int fd, uint32_t addr, int count, void *buffer, uint32_t opflags )
{
    UNUSED( opflags );
    DSP_CELL cell = ( fd == 0 ) ? addrToCell( addr ) : fdToCell( fd );
    int ret = core->read( cell, DSP_CMD::READ_BUS, makeAddr( addr ), reinterpret_cast< uint8_t * >( buffer ), count );
    return ( ret < 0 ) ? ret : EXIT_SUCCESS;
}
//----------------------------------------------------------------------------

int unii_read32a32( int fd, uint32_t addr32, int count32, void *buffer )
{
    return unii_read( fd, addr32, count32 * sizeof( uint32_t ), buffer, 0 );
}
//----------------------------------------------------------------------------

int unii_read32a32ex( int fd, uint32_t addr32, int count32, void *buffer, int opflags )
{
    return unii_read( fd, addr32, count32 * sizeof( uint32_t ), buffer, opflags );
}
//----------------------------------------------------------------------------

int unii_write( int fd, uint32_t addr, int count, void *buffer, uint32_t opflags )
{
    UNUSED( opflags );
    DSP_CELL cell = ( fd == 0 ) ? addrToCell( addr ) : fdToCell( fd );
    int ret = core->write( cell, DSP_CMD::WRITE_BUS, makeAddr( addr ), reinterpret_cast< uint8_t * >( buffer ), count );
    return ( ret < 0 ) ? ret : EXIT_SUCCESS;
}
//----------------------------------------------------------------------------

int unii_write32a32( int fd, uint32_t addr32, int count32, void *buffer )
{
    return unii_write( fd, addr32, count32 * sizeof( uint32_t ), buffer, 0 );
}
//----------------------------------------------------------------------------

int unii_write32a32ex( int fd, uint32_t addr32, int count32, void *buffer, int opflags )
{
    return unii_write( fd, addr32, count32 * sizeof( uint32_t ), buffer, opflags );
}
//----------------------------------------------------------------------------

int unii_dma_read( int fd, uint32_t addr, int count, unii_buffer *pbuf, uint32_t opflags )
{
    UNUSED( opflags );
    DSP_CELL cell = ( fd == 0 ) ? addrToCell( addr ) : fdToCell( fd );
    int ret = core->read( cell, DSP_CMD::READ_BUS, makeAddr( addr ), reinterpret_cast< uint8_t * >( pbuf->pData ), count );
    return ( ret < 0 ) ? ret : EXIT_SUCCESS;
}
//----------------------------------------------------------------------------

int unii_dma_read32a32( int fd, uint32_t addr32, int count32, unii_buffer *pbuf )
{
    return unii_dma_read( fd, addr32, count32 * sizeof( uint32_t ), pbuf, 0 );
}
//----------------------------------------------------------------------------

int unii_dma_read32a32ex( int fd, uint32_t addr32, int count32, unii_buffer *pbuf, int opflags )
{
    return unii_dma_read( fd, addr32, count32 * sizeof( uint32_t ), pbuf, opflags );
}
//----------------------------------------------------------------------------

int unii_dma_write( int fd, uint32_t addr, int count, unii_buffer *pbuf, uint32_t opflags )
{
    UNUSED( opflags );
    DSP_CELL cell = ( fd == 0 ) ? addrToCell( addr ) : fdToCell( fd );
    int ret = core->write( cell, DSP_CMD::WRITE_BUS, makeAddr( addr ), reinterpret_cast< uint8_t * >( pbuf->pData ), count );
    return ( ret < 0 ) ? ret : EXIT_SUCCESS;
}
//----------------------------------------------------------------------------

int unii_dma_write32a32( int fd, uint32_t addr32, int count32, unii_buffer *pbuf )
{
    return unii_dma_write( fd, addr32, count32 * sizeof( uint32_t ), pbuf, 0 );
}
//----------------------------------------------------------------------------

int unii_dma_write32a32ex( int fd, uint32_t addr32, int count32, unii_buffer *pbuf, int opflags )
{
    return unii_dma_write( fd, addr32, count32 * sizeof( uint32_t ), pbuf, opflags );
}
//----------------------------------------------------------------------------

int unii_read_reg( int fd, uint32_t addr, uint32_t *value )
{
    int ret = core->read( fdToCell( fd ), DSP_CMD::READ_REG, addr, reinterpret_cast< uint8_t * >( value ), sizeof( uint32_t ) );
    return ( ret < 0 ) ? ret : EXIT_SUCCESS;
}
//----------------------------------------------------------------------------

int unii_write_reg( int fd, uint32_t addr, uint32_t value )
{
    int ret = core->write( fdToCell( fd ), DSP_CMD::WRITE_REG, addr, reinterpret_cast< uint8_t * >( &value ), sizeof( uint32_t ) );
    return ( ret < 0 ) ? ret : EXIT_SUCCESS;
}
//----------------------------------------------------------------------------

int unii_register_irq( int fd, IRQ_callback icb )
{
    return core->registerIRQCallback( fdToCell( fd ), icb );
}
//----------------------------------------------------------------------------

int unii_unregister_irq( int fd )
{
    return core->unregisterIRQCallback( fdToCell( fd ) );
}
//----------------------------------------------------------------------------

int unii_register_status( int fd, STATUS_callback scb )
{
    return core->registerSTATUSCallback( fdToCell( fd ), scb );
}
//----------------------------------------------------------------------------

int unii_unregister_status( int fd )
{
    return core->unregisterSTATUSCallback( fdToCell( fd ) );
}
//----------------------------------------------------------------------------
