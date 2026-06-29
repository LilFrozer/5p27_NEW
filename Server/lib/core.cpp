#include "core.h"

Core *Core::instance = nullptr;

Core::Core()
{
}
//----------------------------------------------------------------------------

int Core::init( const char *local_ip, const char *server_ip, bool debug )
{
    this->debug = debug;

    unicastSocket = initUnicastSocket();
    if( unicastSocket == -1 )
        return -1;

    mcastSocket = initMulticastSocket( local_ip );
    if( mcastSocket == -1 )
        return -1;

    dspApiServerAddr.sin_family = AF_INET;
    dspApiServerAddr.sin_port = htons( dspApiServerPort );
    inet_pton( AF_INET, server_ip, &dspApiServerAddr.sin_addr );

    mcastThread = std::thread( &Core::mcastThreadFunc, this );

    isInit = true;
    return 0;
}
//----------------------------------------------------------------------------

Core::~Core()
{
    close( unicastSocket );
    close( mcastSocket );
}
//----------------------------------------------------------------------------

int Core::initUnicastSocket()
{
    int sd = socket( AF_INET, SOCK_DGRAM, IPPROTO_UDP );
    if( sd < 0 )
    {
        LOG( debug, perror( "dspapi socket" ) );
        return -1;
    }

    int val = 1;
    if( setsockopt( sd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof( val ) ) < 0 )
    {
        LOG( debug, perror( "dspapi socket setsockopt SO_REUSEADDR" ) );
        close( sd );
        return -1;
    }

    timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = recvTimeoutUSec;
    if( setsockopt( sd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof( tv ) ) < 0 )
    {
        LOG( debug, perror( "dspapi socket setsockopt SO_RCVTIMEO" ) );
        close( sd );
        return -1;
    }

    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = 0;

    if( bind( sd, ( sockaddr * )&addr, sizeof( sockaddr_in ) ) < 0 )
    {
        LOG( debug, perror( "dspapi socket bind" ) );
        close( sd );
        return -1;
    }

    return sd;
}
//----------------------------------------------------------------------------

int Core::initMulticastSocket( const char *local_ip )
{
    int sd = socket( AF_INET, SOCK_DGRAM, IPPROTO_UDP );
    if( sd < 0 )
    {
        LOG( debug, perror( "dspapi multicast socket" ) );
        return -1;
    }

    int val = 1;
    if( setsockopt( sd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof( val ) ) < 0 )
    {
        LOG( debug, perror( "dspapi multicast socket setsockopt SO_REUSEADDR" ) );
        close( sd );
        return -1;
    }

    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons( dspApiMcastServerPort );
    inet_pton( AF_INET, dspApiMulticastIP, &addr.sin_addr );

    if( bind( sd, ( sockaddr * )&addr, sizeof( sockaddr_in ) ) < 0 )
    {
        LOG( debug, perror( "dspapi multicast socket bind" ) );
        close( sd );
        return -1;
    }

    struct ip_mreq mreq;
    inet_pton( AF_INET, dspApiMulticastIP, &mreq.imr_multiaddr.s_addr );
    inet_pton( AF_INET, local_ip, &mreq.imr_interface.s_addr );

    if( setsockopt( sd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof( mreq ) ) < 0 )
    {
        LOG( debug, perror( "dspapi multicast socket setsockopt IP_ADD_MEMBERSHIP" ) );
        close( sd );
        return -1;
    }

    if( setsockopt( sd, IPPROTO_IP, IP_MULTICAST_IF, &mreq.imr_interface.s_addr, sizeof( in_addr ) ) < 0 )
    {
        LOG( debug, perror( "dspapi multicast socket setsockopt IP_MULTICAST_IF" ) );
        close( sd );
        return -1;
    }

    return sd;
}
//----------------------------------------------------------------------------

void Core::mcastThreadFunc()
{
    while( 1 )
    {
        API_RESPONSE *ar = reinterpret_cast< API_RESPONSE * >( mcastBuf );
        memset( ar, 0, sizeof( API_RESPONSE ) );

        ssize_t recvBytes = recvfrom( mcastSocket, mcastBuf, sizeof( mcastBuf ), 0, 0, 0 );
        if( recvBytes < 0 )
        {
            LOG( debug, perror( "dspapi multicast socket recvfrom" ) );
            break;
        }

        if( ar->response.RESP == DSP_RESPONSE::IRQ )
        {
            if( irq_callbacks.find( ar->header.cell ) != irq_callbacks.end() )
            {
                irq_callbacks[ ar->header.cell ]( reinterpret_cast< uint8_t * >( &ar->response ), sizeof( RESPONSE ) );
            }
        }
        else
        {
            if( status_callbacks.find( ar->header.cell ) != status_callbacks.end() )
            {
                status_callbacks[ ar->header.cell ]( reinterpret_cast< uint8_t * >( &ar->response ), sizeof( RESPONSE ) );
            }
        }
    }
    LOG( debug, printf( "dspapi quit multicast recieve thread\n" ) );
}
//----------------------------------------------------------------------------

