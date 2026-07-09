/*
 * KnotLink SDK - Qt C++ Integration Test
 * Signal-Subscribe pattern: Subscriber + Sender
 */

#include <QCoreApplication>
#include <QThread>
#include <QTimer>
#include <QDebug>
#include "../knotlink/signalsubscriber.h"
#include "../knotlink/signalsender.h"

static bool shutdown = false;

void run_subscriber() {
    SignalSubscriber sub("app.knotlinksdk.test", "test_signal");
    QObject::connect(&sub, &SignalSubscriber::receivedData,
        [](const QString &data) {
            qDebug() << "[Subscriber] Received:" << data;
        });
    qDebug() << "[Subscriber] Listening...";
    while (!shutdown) {
        QThread::msleep(1000);
    }
}

void run_sender() {
    QThread::msleep(1000);
    SignalSender sender;
    sender.setConfig("app.knotlinksdk.test", "test_signal");
    sender.emitt("Hello from Sender!");
    qDebug() << "[Sender] Signal sent.";
    QThread::msleep(2000);
    shutdown = true;
}

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);

    QThread* t = QThread::create(run_subscriber);
    t->start();

    QTimer::singleShot(100, run_sender);
    QTimer::singleShot(5000, &app, &QCoreApplication::quit);

    app.exec();
    t->wait();
    return 0;
}
