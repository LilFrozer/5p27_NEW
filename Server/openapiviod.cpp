#include "openapiviod.h"

OpenApiViod::OpenApiViod(QObject *parent) :
    ViodSender{parent},
    send_buf_{std::make_unique<unsigned[]>(1472)},
    recv_buf_{std::make_unique<unsigned[]>(1472)}
{
    o43_sock_ = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (o43_sock_ < 0)
        throw std::runtime_error("Socket creation failed");

    int val = 1;
    int res = setsockopt(o43_sock_, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));

    o43addr_.sin_family = AF_INET;
    inet_pton(AF_INET, s_o43_addr_.c_str(), &o43addr_.sin_addr);
    o43addr_.sin_port = htons(o43_port);

    if (bind(o43_sock_, (const sockaddr*)&o43addr_, sizeof(o43addr_)) < 0) {
        close(o43_sock_);
        throw std::runtime_error("Bind failed");
    }

    yakaddr_.sin_family = AF_INET;
    inet_pton(AF_INET, s_yak_addr_.c_str(), &yakaddr_.sin_addr);
    yakaddr_.sin_port = htons(yak_port_);

    m_running = true;
    m_thread = std::thread(&OpenApiViod::rcvLoop, this);
    m_thread.detach();

    qDebug() << "UDP server started on IP/port:" << s_o43_addr_.c_str() << "/" << o43_port;
}

OpenApiViod::~OpenApiViod()
{
    m_running = false;
    if (m_thread.joinable())
        m_thread.join();
    if (o43_sock_ >= 0) {
        close(o43_sock_);
        o43_sock_ = -1;
    }
}

ssize_t OpenApiViod::SendUdpBE(const size_t size32, const sockaddr_in *addr)
{
    auto temp = send_buf_.get();
    for (size_t i = 0; i < size32; ++i)
        temp[i] = htonl(temp[i]);

    ssize_t bytes_sended = sendto(o43_sock_, (char*)temp, size32 * 4, 0,
                                (sockaddr*)addr, sizeof(sockaddr_in));
    memset(temp, 0, size32 * 4);
    return bytes_sended;
}

ssize_t OpenApiViod::SendUdpBE(uint32_t *data, const size_t size32, const sockaddr_in *addr)
{
    auto temp = send_buf_.get();
    for (size_t i = 0; i < size32; ++i, ++data)
        temp[i] = htonl(*data);

    ssize_t bytes_sended = sendto(o43_sock_, (char*)temp, size32 * 4, 0,
                                (sockaddr*)addr, sizeof(sockaddr_in));
    return bytes_sended;
}

ssize_t OpenApiViod::RecvUdpBE(uint32_t* data, size_t size32)
{
    sockaddr_in client_addr;
    int s_size = sizeof(sockaddr_in);
    ssize_t recv_bytes = recvfrom(o43_sock_, (char*)data, size32 * 4, 0,
                                (sockaddr*)&client_addr, (socklen_t*)&s_size);

    for (ssize_t i = 0; i < recv_bytes / 4; ++i, ++data)
        *data = ntohl(*data);

    return recv_bytes;
}

ssize_t OpenApiViod::RecvUdpBE(size_t size32)
{
    sockaddr_in client_addr;
    auto data = recv_buf_.get();
    int s_size = sizeof(sockaddr_in);
    ssize_t recv_bytes = recvfrom(o43_sock_, (char*)data, size32 * 4, 0,
                                (sockaddr*)&client_addr, (socklen_t*)&s_size);

    for (ssize_t i = 0; i < recv_bytes / 4; ++i, ++data)
        *data = ntohl(*data);

    return recv_bytes;
}

