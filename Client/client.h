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
private:
    QByteArray recvBuffer_;
    TransferProtocol::Packet readPacketFast(QByteArray &DATA);
    QByteArray decompressData(const QByteArray& compressed);
};
#endif // CLIENT_H
