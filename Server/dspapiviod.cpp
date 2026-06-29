#include "dspapiviod.h"

//DspApiViod::DspApiViod(QObject *parent) : ViodSender{parent}
//{
//    this->m_dspApi = dspapi_init(0, "127.0.0.1", true);
//    if (!this->m_dspApi)
//        throw std::runtime_error("dspapi - bad");

//    this->m_thread = std::thread(&DspApiViod::rcvLoop, this);
//    this->m_thread.detach();

//    qDebug() << Q_FUNC_INFO;
//}

//DspApiViod::~DspApiViod()
//{
//    this->m_running = false;
//    if (m_thread.joinable())
//        m_thread.join();
//    dspapi_close(m_dspApi);
//}

//void DspApiViod::sendViodMsg(std::vector<QWord> &data,
//                             const u16 smid,
//                             const u16 spid,
//                             const u16 slid,
//                             const u16 dmid,
//                             const u16 dpid,
//                             const u16 dlid,
//                             const u16 chid)
//{
//    VIOD_PACKET viodPacket{};
//    for (size_t i=0;i<data.size();++i)
//    {
//        viodPacket.data[0 + 4*i] = data[i].word_0;
//        viodPacket.data[1 + 4*i] = data[i].word_1;
//        viodPacket.data[2 + 4*i] = data[i].word_2;
//        viodPacket.data[3 + 4*i] = data[i].word_3;
//    }

//    viodPacket.header.prmbl = static_cast<uint16_t>(0xAA55);
//    viodPacket.header.smid = 0;
//    viodPacket.header.spid = 0;
//    viodPacket.header.slid = 0;
//    viodPacket.header.dmid = static_cast<uint16_t>(0x8);
//    viodPacket.header.dpid = 0;
//    viodPacket.header.dlid = 0;
//    viodPacket.header.f = 1;
//    viodPacket.header.e = 1;
//    viodPacket.header.chid = chid;
//    viodPacket.header.ip_port = 10001;

//    uint16_t remaining_qwords = static_cast<uint16_t>(data.size());
//    uint16_t offset_qwords = 0;

//    while(remaining_qwords > 0)
//    {
//        int16_t qwords_to_send = (remaining_qwords < Constants::VIOD_MAX_PAYLOAD_WORDS)
//                                               ? remaining_qwords
//                                               : Constants::VIOD_MAX_PAYLOAD_WORDS;

//        viodPacket.header.full_length = qwords_to_send;
//        viodPacket.header.length = qwords_to_send;

//        qDebug() << "DATA_LENGTH[send] =" << viodPacket.header.length;
//        for(size_t i=0;i<viodPacket.header.length;++i)
//        {
//            QWord *dataR = reinterpret_cast<QWord*>(viodPacket.data);
//            qDebug() << "0x" << QString("%1").arg(dataR[i].word_0, 8, 16, QChar('0')) << " "
//                << "0x" << QString("%1").arg(dataR[i].word_1, 8, 16, QChar('0')) << " "
//                << "0x" << QString("%1").arg(dataR[i].word_2, 8, 16, QChar('0')) << " "
//                << "0x" << QString("%1").arg(dataR[i].word_3, 8, 16, QChar('0'));
//        }

//        // !!!from Kyznecov!!!
//        QThread::usleep(10000);

//        dspapi_send(this->m_dspApi, viodPacket);

//        remaining_qwords -= qwords_to_send;
//        offset_qwords += qwords_to_send;
//    }
//}

//void DspApiViod::rcvLoop()
//{
//    qDebug() << "RCV_LOOP -> ACTIVATED";
//    while(this->m_running)
//    {
//        VIOD_PACKET rcv{};
//        dspapi_recv(this->m_dspApi, &rcv);
//        qDebug() << "DATA_LENGTH[rcv] =" << rcv.header.length;
//        for(size_t i=0;i<rcv.header.length;++i)
//        {
//            QWord *dataR = reinterpret_cast<QWord*>(rcv.data);
//            qDebug() << "0x" << QString("%1").arg(dataR[i].word_0, 8, 16, QChar('0')) << " "
//                << "0x" << QString("%1").arg(dataR[i].word_1, 8, 16, QChar('0')) << " "
//                << "0x" << QString("%1").arg(dataR[i].word_2, 8, 16, QChar('0')) << " "
//                << "0x" << QString("%1").arg(dataR[i].word_3, 8, 16, QChar('0'));
//        }
//    }
//}
