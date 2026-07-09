// unit test: SignalSender (gui)
#include <QCoreApplication>
#include <QDebug>
#include "../../knotlink/signalsender.h"

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);
    SignalSender sender;
    sender.setConfig("app.knotlinksdk.test", "test_signal");
    sender.emitt("Hello from Sender!");
    qDebug() << "Signal sent.";
    return 0;
}
