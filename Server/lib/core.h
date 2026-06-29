#ifndef CORE_H_INCLUDED
#define CORE_H_INCLUDED

#include <arpa/inet.h>
#include <cstdint>
#include <errno.h>
#include <map>
#include <mutex>
#include <net/if.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h> // memset, memcpy
#include <sys/socket.h>
#include <sys/types.h>
#include <thread>
#include <unistd.h>

#include "protocol.h"

#define LOG( level, what ) \
    if( level )            \
    {                      \
        what;              \
    }

const long recvTimeoutUSec = 100000; // microsec
const int maxRetry = 3;

using IRQ_callback = void ( * )( uint8_t *data, ssize_t size );
using STATUS_callback = void ( * )( uint8_t *data, ssize_t size );
using IRQ_callbacks = std::map< DSP_CELL, IRQ_callback >;
using STATUS_callbacks = std::map< DSP_CELL, STATUS_callback >;

class Core
{
    static Core *instance;

    int unicastSocket{ -1 };
    int mcastSocket{ -1 };

    sockaddr_in dspApiServerAddr{};
    bool isInit{ false };
    bool debug{ false };

    uint8_t recvBuf[ maxPayloadBytes ] = {};
    uint8_t sendBuf[ maxPayloadBytes ] = {};
    uint8_t mcastBuf[ maxPayloadBytes ] = {};

    std::mutex rw_mutex;

    IRQ_callbacks irq_callbacks;
    STATUS_callbacks status_callbacks;
    std::mutex irq_mutex;
    std::mutex status_mutex;

    std::thread mcastThread;

    Core();
    ~Core();

    int initUnicastSocket();
    int initMulticastSocket( const char *local_ip );

    void mcastThreadFunc();

    bool connectedToServer();

  public:
    static Core *getInstance();

    Core( const Core & ) = delete;
    Core &operator=( const Core & ) = delete;

    int init( const char *local_ip, const char *server_ip, bool debug = false );

    ssize_t read( DSP_CELL cell, DSP_CMD cmd, uint32_t address, uint8_t *data, int sizeBytes );
    ssize_t write( DSP_CELL cell, DSP_CMD cmd, uint32_t address, const uint8_t *data, int sizeBytes );

    int registerIRQCallback( DSP_CELL cell, IRQ_callback cb );
    int registerSTATUSCallback( DSP_CELL cell, STATUS_callback cb );
    int unregisterIRQCallback( DSP_CELL cell );
    int unregisterSTATUSCallback( DSP_CELL cell );
};

#endif // CORE_H_INCLUDED