void OpenApiViod::sendViodMsg(std::vector<qword> &data,
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

    msg_header->data_header.cmd = static_cast<uint8_t>(0x06);
    msg_header->viod_header.prmbl = static_cast<uint16_t>(0xAA55);
    msg_header->viod_header.addr_info.smid = smid;
    msg_header->viod_header.addr_info.spid = spid;
    msg_header->viod_header.addr_info.slid = slid;
    msg_header->viod_header.addr_info.dmid = static_cast<uint16_t>(dmid);
    msg_header->viod_header.addr_info.dpid = dpid;
    msg_header->viod_header.addr_info.dlid = dlid;
    msg_header->viod_header.f = 1;
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

        qword *transport_hdr = reinterpret_cast<qword*>(&msg_header->data_header);
        qDebug() << "TRANSPORT_HDR[send]0x" << QString("%1").arg(transport_hdr->word_0, 8, 16, QChar('0')) << " "
                    << "0x" << QString("%1").arg(transport_hdr->word_1, 8, 16, QChar('0')) << " "
                    << "0x" << QString("%1").arg(transport_hdr->word_2, 8, 16, QChar('0')) << " "
                    << "0x" << QString("%1").arg(transport_hdr->word_3, 8, 16, QChar('0'));

        qword *viod_hdr = reinterpret_cast<qword*>(&msg_header->viod_header);
        qDebug() << "VIOD_HDR[send]0x" << QString("%1").arg(viod_hdr->word_0, 8, 16, QChar('0')) << " "
                    << "0x" << QString("%1").arg(viod_hdr->word_1, 8, 16, QChar('0')) << " "
                    << "0x" << QString("%1").arg(viod_hdr->word_2, 8, 16, QChar('0')) << " "
                    << "0x" << QString("%1").arg(viod_hdr->word_3, 8, 16, QChar('0'));

        qDebug() << "DATA[send]";
        for(size_t i{};i<data.size();++i)
        {
            qDebug() << "[send]0x" << QString("%1").arg(data[i].word_0, 8, 16, QChar('0')) << " "
                    << "0x" << QString("%1").arg(data[i].word_1, 8, 16, QChar('0')) << " "
                    << "0x" << QString("%1").arg(data[i].word_2, 8, 16, QChar('0')) << " "
                    << "0x" << QString("%1").arg(data[i].word_3, 8, 16, QChar('0'));
        }

        memcpy(data_ptr, data.data(), qwords_to_send * 16);

        auto raw_size32 = (kviodmingmssize / 4) + (remaining_qwords * 4);
        auto sended = SendUdpBE(raw_size32, &yakaddr_);
        if (sended == 0)
            throw std::runtime_error("Cant send data\n");

        remaining_qwords -= qwords_to_send;
        offset_qwords += qwords_to_send;
    }
}

void OpenApiViod::sendRawData(uint32_t *ptr, size_t size)
{
    auto sended = SendUdpBE(ptr, size, &yakaddr_);
    if (sended == 0)
        throw std::runtime_error("Cant send data\n");
}

