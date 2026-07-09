// unit test: SignalSubscriber (gui)
#include <QCoreApplication>
#include <QDebug>
#include "../../knotlink/signalsubscriber.h"

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);
    SignalSubscriber sub("app.knotlinksdk.test", "test_signal");
    QObject::connect(&sub, &SignalSubscriber::receivedData,
        [](const QString &data) {
            qDebug() << "Received:" << data;
        });
    qDebug() << "Subscriber listening...";
    app.exec();
    return 0;
}
