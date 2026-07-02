#include "openapiviod.h"
#include "executor.h"

OpenApiViod::OpenApiViod() : send_buf_{std::make_unique<unsigned[]>(1472)}, recv_buf_{std::make_unique<unsigned[]>(1472)}
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
            if(size == 0) {
                qDebug() << "[OpenApiViod]Receive error or timeout:" << (size < 0 ? strerror(errno) : "no data");
                continue;
            }
            if(header->info_head.cmd == 0x86) {
//                wait_msgs -= header->info_head.Delta_num;
//                qDebug() << "header->info_head.CMD == 0x86";
            }
            else if(header->data_head.cmd == 0x07) {
                auto full_header = reinterpret_cast<OPEN_API_VIOD::ViodFullFrame*>(recv_buf_.get());
                auto data_in = reinterpret_cast<qword*>(full_header + 1);

                if (full_header->viod_header.addr_info.dmid == 0) {
                    length = full_header->viod_header.length;
                    switch (full_header->viod_header.chid) {
                    case 0: {
                        static std::vector<qword> buffer;
                        for (u32 i{};i<length;++i) {
                            buffer.push_back(data_in[i]);
                        }
                        if (full_header->viod_header.f != 1 && full_header->viod_header.e == 1) {
                            far_data_.ki_kd = full_header->viod_header.full_length * 4;
                            qDebug() << "[OpenApiViod]FullSizeData1=" << buffer.size();
                            for (u32 i{};i<buffer.size();++i) {
                                far_data_.channel1[i*4] = buffer[i].word_0;
                                far_data_.channel1[i*4+1] = buffer[i].word_1;
                                far_data_.channel1[i*4+2] = buffer[i].word_2;
                                far_data_.channel1[i*4+3] = buffer[i].word_3;
                            }
                            buffer.clear();
                        }
                        Server::instance().sendData(UDP_DATA::CHANNEL_DATA1);
                        break;
                    }
                    case 1: {
                        static std::vector<qword> buffer;
                        for (u32 i{};i<length;++i) {
                            buffer.push_back(data_in[i]);
                        }
                        if (full_header->viod_header.f != 1 && full_header->viod_header.e == 1) {
                            far_data_.ki_kd = full_header->viod_header.full_length * 4;
                            qDebug() << "[OpenApiViod]FullSizeData2=" << buffer.size();
                            for (u32 i{};i<buffer.size();++i) {
                                far_data_.channel2[i*4] = buffer[i].word_0;
                                far_data_.channel2[i*4+1] = buffer[i].word_1;
                                far_data_.channel2[i*4+2] = buffer[i].word_2;
                                far_data_.channel2[i*4+3] = buffer[i].word_3;
                            }
                            buffer.clear();
                        }
                        Server::instance().sendData(UDP_DATA::CHANNEL_DATA2);
                        break;
                    }
                    case 2: {
                        static std::vector<qword> buffer;
                        for (u32 i{};i<length;++i) {
                            buffer.push_back(data_in[i]);
                        }
                        if (full_header->viod_header.f != 1 && full_header->viod_header.e == 1) {
                            far_data_.ki_kd = full_header->viod_header.full_length * 4;
                            qDebug() << "[OpenApiViod]FullSizeData3=" << buffer.size();
                            for (u32 i{};i<buffer.size();++i) {
                                far_data_.channel3[i*4] = buffer[i].word_0;
                                far_data_.channel3[i*4+1] = buffer[i].word_1;
                                far_data_.channel3[i*4+2] = buffer[i].word_2;
                                far_data_.channel3[i*4+3] = buffer[i].word_3;
                            }
                            buffer.clear();
                        }
                        Server::instance().sendData(UDP_DATA::CHANNEL_DATA3);
                        break;
                    }
                    case 3: {
                        static std::vector<qword> buffer;
                        for (u32 i{};i<length;++i) {
                            buffer.push_back(data_in[i]);
                        }
                        if (full_header->viod_header.f != 1 && full_header->viod_header.e == 1) {
                            far_data_.ki_kd = full_header->viod_header.full_length * 4;
                            qDebug() << "[OpenApiViod]FullSizeData4=" << buffer.size();
                            for (u32 i{};i<buffer.size();++i) {
                                far_data_.channel4[i*4] = buffer[i].word_0;
                                far_data_.channel4[i*4+1] = buffer[i].word_1;
                                far_data_.channel4[i*4+2] = buffer[i].word_2;
                                far_data_.channel4[i*4+3] = buffer[i].word_3;
                            }
                            buffer.clear();
                        }
                        Server::instance().sendData(UDP_DATA::CHANNEL_DATA4);
                        break;
                    }
                    case 4: {
                        static std::vector<qword> buffer;
                        for (u32 i{};i<length;++i) {
                            buffer.push_back(data_in[i]);
                        }
                        if (full_header->viod_header.f != 1 && full_header->viod_header.e == 1) {
                            far_data_.ki_kd = full_header->viod_header.full_length * 4;
                            qDebug() << "[OpenApiViod]FullSizeData5=" << buffer.size();
                            for (u32 i{};i<buffer.size();++i) {
                                far_data_.channel5[i*4] = buffer[i].word_0;
                                far_data_.channel5[i*4+1] = buffer[i].word_1;
                                far_data_.channel5[i*4+2] = buffer[i].word_2;
                                far_data_.channel5[i*4+3] = buffer[i].word_3;
                            }
                            buffer.clear();
                        }
                        Server::instance().sendData(UDP_DATA::CHANNEL_DATA5);
                        break;
                    }
                    case 5: {
                        for (size_t i{}; i < 4; i++) {
                            far_data_.status_css[i * 4 + 0] = data_in[i].word_0;
                            far_data_.status_css[i * 4 + 1] = data_in[i].word_1;
                            far_data_.status_css[i * 4 + 2] = data_in[i].word_2;
                            far_data_.status_css[i * 4 + 3] = data_in[i].word_3;
                        }
                        Server::instance().sendData(UDP_DATA::STATUS_CSS);
                        for (size_t i{}; i < 3; i++) {
                            for (size_t j{}; j < 8; j++) {
                                far_data_.status_sum[i][j * 4 + 0] = data_in[4 + i * 8 + j].word_0;
                                far_data_.status_sum[i][j * 4 + 1] = data_in[4 + i * 8 + j].word_1;
                                far_data_.status_sum[i][j * 4 + 2] = data_in[4 + i * 8 + j].word_2;
                                far_data_.status_sum[i][j * 4 + 3] = data_in[4 + i * 8 + j].word_3;
                            }
                        }
                        Server::instance().sendData(UDP_DATA::STATUS_SUM);
                        break;
                    }
                    default: qDebug() << "[OpenApiViod]???"; break;
                    }
                }
            }
            else {
                qDebug() << "[OpenApiViod]Unexpected packet type, size bytes = " << size;
            }
        }
    }
}
