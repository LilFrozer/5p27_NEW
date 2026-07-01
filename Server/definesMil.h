
#pragma once

#include <thread>
#include <atomic>
#include <vector>
#include <QObject>
#include <QDebug>
#include <fcntl.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "farbos/proto.h"

// !!!IMITATOR!!!
//#define IMIT
#define PATH_HARD "/po/complex/tech_new/furke/hard.json"
#define BUFFER_SIZE 100000
#ifdef IMIT
    // some info: 043 -> ячейка, где лежит ОС(kpda2021),
    // як -> ячейка управления(техно. канал + канал данных)
    #define O43_ADDR "192.168.58.100"
    #define YAK_ADDR "192.168.58.100"
    #define SERVER_ADDR "192.168.58.100"
#else
    #define O43_ADDR "192.0.0.100"
    #define YAK_ADDR "192.0.0.115"
    #define SERVER_ADDR "190.0.3.34"
#endif


using u32 = unsigned;
using u16 = uint16_t;
using u8 = uint8_t;

namespace Constants
{
const u16 O43_PORT = 10000;
const u16 YAK_PORT = 10001;
const u16 SERVER_PORT = 1212;
const u16 VIOD_MAX_PAYLOAD_WORDS = 89;
const u16 MAX_KNS = 255;
const u32 SERVER_HASH = 0x228;
const u32 MAX_SIZE_PACKET_DATA = 1400;
const u32 AMOUNT_STATUSES_CSS = 16;
const u32 AMOUNT_SUM = 3;
const u32 AMOUNT_STATUSES_SUM = 32;
}

struct qword
{
    u32 word_0;
    u32 word_1;
    u32 word_2;
    u32 word_3;
};

class IViodSender
{
public:
    virtual void sendViodMsg(std::vector<qword> &data,
                                const u16 smid,
                                const u16 spid,
                                const u16 slid,
                                const u16 dmid,
                                const u16 dpid,
                                const u16 dlid,
                                const u16 chid) = 0;
    virtual void rcvLoop() = 0;
    virtual ~IViodSender() {}
};

struct far_data
{
    u32 status_css[16]{};
    u32 status_sum[3][32]{};
    u32 ki_kd{};
    std::vector<u32> channel1{};
    std::vector<u32> channel2{};
    std::vector<u32> channel3{};
    std::vector<u32> channel4{};
    std::vector<u32> channel5{};
    far_data() :
        channel1(BUFFER_SIZE, 0),
        channel2(BUFFER_SIZE, 0),
        channel3(BUFFER_SIZE, 0),
        channel4(BUFFER_SIZE, 0),
        channel5(BUFFER_SIZE, 0) {}
};

