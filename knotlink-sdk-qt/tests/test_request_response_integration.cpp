/*
 * KnotLink SDK - Qt C++ Integration Test
 * Request-Response pattern: Responser + Querier
 */

#include <QCoreApplication>
#include <QThread>
#include <QTimer>
#include <QDebug>
#include "../knotlink/opensocketresponser.h"
#include "../knotlink/opensocketquerier.h"

static bool shutdown = false;

void run_responser() {
    OpenSocketResponser res("app.knotlinksdk.test", "test_socket");
    QObject::connect(&res, &OpenSocketResponser::receivedData,
        [](const QString &data, QString) {
            qDebug() << "[Responser] Received:" << data;
        });
    qDebug() << "[Responser] Running...";
    while (!shutdown) {
        QThread::msleep(1000);
    }
}

void run_querier() {
    QThread::msleep(1000);
    OpenSocketQuerier q("app.knotlinksdk.test", "test_socket");
    QString result = q.query_l("Hello, Responser!");
    qDebug() << "[Querier] Response:" << result;
    shutdown = true;
}

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);

    QThread* t = QThread::create(run_responser);
    t->start();

    QTimer::singleShot(100, run_querier);
    QTimer::singleShot(5000, &app, &QCoreApplication::quit);

    app.exec();
    t->wait();
    return 0;
}