void OpenApiViod::rcvLoop()
{
    std::vector<qword> data{};
    while(this->m_running)
    {
        data.clear();
        int length = 0;

        auto header = reinterpret_cast<OPEN_API_VIOD::AbstractHeader*>(recv_buf_.get());
        memset(header, 0, 1472);

        while(header->data_head.cmd != 0x07 && m_running)
        {
            auto size = RecvUdpBE(recv_buf_.get(), 1472 / 4);

            if(size == 0)
            {
                qDebug() << "Receive error or timeout:" << (size < 0 ? strerror(errno) : "no data");
                continue;
            }

            if(header->info_head.cmd == 0x86)
            {
//                wait_msgs -= header->info_head.Delta_num;
//                qDebug() << "header->info_head.CMD == 0x86";
            }

            else if(header->data_head.cmd == 0x07)
            {
                auto full_header = reinterpret_cast<OPEN_API_VIOD::ViodFullFrame*>(recv_buf_.get());
                auto data_in = reinterpret_cast<qword*>(full_header + 1);

                if(full_header->viod_header.addr_info.dmid == 0)
                {
                    length = full_header->viod_header.length;
                    proto_farbos::farbos_header *h = reinterpret_cast<proto_farbos::farbos_header*>(&data_in[0]);
                    switch(static_cast<uint8_t>(h->tk))
                    {
                    case 13:
                    {
                        // -> firstly статусы css
                        far_data_.status_css[0] = data_in[0].word_0;
                        far_data_.status_css[1] = data_in[0].word_1;
                        far_data_.status_css[2] = data_in[0].word_2;
                        far_data_.status_css[3] = data_in[0].word_3;
                        far_data_.status_css[4] = data_in[1].word_0;
                        far_data_.status_css[5] = data_in[1].word_1;
                        far_data_.status_css[6] = data_in[1].word_2;
                        far_data_.status_css[7] = data_in[1].word_3;
                        far_data_.status_css[8] = data_in[2].word_0;
                        far_data_.status_css[9] = data_in[2].word_1;
                        far_data_.status_css[10] = data_in[2].word_2;
                        far_data_.status_css[11] = data_in[2].word_3;
                        far_data_.status_css[12] = data_in[3].word_0;
                        far_data_.status_css[13] = data_in[3].word_1;
                        far_data_.status_css[14] = data_in[3].word_2;
                        far_data_.status_css[15] = data_in[3].word_3;

                        // -> next статусы sum
                        far_data_.status_sum[0][0] = data_in[4].word_0;
                        far_data_.status_sum[0][1] = data_in[4].word_1;
                        far_data_.status_sum[0][2] = data_in[4].word_2;
                        far_data_.status_sum[0][3] = data_in[4].word_3;
                        far_data_.status_sum[0][4] = data_in[5].word_0;
                        far_data_.status_sum[0][5] = data_in[5].word_1;
                        far_data_.status_sum[0][6] = data_in[5].word_2;
                        far_data_.status_sum[0][7] = data_in[5].word_3;
                        far_data_.status_sum[0][8] = data_in[6].word_0;
                        far_data_.status_sum[0][9] = data_in[6].word_1;
                        far_data_.status_sum[0][10] = data_in[6].word_2;
                        far_data_.status_sum[0][11] = data_in[6].word_3;
                        far_data_.status_sum[0][12] = data_in[7].word_0;
                        far_data_.status_sum[0][13] = data_in[7].word_1;
                        far_data_.status_sum[0][14] = data_in[7].word_2;
                        far_data_.status_sum[0][15] = data_in[7].word_3;
                        far_data_.status_sum[0][16] = data_in[8].word_0;
                        far_data_.status_sum[0][17] = data_in[8].word_1;
                        far_data_.status_sum[0][18] = data_in[8].word_2;
                        far_data_.status_sum[0][19] = data_in[8].word_3;
                        far_data_.status_sum[0][20] = data_in[9].word_0;
                        far_data_.status_sum[0][21] = data_in[9].word_1;
                        far_data_.status_sum[0][22] = data_in[9].word_2;
                        far_data_.status_sum[0][23] = data_in[9].word_3;
                        far_data_.status_sum[0][24] = data_in[10].word_0;
                        far_data_.status_sum[0][25] = data_in[10].word_1;
                        far_data_.status_sum[0][26] = data_in[10].word_2;
                        far_data_.status_sum[0][27] = data_in[10].word_3;
                        far_data_.status_sum[0][28] = data_in[11].word_0;
                        far_data_.status_sum[0][29] = data_in[11].word_1;
                        far_data_.status_sum[0][30] = data_in[11].word_2;
                        far_data_.status_sum[0][31] = data_in[11].word_3;

                        far_data_.status_sum[1][0] = data_in[12].word_0;
                        far_data_.status_sum[1][1] = data_in[12].word_1;
                        far_data_.status_sum[1][2] = data_in[12].word_2;
                        far_data_.status_sum[1][3] = data_in[12].word_3;
                        far_data_.status_sum[1][4] = data_in[13].word_0;
                        far_data_.status_sum[1][5] = data_in[13].word_1;
                        far_data_.status_sum[1][6] = data_in[13].word_2;
                        far_data_.status_sum[1][7] = data_in[13].word_3;
                        far_data_.status_sum[1][8] = data_in[14].word_0;
                        far_data_.status_sum[1][9] = data_in[14].word_1;
                        far_data_.status_sum[1][10] = data_in[14].word_2;
                        far_data_.status_sum[1][11] = data_in[14].word_3;
                        far_data_.status_sum[1][12] = data_in[15].word_0;
                        far_data_.status_sum[1][13] = data_in[15].word_1;
                        far_data_.status_sum[1][14] = data_in[15].word_2;
                        far_data_.status_sum[1][15] = data_in[15].word_3;
                        far_data_.status_sum[1][16] = data_in[16].word_0;
                        far_data_.status_sum[1][17] = data_in[16].word_1;
                        far_data_.status_sum[1][18] = data_in[16].word_2;
                        far_data_.status_sum[1][19] = data_in[16].word_3;
                        far_data_.status_sum[1][20] = data_in[17].word_0;
                        far_data_.status_sum[1][21] = data_in[17].word_1;
                        far_data_.status_sum[1][22] = data_in[17].word_2;
                        far_data_.status_sum[1][23] = data_in[17].word_3;
                        far_data_.status_sum[1][24] = data_in[18].word_0;
                        far_data_.status_sum[1][25] = data_in[18].word_1;
                        far_data_.status_sum[1][26] = data_in[18].word_2;
                        far_data_.status_sum[1][27] = data_in[18].word_3;
                        far_data_.status_sum[1][28] = data_in[19].word_0;
                        far_data_.status_sum[1][29] = data_in[19].word_1;
                        far_data_.status_sum[1][30] = data_in[19].word_2;
                        far_data_.status_sum[1][31] = data_in[19].word_3;

                        far_data_.status_sum[2][0] = data_in[20].word_0;
                        far_data_.status_sum[2][1] = data_in[20].word_1;
                        far_data_.status_sum[2][2] = data_in[20].word_2;
                        far_data_.status_sum[2][3] = data_in[20].word_3;
                        far_data_.status_sum[2][4] = data_in[21].word_0;
                        far_data_.status_sum[2][5] = data_in[21].word_1;
                        far_data_.status_sum[2][6] = data_in[21].word_2;
                        far_data_.status_sum[2][7] = data_in[21].word_3;
                        far_data_.status_sum[2][8] = data_in[22].word_0;
                        far_data_.status_sum[2][9] = data_in[22].word_1;
                        far_data_.status_sum[2][10] = data_in[22].word_2;
                        far_data_.status_sum[2][11] = data_in[22].word_3;
                        far_data_.status_sum[2][12] = data_in[23].word_0;
                        far_data_.status_sum[2][13] = data_in[23].word_1;
                        far_data_.status_sum[2][14] = data_in[23].word_2;
                        far_data_.status_sum[2][15] = data_in[23].word_3;
                        far_data_.status_sum[2][16] = data_in[24].word_0;
                        far_data_.status_sum[2][17] = data_in[24].word_1;
                        far_data_.status_sum[2][18] = data_in[24].word_2;
                        far_data_.status_sum[2][19] = data_in[24].word_3;
                        far_data_.status_sum[2][20] = data_in[25].word_0;
                        far_data_.status_sum[2][21] = data_in[25].word_1;
                        far_data_.status_sum[2][22] = data_in[25].word_2;
                        far_data_.status_sum[2][23] = data_in[25].word_3;
                        far_data_.status_sum[2][24] = data_in[26].word_0;
                        far_data_.status_sum[2][25] = data_in[26].word_1;
                        far_data_.status_sum[2][26] = data_in[26].word_2;
                        far_data_.status_sum[2][27] = data_in[26].word_3;
                        far_data_.status_sum[2][28] = data_in[27].word_0;
                        far_data_.status_sum[2][29] = data_in[27].word_1;
                        far_data_.status_sum[2][30] = data_in[27].word_2;
                        far_data_.status_sum[2][31] = data_in[27].word_3;
                        break;
                    }
                    default:
                    {
                        static std::vector<qword> buffer;

                        qDebug() << "FB[rcv DATA]: length ->" << length;

//                        static int cnt = 0;

                        for (int i=0;i<length;++i)
                            buffer.push_back(data_in[i]);

//                        for(size_t i{};i<length;++i)
//                        {
//                            qDebug() << "[rcv]0x" << QString("%1").arg(data_in[i].word_0, 8, 16, QChar('0')) << " "
//                                << "0x" << QString("%1").arg(data_in[i].word_1, 8, 16, QChar('0')) << " "
//                                << "0x" << QString("%1").arg(data_in[i].word_2, 8, 16, QChar('0')) << " "
//                                << "0x" << QString("%1").arg(data_in[i].word_3, 8, 16, QChar('0'));
//                        }

                        if (full_header->viod_header.f == 1 && full_header->viod_header.e != 1)
                            qDebug() << "first!";

                        if (full_header->viod_header.f != 1 && full_header->viod_header.e == 1)
                        {
                            far_data_.ki_kd = buffer.size() * 4;
                            qDebug() << "last! buffer.size=" << buffer.size();
//                            if (cnt > 5) {
                                for (size_t i{};i<buffer.size();++i)
                                {
                                    far_data_.channel1[i*4] = buffer[i].word_0;
                                    far_data_.channel1[i*4+1] = buffer[i].word_1;
                                    far_data_.channel1[i*4+2] = buffer[i].word_2;
                                    far_data_.channel1[i*4+3] = buffer[i].word_3;
                                }
//                            }
//                            ++cnt;
                            buffer.clear();
                        }
                        break;
                    }
                    }
                }
            }

            else
            {
                qDebug() << "Unexpected packet type, size bytes = " << size;
            }
        }
    }
}
