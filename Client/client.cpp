#include "client.h"
#include "./ui_client.h"

Client::Client(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::Client)
    , plot_{std::make_unique<QCustomPlot>(this)}
    , socket_{std::make_unique<QTcpSocket>(this)}
    , udp_socket_{std::make_unique<QUdpSocket>(this)}
{
    ui->setupUi(this);

    plot_->setNotAntialiasedElements(QCP::aeAll);
    plot_->legend->setVisible(false);

    plot_->addGraph();
    plot_->graph(0)->setPen(QPen(Qt::red, 1));

    plot_->addGraph();
    plot_->graph(1)->setPen(QPen(Qt::blue, 1));

    plot_->addGraph();
    plot_->graph(2)->setPen(QPen(Qt::green, 1));

    plot_->addGraph();
    plot_->graph(3)->setPen(QPen(Qt::magenta, 1));

    plot_->addGraph();
    plot_->graph(4)->setPen(QPen(Qt::cyan, 1));

    for (size_t i{};i<5;++i) {
        plot_->graph(i)->setScatterStyle(QCPScatterStyle::ssNone);  // Без точек
        plot_->graph(i)->setLineStyle(QCPGraph::lsLine);  // Только линия
    }

    ui->graphicLayout->addWidget(plot_.get());

    wdgts_status_css_ = {ui->wdgt_status_css1,
                        ui->wdgt_status_css2,
                        ui->wdgt_status_css3,
                        ui->wdgt_status_css4,
                        ui->wdgt_status_css5,
                        ui->wdgt_status_css6,
                        ui->wdgt_status_css7,
                        ui->wdgt_status_css8,
                        ui->wdgt_status_css9,
                        ui->wdgt_status_css10,
                        ui->wdgt_status_css11,
                        ui->wdgt_status_css12,
                        ui->wdgt_status_css13,
                        ui->wdgt_status_css14,
                        ui->wdgt_status_css15,
                        ui->wdgt_status_css16};

    wdgts_status_sum1_ = {ui->wdgt_status_sum11,
                          ui->wdgt_status_sum12,
                          ui->wdgt_status_sum13,
                          ui->wdgt_status_sum14,
                          ui->wdgt_status_sum15,
                          ui->wdgt_status_sum16,
                          ui->wdgt_status_sum17,
                          ui->wdgt_status_sum18,
                          ui->wdgt_status_sum19,
                          ui->wdgt_status_sum110,
                          ui->wdgt_status_sum111,
                          ui->wdgt_status_sum112,
                          ui->wdgt_status_sum113,
                          ui->wdgt_status_sum114,
                          ui->wdgt_status_sum115,
                          ui->wdgt_status_sum116,
                          ui->wdgt_status_sum117,
                          ui->wdgt_status_sum118,
                          ui->wdgt_status_sum119,
                          ui->wdgt_status_sum120,
                          ui->wdgt_status_sum121,
                          ui->wdgt_status_sum122,
                          ui->wdgt_status_sum123,
                          ui->wdgt_status_sum124,
                          ui->wdgt_status_sum125,
                          ui->wdgt_status_sum126,
                          ui->wdgt_status_sum127,
                          ui->wdgt_status_sum128,
                          ui->wdgt_status_sum129,
                          ui->wdgt_status_sum130,
                          ui->wdgt_status_sum131,
                          ui->wdgt_status_sum132};

    wdgts_status_sum2_ = {ui->wdgt_status_sum21,
                          ui->wdgt_status_sum22,
                          ui->wdgt_status_sum23,
                          ui->wdgt_status_sum24,
                          ui->wdgt_status_sum25,
                          ui->wdgt_status_sum26,
                          ui->wdgt_status_sum27,
                          ui->wdgt_status_sum28,
                          ui->wdgt_status_sum29,
                          ui->wdgt_status_sum210,
                          ui->wdgt_status_sum211,
                          ui->wdgt_status_sum212,
                          ui->wdgt_status_sum213,
                          ui->wdgt_status_sum214,
                          ui->wdgt_status_sum215,
                          ui->wdgt_status_sum216,
                          ui->wdgt_status_sum217,
                          ui->wdgt_status_sum218,
                          ui->wdgt_status_sum219,
                          ui->wdgt_status_sum220,
                          ui->wdgt_status_sum221,
                          ui->wdgt_status_sum222,
                          ui->wdgt_status_sum223,
                          ui->wdgt_status_sum224,
                          ui->wdgt_status_sum225,
                          ui->wdgt_status_sum226,
                          ui->wdgt_status_sum227,
                          ui->wdgt_status_sum228,
                          ui->wdgt_status_sum229,
                          ui->wdgt_status_sum230,
                          ui->wdgt_status_sum231,
                          ui->wdgt_status_sum232};

    wdgts_status_sum3_ = {ui->wdgt_status_sum31,
                          ui->wdgt_status_sum32,
                          ui->wdgt_status_sum33,
                          ui->wdgt_status_sum34,
                          ui->wdgt_status_sum35,
                          ui->wdgt_status_sum36,
                          ui->wdgt_status_sum37,
                          ui->wdgt_status_sum38,
                          ui->wdgt_status_sum39,
                          ui->wdgt_status_sum310,
                          ui->wdgt_status_sum311,
                          ui->wdgt_status_sum312,
                          ui->wdgt_status_sum313,
                          ui->wdgt_status_sum314,
                          ui->wdgt_status_sum315,
                          ui->wdgt_status_sum316,
                          ui->wdgt_status_sum317,
                          ui->wdgt_status_sum318,
                          ui->wdgt_status_sum319,
                          ui->wdgt_status_sum320,
                          ui->wdgt_status_sum321,
                          ui->wdgt_status_sum322,
                          ui->wdgt_status_sum323,
                          ui->wdgt_status_sum324,
                          ui->wdgt_status_sum325,
                          ui->wdgt_status_sum326,
                          ui->wdgt_status_sum327,
                          ui->wdgt_status_sum328,
                          ui->wdgt_status_sum329,
                          ui->wdgt_status_sum330,
                          ui->wdgt_status_sum331,
                          ui->wdgt_status_sum332};

    QObject::connect(ui->wdgt_connect, &QPushButton::clicked, this, & Client::onConnect);
    QObject::connect(ui->wdgt_load_css, &QPushButton::clicked, this, & Client::onLoadCss);
    QObject::connect(ui->wdgt_start, &QPushButton::clicked, this, & Client::onStart);
    QObject::connect(ui->wdgt_stop, &QPushButton::clicked, this, & Client::onStop);
    QObject::connect(ui->wdgt_send_ku, &QPushButton::clicked, this, & Client::onSendKu);

    QObject::connect(socket_.get(), &QTcpSocket::connected, this, &Client::onConnected);
    QObject::connect(socket_.get(), &QTcpSocket::disconnected, this, &Client::onDisconnected);
    QObject::connect(socket_.get(), &QTcpSocket::readyRead, this, &Client::onReadyRead);
}