bool Core::connectedToServer()
{
    API_REQUEST api_req{};
    api_req.header.cell = DSP_CELL::NONE;

    if( sendto( unicastSocket, &api_req, sizeof( api_req ), 0,
                reinterpret_cast< sockaddr * >( &dspApiServerAddr ), sizeof( sockaddr_in ) ) < 0 )
    {
        LOG( debug, perror( "dspapi socket sendto" ) );
    }

    if( recvfrom( unicastSocket, recvBuf, maxPayloadBytes, 0, 0, 0 ) < 0 )
    {
        LOG( debug, perror( "dspapi socket recvfrom" ) );
        return false;
    }

    return true;
}
//----------------------------------------------------------------------------

Core *Core::getInstance()
{
    if( !instance )
        instance = new Core();
    return instance;
}
//----------------------------------------------------------------------------

ssize_t Core::read( DSP_CELL cell, DSP_CMD cmd, uint32_t address, uint8_t *data, int sizeBytes )
{
    std::lock_guard< std::mutex > guard( rw_mutex );

    if( !isInit )
    {
        errno = ENONET;
        return -1;
    }

    if( cell == DSP_CELL::NONE )
    {
        errno = ENODEV;
        return -1;
    }

    if( cell == DSP_CELL::CTRL && cmd == DSP_CMD::READ_BUS )
    {
        errno = ENOMEM;
        return -1;
    }

    API_REQUEST api_req{};
    api_req.header.cell = cell;
    api_req.request.CMD = cmd;
    api_req.request.W = BUS_WIDTH::SIZE32;
    api_req.request.ADDR = address;

    int maxDataSizeBytes = maxPayloadBytes - sizeof( API_REQUEST );
    int offsetBytes = 0;
    int retryCounter = 0;
    int remainedBytes = sizeBytes;

    while( remainedBytes > 0 )
    {
        api_req.request.COUNT = std::min( remainedBytes, maxDataSizeBytes ) / sizeof( uint32_t );

        if( sendto( unicastSocket, &api_req, sizeof( api_req ), 0,
                    reinterpret_cast< sockaddr * >( &dspApiServerAddr ), sizeof( sockaddr_in ) ) < 0 )
        {
            LOG( debug, perror( "dspapi socket sendto" ) );
            return -1;
        }

        API_RESPONSE *api_resp = reinterpret_cast< API_RESPONSE * >( recvBuf );
        memset( api_resp, 0, sizeof( API_RESPONSE ) );
        ssize_t recvBytes = 0;

        while( 1 )
        {
            recvBytes = recvfrom( unicastSocket, recvBuf, maxPayloadBytes, 0, 0, 0 );
            if( recvBytes < 0 )
            {
                LOG( debug, perror( "dspapi socket recvfrom" ) );

                if( errno == EAGAIN ) // timeout
                {
                    if( !connectedToServer() )
                    {
                        errno = EPIPE;
                    }
                }
                return -1;
            }

            if( api_resp->response.RESP == DSP_RESPONSE::READ_REG || api_resp->response.RESP == DSP_RESPONSE::READ_BUS )
            {
                break;
            }
            else
            {
                errno = EBADMSG;
                return -1;
            }
        }

        if( api_resp->response.memory.R == PACKET_RETRY::RETRY || api_resp->response.memory.E == BUS_TIMEOUT::TIMEOUT )
        {
            if( ++retryCounter == maxRetry )
            {
                errno = EBUSY;
                return -1;
            }
            continue;
        }

        memcpy( data + offsetBytes, recvBuf + sizeof( API_RESPONSE ), recvBytes - sizeof( API_RESPONSE ) );

        api_req.request.ADDR += ( recvBytes - sizeof( API_RESPONSE ) ) / sizeof( uint32_t );
        offsetBytes += recvBytes - sizeof( API_RESPONSE );
        remainedBytes -= recvBytes - sizeof( API_RESPONSE );
    }

    return sizeBytes;
}
//----------------------------------------------------------------------------

