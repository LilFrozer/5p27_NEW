
#pragma once

#include "definesMil.h"
#include <QTimer>

// !!! -> Имитатор работает только на системных сокетах -> with class OpenApiViod !!!
class ViodImit : public IViodSender
{
private:
    std::thread rcv_thread_;
    std::atomic<bool> running_{true};

    std::unique_ptr<u32[]> send_buf_;
    std::unique_ptr<u32[]> recv_buf_;

    int imit_sock{-1};
    std::string s_imit_addr{YAK_ADDR};
    u16 imit_port{Constants::YAK_PORT};
    sockaddr_in imit_addr_{};

    std::thread send_thread_;
    std::atomic<bool> send_running_{true};

    std::string s_o43_addr_{O43_ADDR};
    u16 o43_port{Constants::O43_PORT};
    sockaddr_in o43addr_{};
public:
    ViodImit();
    virtual ~ViodImit() override;
    void sendViodMsg(std::vector<qword> &data,
                        const u16 smid,
                        const u16 spid,
                        const u16 slid,
                        const u16 dmid,
                        const u16 dpid,
                        const u16 dlid,
                        const u16 chid) override final;
    void rcvLoop() override final;
    void sendLoop();
    ssize_t SendUdpBE(const size_t size32, const sockaddr_in *addr);
    ssize_t SendUdpBE(u32 *data, const size_t size32, const sockaddr_in *addr);
    ssize_t RecvUdpBE(u32* data, size_t size32);
    ssize_t RecvUdpBE(size_t size32);
};