Client::~Client()
{
    delete ui;
}

void Client::onConnect() {
    u16 port = ui->wdgt_server_port->text().toUInt();
    socket_->connectToHost(QHostAddress(ui->wdgt_server_addr->text()), port);
    QTimer::singleShot(2000, [this]() {
        if (socket_->state() == QAbstractSocket::ConnectingState) {
            socket_->abort();
            qDebug() << "Connection timeout";
        }
    });
}

void Client::onConnected() {
    qDebug() << "Подключено к серверу!";
    ui->wdgt_connect->setEnabled(false);
}

void Client::onDisconnected() {
    qDebug() << "Отключено от сервера!";
    udp_socket_->close();
    udp_socket_.reset();
    ui->wdgt_connect->setEnabled(true);
}

void Client::onReadyRead() {
    QDataStream stream(socket_.get());
    stream.setVersion(QDataStream::Qt_5_5);

    quint32 msgSize;
    stream >> msgSize;                          // читаем размер
    QByteArray payload = socket_->read(msgSize); // читаем тело

    // Теперь у нас есть полное сообщение
    GROUP_DATA::client_data data = GROUP_DATA::client_data::deserialize(payload);
    qDebug() << "Getting data...: IP =" << QString::fromStdString(data.ip)
             << "port =" << data.port;

    udp_socket_ = std::make_unique<QUdpSocket>(this);
    QObject::connect(udp_socket_.get(), &QUdpSocket::readyRead, this, &Client::onReadyReadUdp);
    if (udp_socket_->bind(QHostAddress(QString::fromStdString(data.ip)), data.port))
        qDebug() << "udp`s opened...";
    else
        throw std::runtime_error("error .. bad bind udp..");
}

