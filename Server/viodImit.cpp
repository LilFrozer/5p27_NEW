#include "definesMil.h"
#include "openapiviod.h"
#include "viodImit.h"

#ifdef IMIT

ViodImit::ViodImit() :
    send_buf_{std::make_unique<u32[]>(1472)}
    , recv_buf_{std::make_unique<u32[]>(1472)}
{
    imit_sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (imit_sock < 0)
        throw std::runtime_error("imit_sock creation failed");

    int val = 1;
    int res = setsockopt(imit_sock, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));

    imit_addr_.sin_family = AF_INET;
    inet_pton(AF_INET, s_imit_addr.c_str(), &imit_addr_.sin_addr);
    imit_addr_.sin_port = htons(imit_port);

    if (bind(imit_sock, (const sockaddr*)&imit_addr_, sizeof(imit_addr_)) < 0) {
        close(imit_sock);
        throw std::runtime_error("imit_sock bind failed");
    }

    o43addr_.sin_family = AF_INET;
    inet_pton(AF_INET, s_o43_addr_.c_str(), &o43addr_.sin_addr);
    o43addr_.sin_port = htons(o43_port);

    running_ = true;
    rcv_thread_ = std::thread(&ViodImit::rcvLoop, this);
    rcv_thread_.detach();

    qDebug() << "imit rcv started...";
}

ViodImit::~ViodImit()
{
    running_ = false;
    if (rcv_thread_.joinable())
        rcv_thread_.join();
    send_running_ = false;
    if (send_thread_.joinable())
        send_thread_.join();
    if (imit_sock>=0) {
        ::close(imit_sock);
        imit_sock = -1;
    }
}

ssize_t ViodImit::SendUdpBE(const size_t size32, const sockaddr_in *addr)
{
    auto temp = send_buf_.get();
    for (size_t i = 0; i < size32; ++i)
        temp[i] = htonl(temp[i]);

    ssize_t bytes_sended = sendto(imit_sock, (char*)temp, size32 * 4, 0,
                                (sockaddr*)addr, sizeof(sockaddr_in));
    memset(temp, 0, size32 * 4);
    return bytes_sended;
}

ssize_t ViodImit::SendUdpBE(uint32_t *data, const size_t size32, const sockaddr_in *addr)
{
    auto temp = send_buf_.get();
    for (size_t i = 0; i < size32; ++i, ++data)
        temp[i] = htonl(*data);

    ssize_t bytes_sended = sendto(imit_sock, (char*)temp, size32 * 4, 0,
                                (sockaddr*)addr, sizeof(sockaddr_in));
    return bytes_sended;
}

ssize_t ViodImit::RecvUdpBE(uint32_t* data, size_t size32)
{
    sockaddr_in client_addr;
    int s_size = sizeof(sockaddr_in);
    ssize_t recv_bytes = recvfrom(imit_sock, (char*)data, size32 * 4, 0,
                                (sockaddr*)&client_addr, (socklen_t*)&s_size);

    for (ssize_t i = 0; i < recv_bytes / 4; ++i, ++data)
        *data = ntohl(*data);

    return recv_bytes;
}

ssize_t ViodImit::RecvUdpBE(size_t size32)
{
    sockaddr_in client_addr;
    auto data = recv_buf_.get();
    int s_size = sizeof(sockaddr_in);
    ssize_t recv_bytes = recvfrom(imit_sock, (char*)data, size32 * 4, 0,
                                (sockaddr*)&client_addr, (socklen_t*)&s_size);

    for (ssize_t i = 0; i < recv_bytes / 4; ++i, ++data)
        *data = ntohl(*data);

    return recv_bytes;
}

