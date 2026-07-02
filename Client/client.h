#ifndef CLIENT_H
#define CLIENT_H

#include <QMainWindow>
#include "qcustomplot.h"
#include <QTcpSocket>
#include <QUdpSocket>
#include <QDebug>
#include "groupData.h"
#include "zstd.h"
#include <memory>
#include <QTimer>

QT_BEGIN_NAMESPACE
namespace Ui
{
class Client;
}
QT_END_NAMESPACE

namespace UDP_DATA
{
    const u8 STATUS_CSS = 0x0;
    const u8 STATUS_SUM = 0x1;
    const u8 CHANNEL_DATA1 = 0x2;
    const u8 CHANNEL_DATA2 = 0x3;
    const u8 CHANNEL_DATA3 = 0x4;
    const u8 CHANNEL_DATA4 = 0x5;
    const u8 CHANNEL_DATA5 = 0x6;
}

class Client : public QMainWindow
{
    Q_OBJECT

public:
    Client(QWidget *parent = nullptr);
    ~Client();
private slots:
    void onConnect();
    void onLoadCss();
    void onStart();
    void onStop();
    void onSendKu();
private slots:
    void onConnected();
    void onDisconnected();
    void onReadyRead();
    void onReadyReadUdp();
private:
    Ui::Client *ui;
    std::unique_ptr<QCustomPlot> plot_;
    std::unique_ptr<QTcpSocket> socket_;
    std::unique_ptr<QUdpSocket> udp_socket_;
private:
    std::vector<QLabel*> wdgts_status_css_;
    std::vector<QLabel*> wdgts_status_sum1_;
    std::vector<QLabel*> wdgts_status_sum2_;
    std::vector<QLabel*> wdgts_status_sum3_;
private:
    QVector<int> status_css_{};
    QVector<int> status_sum_{};
    QVector<double> x1, x2, x3, x4, x5;
    QVector<double> sin1, cos1, amp1, ph1;
    QVector<double> sin2, cos2, amp2, ph2;
    QVector<double> sin3, cos3, amp3, ph3;
    QVector<double> sin4, cos4, amp4, ph4;
    QVector<double> sin5, cos5, amp5, ph5;
private:
    QByteArray recvBuffer_;
    TransferProtocol::Packet readPacketFast(QByteArray &DATA);
    QByteArray decompressData(const QByteArray& compressed);
};
#endif // CLIENT_H
