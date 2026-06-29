#include <QCoreApplication>
#include <executor.h>
#include <QTextCodec>

int main(int argc, char *argv[])
{
    qRegisterMetaType<u16>("u16");
    qRegisterMetaType<std::vector<int>>("std::vector<int>");
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("CP866"));

    QCoreApplication a(argc, argv);

    Server::instance().startSVthread();
    Executor::instance().startExthread();

    return a.exec();
}