ssize_t Core::write( DSP_CELL cell, DSP_CMD cmd, uint32_t address, const uint8_t *data, int sizeBytes )
{
    std::lock_guard< std::mutex > guard( rw_mutex );

    if( !isInit )
    {
        errno = ENONET;
        return -1;
    }

    if( cell == DSP_CELL::NONE )
    {
        errno = ENODEV;
        return -1;
    }

    if( cell == DSP_CELL::CTRL && cmd == DSP_CMD::WRITE_BUS )
    {
        errno = ENOMEM;
        return -1;
    }

    API_REQUEST api_req{};
    api_req.header.cell = cell;
    api_req.request.CMD = cmd;
    api_req.request.W = BUS_WIDTH::SIZE32;
    api_req.request.ADDR = address;

    int maxDataSizeBytes = maxPayloadBytes - sizeof( API_REQUEST );
    int offsetBytes = 0;
    int retryCounter = 0;
    int remainedBytes = sizeBytes;

    while( remainedBytes > 0 )
    {
        api_req.request.COUNT = std::min( remainedBytes, maxDataSizeBytes ) / sizeof( uint32_t );

        memcpy( sendBuf, &api_req, sizeof( API_REQUEST ) );
        memcpy( sendBuf + sizeof( API_REQUEST ), data + offsetBytes, api_req.request.COUNT * sizeof( uint32_t ) );

        ssize_t sendBytes = sendto( unicastSocket, sendBuf, sizeof( API_REQUEST ) + api_req.request.COUNT * sizeof( uint32_t ), 0,
                                    reinterpret_cast< sockaddr * >( &dspApiServerAddr ), sizeof( sockaddr_in ) );
        if( sendBytes == -1 )
        {
            LOG( debug, perror( "dspapi socket sendto" ) );
            return -1;
        }

        API_RESPONSE *api_resp = reinterpret_cast< API_RESPONSE * >( recvBuf );
        memset( api_resp, 0, sizeof( API_RESPONSE ) );

        while( 1 )
        {
            if( recvfrom( unicastSocket, recvBuf, maxPayloadBytes, 0, 0, 0 ) <= 0 )
            {
                LOG( debug, perror( "dspapi socket recvfrom" ) );

                if( errno == EAGAIN ) // timeout
                {
                    if( !connectedToServer() )
                    {
                        errno = EPIPE;
                    }
                }
                return -1;
            }

            if( api_resp->response.RESP == DSP_RESPONSE::WRITE_REG || api_resp->response.RESP == DSP_RESPONSE::WRITE_BUS )
            {
                break;
            }
            else
            {
                errno = EBADMSG;
                return -1;
            }
        }

        if( api_resp->response.memory.R == PACKET_RETRY::RETRY || api_resp->response.memory.E == BUS_TIMEOUT::TIMEOUT )
        {
            if( ++retryCounter == maxRetry )
            {
                errno = EBUSY;
                return -1;
            }
            continue;
        }

        api_req.request.ADDR += ( sendBytes - sizeof( API_REQUEST ) ) / sizeof( uint32_t );
        offsetBytes += sendBytes - sizeof( API_REQUEST );
        remainedBytes -= sendBytes - sizeof( API_REQUEST );
    }

    return sizeBytes;
}
//----------------------------------------------------------------------------

int Core::registerIRQCallback( DSP_CELL cell, IRQ_callback cb )
{
    std::lock_guard< std::mutex > guard( irq_mutex );
    if( !isInit )
    {
        errno = ENONET;
        return -1;
    }
    irq_callbacks[ cell ] = cb;
    return 0;
}
//----------------------------------------------------------------------------

int Core::registerSTATUSCallback( DSP_CELL cell, STATUS_callback cb )
{
    std::lock_guard< std::mutex > guard( status_mutex );
    if( !isInit )
    {
        errno = ENONET;
        return -1;
    }
    status_callbacks[ cell ] = cb;
    return 0;
}
//----------------------------------------------------------------------------

int Core::unregisterIRQCallback( DSP_CELL cell )
{
    std::lock_guard< std::mutex > guard( irq_mutex );
    if( !isInit )
    {
        errno = ENONET;
        return -1;
    }
    irq_callbacks.erase( cell );
    return 0;
}
//----------------------------------------------------------------------------

int Core::unregisterSTATUSCallback( DSP_CELL cell )
{
    std::lock_guard< std::mutex > guard( status_mutex );
    if( !isInit )
    {
        errno = ENONET;
        return -1;
    }
    status_callbacks.erase( cell );
    return 0;
}
//----------------------------------------------------------------------------
