// unit test: OpenSocketResponser (gui)
#include <QCoreApplication>
#include <QDebug>
#include "../../knotlink/opensocketresponser.h"

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);
    OpenSocketResponser res("app.knotlinksdk.test", "test_socket");
    QObject::connect(&res, &OpenSocketResponser::receivedData,
        [](const QString &data, QString) {
            qDebug() << "Received:" << data;
        });
    qDebug() << "Responser running...";
    app.exec();
    return 0;
}
