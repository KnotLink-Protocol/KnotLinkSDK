// loop test: Signal-Subscribe (Sender + Subscriber)
#include <QCoreApplication>
#include <QThread>
#include <QTimer>
#include <QDebug>
#include "../knotlink/signalsubscriber.h"
#include "../knotlink/signalsender.h"

static bool shutdown = false;
constexpr int MAX_ITERATIONS = 5;

void run_subscriber() {
    SignalSubscriber sub("app.knotlinksdk.test", "test_signal");
    QObject::connect(&sub, &SignalSubscriber::receivedData,
        [](const QString &data) {
            qDebug() << "[Subscriber] Received:" << data;
        });
    qDebug() << "[Subscriber] Listening...";
    while (!shutdown) QThread::msleep(1000);
}

void run_sender() {
    QThread::msleep(1000);
    SignalSender sender;
    sender.setConfig("app.knotlinksdk.test", "test_signal");
    for (int i = 1; i <= MAX_ITERATIONS && !shutdown; i++) {
        QString data = QString("Signal #%1").arg(i);
        sender.emitt(data);
        qDebug() << "[Sender] Sent:" << data;
        QThread::sleep(3);
    }
    shutdown = true;
}

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);
    QThread* t = QThread::create(run_subscriber);
    t->start();
    QTimer::singleShot(100, run_sender);
    QTimer::singleShot(30000, &app, &QCoreApplication::quit);
    app.exec();
    t->wait();
    return 0;
}
