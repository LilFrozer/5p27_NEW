#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <cstdint>

enum class DSP_CMD : uint8_t
{
    WRITE_REG = 0x1,
    READ_REG = 0x2,
    WRITE_BUS = 0x3,
    READ_BUS = 0x4
};

enum class DSP_RESPONSE : uint8_t
{
    IRQ = 0x5,
    STATUS = 0x6,
    WRITE_REG = 0x81,
    READ_REG = 0x82,
    WRITE_BUS = 0x83,
    READ_BUS = 0x84
};

enum class BUS_WIDTH : uint8_t
{
    SIZE32,
    SIZE64,
    SIZE128
};

enum class PACKET_RETRY : uint8_t
{
    OK,
    RETRY
};

enum class BUFF_FULLNESS : uint8_t
{
    LESS_HALF,
    GREAT_HALF
};

enum class BUS_TIMEOUT : uint8_t
{
    OK,
    TIMEOUT
};

enum class PACKET_LOST : uint8_t
{
    OK,
    LOST
};

enum class TOP_STATUS : uint8_t
{
    NOT_READY = 0x0,
    READY = 0x1,
    WORK = 0x2,
    ERROR = 0x4,
    CRIT_ERROR = 0x5
};

enum class DSP_CELL : uint16_t
{
    NONE = 0xDEAD,
    CELL0 = 10003,
    CELL1 = 10004,
    CELL2 = 10005,
    CELL3 = 10006,
    CELL4 = 10007,
    CELL5 = 10008,
    CELL6 = 10009,
    CTRL = 10010
};

const uint32_t BROADCAST_ADDR = 0x0C000000;
const uint32_t DSP0_ADDR = 0x10000000;
const uint32_t DSP1_ADDR = 0x14000000;
const uint32_t DSP2_ADDR = 0x18000000;
const uint32_t DSP3_ADDR = 0x1C000000;
const uint32_t SDRAM_ADDR = 0x30000000;

const char dspApiMulticastIP[] = "224.2.2.4";
const uint16_t dspApiServerPort = 10002;
const uint16_t dspApiMcastServerPort = 10012;
const uint32_t maxPayloadBytes = 1472;

#pragma pack( push, 1 )

struct REQUEST
{
    DSP_CMD CMD;
    BUS_WIDTH W		  : 2;
    uint8_t RESERVED1 : 6;
    uint16_t COUNT;
    uint32_t ID;
    uint32_t ADDR;
    uint32_t RESERVED2;
    uint32_t DATA[ 0 ];
};

struct MEMORY
{
    PACKET_RETRY R	   : 1;
    BUFF_FULLNESS F	   : 1;
    BUS_TIMEOUT E	   : 1;
    PACKET_LOST L	   : 1;
    uint32_t RESERVED1 : 20;
    uint32_t ID;
    uint32_t RESERVED2;
    uint32_t RESERVED3;
    uint32_t DATA[ 0 ];
};

struct STATUS
{
    TOP_STATUS TS;
    uint16_t ADDR;
    uint32_t STATUS_INT;
    uint32_t STATUS_EXT;
    uint32_t STATUS_DSP;
};

struct IRQ
{
    uint8_t RESERVED1;
    uint16_t INT;
    uint32_t RESERVED2;
    uint32_t RESERVED3;
    uint32_t RESERVED4;
};

struct RESPONSE
{
    DSP_RESPONSE RESP;
    union
    {
        MEMORY memory;
        STATUS status;
        IRQ irq;
    };
};

struct API_HEADER
{
    DSP_CELL cell;
    uint16_t reserved;
};

struct API_REQUEST
{
    API_HEADER header;
    REQUEST request;
};

struct API_RESPONSE
{
    API_HEADER header;
    RESPONSE response;
};

#pragma pack( pop )

#endif