void Client::onReadyReadUdp() {
    static QHostAddress sAddr;
    static quint16 sPort = 0;

    while (udp_socket_ && udp_socket_->hasPendingDatagrams())
    {
        const qint64 pendingSize = udp_socket_->pendingDatagramSize();
        if (pendingSize <= 0)
        {
            udp_socket_->readDatagram(nullptr, 0, &sAddr, &sPort);
            continue;
        }

        // !!!резервируем если необходимо и затем resize для чтения!!!
        if (this->recvBuffer_.capacity() < pendingSize)
            this->recvBuffer_.reserve(static_cast<int>(pendingSize));
        this->recvBuffer_.resize(static_cast<int>(pendingSize));

        qint64 got = udp_socket_->readDatagram(this->recvBuffer_.data(), this->recvBuffer_.size(), &sAddr, &sPort);
        if (got <= 0)
            continue;

        const qint64 minHeaderSize = static_cast<qint64>(sizeof(TransferProtocol::PacketHeader) + sizeof(int8_t));
        if (got < minHeaderSize)
            continue;

        TransferProtocol::Packet packet = readPacketFast(this->recvBuffer_);

        switch (packet.m_int_dataType_)
        {
        case UDP_DATA::STATUS_CSS: {
            static QByteArray assembledStatusCssData;
            this->status_css_.clear();
            assembledStatusCssData.append(packet.m_data_);
            QByteArray finalData;
            if (packet.m_header_.m_flags_.m_boolean_isCompressed)
                finalData = decompressData(assembledStatusCssData);
            else
                finalData = assembledStatusCssData;
            const int* intArray = reinterpret_cast<const int*>(finalData.constData());
            const quint32 dataSizeInInts = finalData.size() / sizeof(int);
            this->status_css_.resize(static_cast<int>(dataSizeInInts));
            memcpy(this->status_css_.data(), intArray, static_cast<size_t>(dataSizeInInts) << 2);

            for (u32 i{};i<16;++i)
                wdgts_status_css_[i]->setText("0x" + QString().asprintf("%.8x", status_css_[i]));

            assembledStatusCssData.clear();
            break;
        }
        case UDP_DATA::STATUS_SUM: {
            static QByteArray assembledStatusSumData;
            this->status_sum_.clear();
            assembledStatusSumData.append(packet.m_data_);
            QByteArray finalData;
            if (packet.m_header_.m_flags_.m_boolean_isCompressed)
                finalData = decompressData(assembledStatusSumData);
            else
                finalData = assembledStatusSumData;
            const int* intArray = reinterpret_cast<const int*>(finalData.constData());
            const quint32 dataSizeInInts = finalData.size() / sizeof(int);
            this->status_sum_.resize(static_cast<int>(dataSizeInInts));
            memcpy(this->status_sum_.data(), intArray, static_cast<size_t>(dataSizeInInts) << 2);

            for (u32 i{};i<32;++i)
                wdgts_status_sum1_[i]->setText("0x" + QString().asprintf("%.8x", status_sum_[i]));

            for (u32 i{};i<32;++i)
                wdgts_status_sum2_[i]->setText("0x" + QString().asprintf("%.8x", status_sum_[i+32]));

            for (u32 i{};i<32;++i)
                wdgts_status_sum3_[i]->setText("0x" + QString().asprintf("%.8x", status_sum_[i+32+32]));

            assembledStatusSumData.clear();
            break;
        }
        case UDP_DATA::CHANNEL_DATA1: {
            static QByteArray assembledImpulseData;

            QVector<int> listSignalData;
            listSignalData.reserve(packet.m_header_.m_int_totalDataSize_/sizeof(int));

            const bool isFirst = packet.m_header_.m_flags_.m_boolean_isFirst_;
            const bool isLast = packet.m_header_.m_flags_.m_boolean_isLast_;
            if ((!isFirst && isLast) || (isFirst && isLast))
            {
                assembledImpulseData.append(packet.m_data_);
                QByteArray finalData;
                if (packet.m_header_.m_flags_.m_boolean_isCompressed)
                    finalData = decompressData(assembledImpulseData);
                else
                    finalData = assembledImpulseData;

                const int* intArray = reinterpret_cast<const int*>(finalData.constData());
                const quint32 dataSizeInInts = finalData.size() / sizeof(int);
                listSignalData.resize(static_cast<int>(dataSizeInInts));
                memcpy(listSignalData.data(), intArray, finalData.size());

                x1.reserve(listSignalData.size());
                sin1.reserve(listSignalData.size());
                cos1.reserve(listSignalData.size());
                amp1.reserve(listSignalData.size());

                for(int i{};i<listSignalData.size();++i) {
                    sin1.push_back(static_cast<short>(listSignalData[i] & 0xFFFF));
                    cos1.push_back(static_cast<short>((listSignalData[i] >> 16) & 0xFFFF));
                    amp1.push_back(static_cast<float>(sqrt(10.0)*sqrt(sin1[i] * sin1[i]/10.0 + cos1[i] * cos1[i]/10.0)));
                    x1.push_back(i);
                }

                if (ui->wdgt_CHAN1->isChecked()) {
                    plot_->graph(0)->setVisible(true);
                    plot_->graph(0)->setData(x1, amp1);
                } else {
                    plot_->graph(0)->setVisible(false);
                }

                plot_->xAxis->rescale();
                plot_->yAxis->rescale();

                plot_->replot(QCustomPlot::rpQueuedReplot);

                x1.clear();
                sin1.clear();
                cos1.clear();
                amp1.clear();

                assembledImpulseData.clear();
            }
            if ((!isFirst && !isLast) || (isFirst && !isLast))
            {
                assembledImpulseData.append(packet.m_data_);
            }

            break;
        }
        case UDP_DATA::CHANNEL_DATA2: {
            static QByteArray assembledImpulseData;

            QVector<int> listSignalData;
            listSignalData.reserve(packet.m_header_.m_int_totalDataSize_/sizeof(int));

            const bool isFirst = packet.m_header_.m_flags_.m_boolean_isFirst_;
            const bool isLast = packet.m_header_.m_flags_.m_boolean_isLast_;
            if ((!isFirst && isLast) || (isFirst && isLast))
            {
                assembledImpulseData.append(packet.m_data_);
                QByteArray finalData;
                if (packet.m_header_.m_flags_.m_boolean_isCompressed)
                    finalData = decompressData(assembledImpulseData);
                else
                    finalData = assembledImpulseData;

                const int* intArray = reinterpret_cast<const int*>(finalData.constData());
                const quint32 dataSizeInInts = finalData.size() / sizeof(int);
                listSignalData.resize(static_cast<int>(dataSizeInInts));
                memcpy(listSignalData.data(), intArray, finalData.size());

                x2.reserve(listSignalData.size());
                sin2.reserve(listSignalData.size());
                cos2.reserve(listSignalData.size());
                amp2.reserve(listSignalData.size());

                for(int i{};i<listSignalData.size();++i) {
                    sin2.push_back(static_cast<short>(listSignalData[i] & 0xFFFF));
                    cos2.push_back(static_cast<short>((listSignalData[i] >> 16) & 0xFFFF));
                    amp2.push_back(static_cast<float>(sqrt(10.0)*sqrt(sin2[i] * sin2[i]/10.0 + cos2[i] * cos2[i]/10.0)));
                    x2.push_back(i);
                }

                if (ui->wdgt_CHAN2->isChecked()) {
                    plot_->graph(1)->setVisible(true);
                    plot_->graph(1)->setData(x2, amp2);
                } else {
                    plot_->graph(1)->setVisible(false);
                }

                plot_->xAxis->rescale();
                plot_->yAxis->rescale();

                plot_->replot(QCustomPlot::rpQueuedReplot);

                x2.clear();
                sin2.clear();
                cos2.clear();
                amp2.clear();

                assembledImpulseData.clear();
            }
            if ((!isFirst && !isLast) || (isFirst && !isLast))
            {
                assembledImpulseData.append(packet.m_data_);
            }

            break;
        }
        case UDP_DATA::CHANNEL_DATA3: {
            static QByteArray assembledImpulseData;

            QVector<int> listSignalData;
            listSignalData.reserve(packet.m_header_.m_int_totalDataSize_/sizeof(int));

            const bool isFirst = packet.m_header_.m_flags_.m_boolean_isFirst_;
            const bool isLast = packet.m_header_.m_flags_.m_boolean_isLast_;
            if ((!isFirst && isLast) || (isFirst && isLast))
            {
                assembledImpulseData.append(packet.m_data_);
                QByteArray finalData;
                if (packet.m_header_.m_flags_.m_boolean_isCompressed)
                    finalData = decompressData(assembledImpulseData);
                else
                    finalData = assembledImpulseData;

                const int* intArray = reinterpret_cast<const int*>(finalData.constData());
                const quint32 dataSizeInInts = finalData.size() / sizeof(int);
                listSignalData.resize(static_cast<int>(dataSizeInInts));
                memcpy(listSignalData.data(), intArray, finalData.size());

                x3.reserve(listSignalData.size());
                sin3.reserve(listSignalData.size());
                cos3.reserve(listSignalData.size());
                amp3.reserve(listSignalData.size());

                for(int i{};i<listSignalData.size();++i) {
                    sin3.push_back(static_cast<short>(listSignalData[i] & 0xFFFF));
                    cos3.push_back(static_cast<short>((listSignalData[i] >> 16) & 0xFFFF));
                    amp3.push_back(static_cast<float>(sqrt(10.0)*sqrt(sin3[i] * sin3[i]/10.0 + cos3[i] * cos3[i]/10.0)));
                    x3.push_back(i);
                }

                if (ui->wdgt_CHAN3->isChecked()) {
                    plot_->graph(2)->setVisible(true);
                    plot_->graph(2)->setData(x3, amp3);
                } else {
                    plot_->graph(2)->setVisible(false);
                }

                plot_->xAxis->rescale();
                plot_->yAxis->rescale();

                plot_->replot(QCustomPlot::rpQueuedReplot);

                x3.clear();
                sin3.clear();
                cos3.clear();
                amp3.clear();

                assembledImpulseData.clear();
            }
            if ((!isFirst && !isLast) || (isFirst && !isLast))
            {
                assembledImpulseData.append(packet.m_data_);
            }

            break;
        }
        case UDP_DATA::CHANNEL_DATA4: {
            static QByteArray assembledImpulseData;

            QVector<int> listSignalData;
            listSignalData.reserve(packet.m_header_.m_int_totalDataSize_/sizeof(int));

            const bool isFirst = packet.m_header_.m_flags_.m_boolean_isFirst_;
            const bool isLast = packet.m_header_.m_flags_.m_boolean_isLast_;
            if ((!isFirst && isLast) || (isFirst && isLast))
            {
                assembledImpulseData.append(packet.m_data_);
                QByteArray finalData;
                if (packet.m_header_.m_flags_.m_boolean_isCompressed)
                    finalData = decompressData(assembledImpulseData);
                else
                    finalData = assembledImpulseData;

                const int* intArray = reinterpret_cast<const int*>(finalData.constData());
                const quint32 dataSizeInInts = finalData.size() / sizeof(int);
                listSignalData.resize(static_cast<int>(dataSizeInInts));
                memcpy(listSignalData.data(), intArray, finalData.size());

                x4.reserve(listSignalData.size());
                sin4.reserve(listSignalData.size());
                cos4.reserve(listSignalData.size());
                amp4.reserve(listSignalData.size());

                for(int i{};i<listSignalData.size();++i) {
                    sin4.push_back(static_cast<short>(listSignalData[i] & 0xFFFF));
                    cos4.push_back(static_cast<short>((listSignalData[i] >> 16) & 0xFFFF));
                    amp4.push_back(static_cast<float>(sqrt(10.0)*sqrt(sin4[i] * sin4[i]/10.0 + cos4[i] * cos4[i]/10.0)));
                    x4.push_back(i);
                }

                if (ui->wdgt_CHAN4->isChecked()) {
                    plot_->graph(3)->setVisible(true);
                    plot_->graph(3)->setData(x4, amp4);
                } else {
                    plot_->graph(3)->setVisible(false);
                }

                plot_->xAxis->rescale();
                plot_->yAxis->rescale();

                plot_->replot(QCustomPlot::rpQueuedReplot);

                x4.clear();
                sin4.clear();
                cos4.clear();
                amp4.clear();

                assembledImpulseData.clear();
            }
            if ((!isFirst && !isLast) || (isFirst && !isLast))
            {
                assembledImpulseData.append(packet.m_data_);
            }

            break;
        }
        case UDP_DATA::CHANNEL_DATA5: {
            static QByteArray assembledImpulseData;

            QVector<int> listSignalData;
            listSignalData.reserve(packet.m_header_.m_int_totalDataSize_/sizeof(int));

            const bool isFirst = packet.m_header_.m_flags_.m_boolean_isFirst_;
            const bool isLast = packet.m_header_.m_flags_.m_boolean_isLast_;
            if ((!isFirst && isLast) || (isFirst && isLast))
            {
                assembledImpulseData.append(packet.m_data_);
                QByteArray finalData;
                if (packet.m_header_.m_flags_.m_boolean_isCompressed)
                    finalData = decompressData(assembledImpulseData);
                else
                    finalData = assembledImpulseData;

                const int* intArray = reinterpret_cast<const int*>(finalData.constData());
                const quint32 dataSizeInInts = finalData.size() / sizeof(int);
                listSignalData.resize(static_cast<int>(dataSizeInInts));
                memcpy(listSignalData.data(), intArray, finalData.size());

                x5.reserve(listSignalData.size());
                sin5.reserve(listSignalData.size());
                cos5.reserve(listSignalData.size());
                amp5.reserve(listSignalData.size());

                for(int i{};i<listSignalData.size();++i) {
                    sin5.push_back(static_cast<short>(listSignalData[i] & 0xFFFF));
                    cos5.push_back(static_cast<short>((listSignalData[i] >> 16) & 0xFFFF));
                    amp5.push_back(static_cast<float>(sqrt(10.0)*sqrt(sin5[i] * sin5[i]/10.0 + cos5[i] * cos5[i]/10.0)));
                    x5.push_back(i);
                }

                if (ui->wdgt_CHAN5->isChecked()) {
                    plot_->graph(4)->setVisible(true);
                    plot_->graph(4)->setData(x5, amp5);
                } else {
                    plot_->graph(4)->setVisible(false);
                }

                plot_->xAxis->rescale();
                plot_->yAxis->rescale();

                plot_->replot(QCustomPlot::rpQueuedReplot);

                x5.clear();
                sin5.clear();
                cos5.clear();
                amp5.clear();

                assembledImpulseData.clear();
            }
            if ((!isFirst && !isLast) || (isFirst && !isLast))
            {
                assembledImpulseData.append(packet.m_data_);
            }

            break;
        }
        }
    }
}