void ViodImit::rcvLoop()
{
    using fb_hdr = proto_farbos::farbos_header;
    std::vector<qword> data{};
    while (running_) {
        data.clear();
        int length = 0;
        auto header = reinterpret_cast<OPEN_API_VIOD::AbstractHeader*>(recv_buf_.get());
        memset(header, 0, 1472);

        auto size = RecvUdpBE(recv_buf_.get(), 1472 / 4);
        if (size==0) {
            qDebug() << "[ViodImit::Rcv]receive error or timeout:" << (size < 0 ? strerror(errno) : "no data");
            continue;
        }

        auto full_header = reinterpret_cast<OPEN_API_VIOD::ViodFullFrame*>(recv_buf_.get());
        auto data_in = reinterpret_cast<qword*>(full_header + 1);
        length = full_header->viod_header.length;

        // !!!start&stop!!!
        if (full_header->viod_header.addr_info.dmid == 4 &&
            full_header->viod_header.addr_info.dpid == 1 &&
            full_header->viod_header.addr_info.dlid == 1 &&
            full_header->viod_header.chid == 3)
        {
            switch (data_in[0].word_0) {
            case 1: {
                send_running_ = true;
                send_thread_ = std::thread(&ViodImit::sendLoop, this);
                send_thread_.detach();
                break;
            }
            case 0: {
                send_running_ = false;
                if (send_thread_.joinable())
                    send_thread_.join();
                break;
            }
            }
        }

//        fb_hdr *hdr = reinterpret_cast<fb_hdr*>(&data_in[0]);
//        switch (hdr->tk) {
//        default: {
//            qDebug() << "[ViodImit::Rcv]unknown header:" << hdr->tk;
//            break;
//        }
//        }
    }
}

void ViodImit::sendViodMsg(std::vector<qword> &data,
                            const u16 smid,
                            const u16 spid,
                            const u16 slid,
                            const u16 dmid,
                            const u16 dpid,
                            const u16 dlid,
                            const u16 chid)
{
    size_t kviodmingmssize = sizeof(OPEN_API_VIOD::DataHeadStruct)+sizeof(OPEN_API_VIOD::ViodHeadStruct);
    auto data_ptr = send_buf_.get() + kviodmingmssize / sizeof(unsigned);
    auto msg_header = reinterpret_cast<OPEN_API_VIOD::ViodFullFrame*>(send_buf_.get());
    auto id_counter = 0u;

    msg_header->data_header.cmd = static_cast<uint8_t>(0x07);
    msg_header->viod_header.prmbl = static_cast<uint16_t>(0xAA55);
    msg_header->viod_header.addr_info.smid = smid;
    msg_header->viod_header.addr_info.spid = spid;
    msg_header->viod_header.addr_info.slid = slid;
    msg_header->viod_header.addr_info.dmid = static_cast<uint16_t>(dmid);
    msg_header->viod_header.addr_info.dpid = dpid;
    msg_header->viod_header.addr_info.dlid = dlid;

    if (chid == 0 || chid == 1 || chid == 2 || chid == 3 || chid == 4)
        msg_header->viod_header.f = 0;
    msg_header->viod_header.e = 1;
    msg_header->viod_header.chid = 0xF&chid;
    msg_header->viod_header.id_port = 10001;

    u16 remaining_qwords = static_cast<u16>(data.size());
    u16 offset_qwords = 0;

    while(remaining_qwords > 0)
    {
        u16 qwords_to_send = (remaining_qwords < Constants::VIOD_MAX_PAYLOAD_WORDS)
                                   ? remaining_qwords
                                   : Constants::VIOD_MAX_PAYLOAD_WORDS;

        msg_header->viod_header.full_length = qwords_to_send;
        msg_header->viod_header.length = qwords_to_send;
        msg_header->data_header.mount = 1 + qwords_to_send;
        msg_header->data_header.id_count = id_counter++;

//        qword *transport_hdr = reinterpret_cast<qword*>(&msg_header->data_header);
//        qDebug() << "TRANSPORT_HDR[send]0x" << QString("%1").arg(transport_hdr->word_0, 8, 16, QChar('0')) << " "
//                    << "0x" << QString("%1").arg(transport_hdr->word_1, 8, 16, QChar('0')) << " "
//                    << "0x" << QString("%1").arg(transport_hdr->word_2, 8, 16, QChar('0')) << " "
//                    << "0x" << QString("%1").arg(transport_hdr->word_3, 8, 16, QChar('0'));

//        qword *viod_hdr = reinterpret_cast<qword*>(&msg_header->viod_header);
//        qDebug() << "VIOD_HDR[send]0x" << QString("%1").arg(viod_hdr->word_0, 8, 16, QChar('0')) << " "
//                    << "0x" << QString("%1").arg(viod_hdr->word_1, 8, 16, QChar('0')) << " "
//                    << "0x" << QString("%1").arg(viod_hdr->word_2, 8, 16, QChar('0')) << " "
//                    << "0x" << QString("%1").arg(viod_hdr->word_3, 8, 16, QChar('0'));

//        qDebug() << "DATA[send]";
//        for(size_t i{};i<data.size();++i)
//        {
//            qDebug() << "[send]0x" << QString("%1").arg(data[i].word_0, 8, 16, QChar('0')) << " "
//                    << "0x" << QString("%1").arg(data[i].word_1, 8, 16, QChar('0')) << " "
//                    << "0x" << QString("%1").arg(data[i].word_2, 8, 16, QChar('0')) << " "
//                    << "0x" << QString("%1").arg(data[i].word_3, 8, 16, QChar('0'));
//        }

        memcpy(data_ptr, data.data(), qwords_to_send * 16);

        auto raw_size32 = (kviodmingmssize / 4) + (remaining_qwords * 4);
        auto sended = SendUdpBE(raw_size32, &o43addr_);
        if (sended == 0)
            throw std::runtime_error("Cant send data\n");

        remaining_qwords -= qwords_to_send;
        offset_qwords += qwords_to_send;
    }
}

