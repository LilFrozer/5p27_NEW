#ifndef OPENAPIVIOD_H
#define OPENAPIVIOD_H

#include "definesMil.h"
#include <fcntl.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "farbos/proto.h"

namespace OPEN_API_VIOD
{

struct DataHeadStruct
{
    u32 cmd : 8;
    u32 rsrv1 : 8;
    u32 mount : 16;
    u32 id_count{};
    u32 rsrv2{};
    u32 rsrv3{};
};

struct ViodAddrInfo
{
    u16 slid : 2;   // -> с какого линкпорта отправлен пакет
    u16 spid : 2;   // -> с какого процессора отправлен пакет
    u16 smid : 4;   // -> с какой ячейки отправлен пакети
    u16 dlid : 2;   // ...все также, только куда отправлять пакет...
    u16 dpid : 2;
    u16 dmid : 4;
};

struct ViodHeadStruct
{
    ViodAddrInfo addr_info{};
    u16 prmbl{0xAA55};
    u16 rsrv1 : 2;
    u16 offset : 14;
    u16 length{};
    u16 full_length{};
    u16 chid : 4;
    u16 e : 1;
    u16 f : 1;
    u16 rsrv2 : 10;
    u16 id_port{};
    u16 id_addr{};
};

struct ViodInfoStruct
{
    u32 cmd : 8;        // 0
    u32 rsrv1 : 24;     // 3 2 1
    u32 id_count{};     // 7 6 5 4
    u32 rsrv2{};        // 11 10 9 8
    u32 delta_num{};    // 15 14 13 12
};

union AbstractHeader
{
    ViodInfoStruct info_head;
    DataHeadStruct data_head;
};

struct ViodFullFrame
{
    DataHeadStruct data_header{};
    ViodHeadStruct viod_header{};
};

}

class OpenApiViod : public ViodSender
{
    Q_OBJECT
protected:
    std::unique_ptr<u32[]> send_buf_;
    std::unique_ptr<u32[]> recv_buf_;

    // --- host ---
    int o43_sock_ = 0;
    std::string s_o43_addr_{O43_ADDR};
    u16 o43_port{10000};
    sockaddr_in o43addr_{};

    std::string s_yak_addr_{YAK_ADDR};
    u16 yak_port_{10001};
    sockaddr_in yakaddr_{};

    // --- data from far ---
    far_data far_data_{};
public:
    explicit OpenApiViod(QObject *parent = nullptr);
    virtual ~OpenApiViod();
    void sendViodMsg(std::vector<qword> &data,
                     const u16 smid,
                     const u16 spid,
                     const u16 slid,
                     const u16 dmid,
                     const u16 dpid,
                     const u16 dlid,
                     const u16 chid) override final;
    void rcvLoop() override final;
    ssize_t SendUdpBE(const size_t size32, const sockaddr_in *addr);
    ssize_t SendUdpBE(u32 *data, const size_t size32, const sockaddr_in *addr);
    ssize_t RecvUdpBE(u32* data, size_t size32);
    ssize_t RecvUdpBE(size_t size32);
    void sendRawData(u32 *ptr, size_t size);
    far_data &get_far_data() { return this->far_data_; }
};

#endif // OPENAPIVIOD_H