TransferProtocol::Packet Client::readPacketFast(QByteArray &DATA)
{
    TransferProtocol::Packet packet;
    const char* raw = DATA.constData();

    // !!!Вытаскиваем заголовок и dataType!!!
    memcpy(&packet.m_header_, raw, sizeof(TransferProtocol::PacketHeader));
    memcpy(&packet.m_int_dataType_, raw + sizeof(TransferProtocol::PacketHeader), sizeof(int8_t));

    const int dataOffset = static_cast<int>(sizeof(TransferProtocol::PacketHeader) + sizeof(int8_t));
    int dataSize = 0;
    int currentPacketSize = static_cast<int>(packet.m_header_.m_int_currentPacketSize_);
    (currentPacketSize > dataOffset) ? dataSize = currentPacketSize - dataOffset : dataSize = 0;
    if (dataSize > 0)
        packet.m_data_ = QByteArray::fromRawData(raw + dataOffset, dataSize);
    else packet.m_data_.clear();

    return packet;
}


QByteArray Client::decompressData(const QByteArray& compressed)
{
    // Проверяем, сжаты ли данные (Zstd magic number: 0xFD2FB528)
    if (compressed.size() < 4) return compressed;

    const unsigned char* data = reinterpret_cast<const unsigned char*>(compressed.constData());
    if (data[0] == 0x28 && data[1] == 0xB5 && data[2] == 0x2F && data[3] == 0xFD)
    {
        // Это Zstd сжатые данные
        size_t decompressedSize = ZSTD_getFrameContentSize(
            compressed.constData(),
            compressed.size()
            );

        if (decompressedSize != ZSTD_CONTENTSIZE_ERROR && decompressedSize > 0) {
            QByteArray decompressed(static_cast<int>(decompressedSize), 0);
            size_t actualSize = ZSTD_decompress(
                decompressed.data(), decompressedSize,
                compressed.constData(), compressed.size()
                );

            if (!ZSTD_isError(actualSize) && actualSize > 0) {
                decompressed.resize(static_cast<int>(actualSize));
                return decompressed;
            }
        }
    }

    // Если не сжато или ошибка распаковки - возвращаем как есть
    return compressed;
}


