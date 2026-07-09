// loop test: Request-Response (Querier + Responser)
#include <QCoreApplication>
#include <QThread>
#include <QTimer>
#include <QDebug>
#include "../knotlink/opensocketresponser.h"
#include "../knotlink/opensocketquerier.h"

static bool shutdown = false;
constexpr int MAX_ITERATIONS = 5;

void run_responser() {
    OpenSocketResponser res("app.knotlinksdk.test", "test_socket");
    QObject::connect(&res, &OpenSocketResponser::receivedData,
        [](const QString &data, QString) {
            qDebug() << "[Responser] Received:" << data;
        });
    qDebug() << "[Responser] Running...";
    while (!shutdown) QThread::msleep(1000);
}

void run_querier() {
    QThread::msleep(1000);
    OpenSocketQuerier q("app.knotlinksdk.test", "test_socket");
    for (int i = 1; i <= MAX_ITERATIONS && !shutdown; i++) {
        QString data = QString("Hello #%1").arg(i);
        QString result = q.query_l(data);
        qDebug() << "[Querier] Req:" << data << "-> Res:" << result;
        QThread::sleep(3);
    }
    shutdown = true;
}

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);
    QThread* t = QThread::create(run_responser);
    t->start();
    QTimer::singleShot(100, run_querier);
    QTimer::singleShot(30000, &app, &QCoreApplication::quit);
    app.exec();
    t->wait();
    return 0;
}