void ViodImit::sendLoop()
{
    srand(time(NULL));
    static int cnt = 0;
    while (send_running_) {
        if (cnt == 50000000) {
            std::vector<qword> dataStatuses{qword{0xa5d0000c, 0x05000500, 0x05000000, 0x00008fb7},
                    qword{0x88888888, static_cast<u32>(rand() % 1337), 0x00000002, 0x00000003},
                    qword{0x00000004, 0x0000005, 0x00000006, 0x00000007},
                    qword{0x00000008, 0x00000009, static_cast<u32>(rand() % 1337), 0x0000000b},

                    qword{0xa5c0000c, 0x05000500, 0x05000000, 0x00008fb7},
                    qword{0x88888888, static_cast<u32>(rand() % 1337), 0x00000002, 0x00000003},
                    qword{0x00000004, 0x0000005, 0x00000006, 0x00000007},
                    qword{static_cast<u32>(rand() % 1337), 0x00000009, 0x0000000a, 0x0000000b},
                    qword{0x00000008, 0x00000009, 0x0000000a, 0x0000000b},
                    qword{0x00000008, 0x00000009, static_cast<u32>(rand() % 1337), 0x0000000b},
                    qword{0x00000008, 0x00000009, 0x0000000a, 0x0000000b},
                    qword{0x00000008, static_cast<u32>(rand() % 1337), 0x0000000a, 0x0000000b},

                    qword{0xa5c0000c, 0x05000500, 0x05000000, 0x00008fb7},
                    qword{0x88888888, 0x00000001, 0x00000002, 0x00000003},
                    qword{static_cast<u32>(rand() % 1337), 0x0000005, 0x00000006, 0x00000007},
                    qword{0x00000008, 0x00000009, 0x0000000a, 0x0000000b},
                    qword{0x00000008, static_cast<u32>(rand() % 1337), 0x0000000a, 0x0000000b},
                    qword{0x00000008, 0x00000009, 0x0000000a, 0x0000000b},
                    qword{0x00000008, 0x00000009, static_cast<u32>(rand() % 1337), 0x0000000b},
                    qword{0x00000008, static_cast<u32>(rand() % 1337), 0x0000000a, 0x0000000b},

                    qword{0xa5c0000c, static_cast<u32>(rand() % 1337), 0x05000000, 0x00008fb7},
                    qword{0x88888888, 0x00000001, 0x00000002, 0x00000003},
                    qword{0x00000004, 0x0000005, 0x00000006, 0x00000007},
                    qword{static_cast<u32>(rand() % 1337), 0x00000009, 0x0000000a, 0x0000000b},
                    qword{0x00000008, 0x00000009, 0x0000000a, 0x0000000b},
                    qword{0x00000008, 0x00000009, static_cast<u32>(rand() % 1337), 0x0000000b},
                    qword{0x00000008, 0x00000009, 0x0000000a, 0x0000000b},
                    qword{0x00000008, static_cast<u32>(rand() % 1337), 0x0000000a, 0x0000000b}};
            sendViodMsg(dataStatuses, 0, 0, 0, 0, 0, 0, 5);

            std::vector<qword> data1{};
            for (int i{}; i < 80; i++) {
                qword qw{0, 0, 0, 0};
                short sin_val = static_cast<short>(rand() % 65536);
                short cos_val = static_cast<short>(rand() % 65536);
                u32 packed = (static_cast<u32>(cos_val) << 16) | (static_cast<u32>(sin_val) & 0xFFFF);
                qw.word_0 = packed;
                qw.word_1 = packed;
                qw.word_2 = packed;
                qw.word_3 = packed;
                data1.push_back(qw);
            }
            sendViodMsg(data1, 0, 0, 0, 0, 0, 0, 0);

            std::vector<qword> data2{};
            for (int i{}; i < 80; i++) {
                qword qw{0, 0, 0, 0};
                short sin_val = static_cast<short>(rand() % 65536);
                short cos_val = static_cast<short>(rand() % 65536);
                u32 packed = (static_cast<u32>(cos_val) << 16) | (static_cast<u32>(sin_val) & 0xFFFF);
                qw.word_0 = packed;
                qw.word_1 = packed;
                qw.word_2 = packed;
                qw.word_3 = packed;
                data2.push_back(qw);
            }
            sendViodMsg(data2, 0, 0, 0, 0, 0, 0, 1);

            std::vector<qword> data3{};
            for (int i{}; i < 80; i++) {
                qword qw{0, 0, 0, 0};
                short sin_val = static_cast<short>(rand() % 65536);
                short cos_val = static_cast<short>(rand() % 65536);
                u32 packed = (static_cast<u32>(cos_val) << 16) | (static_cast<u32>(sin_val) & 0xFFFF);
                qw.word_0 = packed;
                qw.word_1 = packed;
                qw.word_2 = packed;
                qw.word_3 = packed;
                data3.push_back(qw);
            }
            sendViodMsg(data3, 0, 0, 0, 0, 0, 0, 2);

            std::vector<qword> data4{};
            for (int i{}; i < 80; i++) {
                qword qw{0, 0, 0, 0};
                short sin_val = static_cast<short>(rand() % 65536);
                short cos_val = static_cast<short>(rand() % 65536);
                u32 packed = (static_cast<u32>(cos_val) << 16) | (static_cast<u32>(sin_val) & 0xFFFF);
                qw.word_0 = packed;
                qw.word_1 = packed;
                qw.word_2 = packed;
                qw.word_3 = packed;
                data4.push_back(qw);
            }
            sendViodMsg(data4, 0, 0, 0, 0, 0, 0, 3);

            std::vector<qword> data5{};
            for (int i{}; i < 80; i++) {
                qword qw{0, 0, 0, 0};
                short sin_val = static_cast<short>(rand() % 65536);
                short cos_val = static_cast<short>(rand() % 65536);
                u32 packed = (static_cast<u32>(cos_val) << 16) | (static_cast<u32>(sin_val) & 0xFFFF);
                qw.word_0 = packed;
                qw.word_1 = packed;
                qw.word_2 = packed;
                qw.word_3 = packed;
                data5.push_back(qw);
            }
            sendViodMsg(data5, 0, 0, 0, 0, 0, 0, 4);

            cnt = 0;
        }
        ++cnt;
    }
}

#endif