void Client::onLoadCss() {
    if (!socket_)
        return;

    GROUP_DATA::command_data cmd;
    cmd.cmd = "loadcss";
    cmd.ki = ui->wdgt_KI->value();
    cmd.ns = ui->wdgt_NS->value();

    QByteArray payload = GROUP_DATA::command_data::serilize(cmd);
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_5);
    out << quint32(payload.size());
    block.append(payload);

    socket_->write(block);
    socket_->flush();
}

void Client::onStart() {
    if (!socket_)
        return;

    GROUP_DATA::command_data cmd;
    cmd.cmd = "start";
    cmd.ki = ui->wdgt_KI->value();
    cmd.ns = ui->wdgt_NS->value();

    QByteArray payload = GROUP_DATA::command_data::serilize(cmd);
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_5);
    out << quint32(payload.size());
    block.append(payload);

    socket_->write(block);
    socket_->flush();
}

void Client::onStop() {
    if (!socket_)
        return;

    GROUP_DATA::command_data cmd;
    cmd.cmd = "stop";
    cmd.ki = ui->wdgt_KI->value();
    cmd.ns = ui->wdgt_NS->value();

    QByteArray payload = GROUP_DATA::command_data::serilize(cmd);
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_5);
    out << quint32(payload.size());
    block.append(payload);

    socket_->write(block);
    socket_->flush();
}

void Client::onSendKu() {
    if (!socket_)
        return;

    GROUP_DATA::command_data cmd;
    cmd.cmd = "loadNewKu";
    cmd.ki = ui->wdgt_KI->value();
    cmd.ns = ui->wdgt_NS->value();

    QByteArray payload = GROUP_DATA::command_data::serilize(cmd);
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_5);
    out << quint32(payload.size());
    block.append(payload);

    socket_->write(block);
    socket_->flush();
}
