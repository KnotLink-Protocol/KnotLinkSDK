// unit test: OpenSocketQuerier (gui)
#include <QCoreApplication>
#include <QDebug>
#include "../../knotlink/opensocketquerier.h"

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);
    OpenSocketQuerier q("app.knotlinksdk.test", "test_socket");
    QString result = q.query_l("Hello, Responser!");
    qDebug() << "Response:" << result;
    return 0;
}
