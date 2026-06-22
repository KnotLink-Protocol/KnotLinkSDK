#include "opensocketresponser.h"

OpenSocketResponser::OpenSocketResponser(QString APPID, QString OpenSocketID, QObject *parent)
    : QObject(parent), appID(APPID), openSocketID(OpenSocketID)
{
    init();
}


void OpenSocketResponser::init()
{
    KLresponser = new TcpClient(this);
    KLresponser->connectToServer("127.0.0.1",6378);
    qDebug()<<"OKK";
    connect(KLresponser, &TcpClient::receivedData, this, &OpenSocketResponser::dataRecv);
    QString s_key = appID + "-" + openSocketID;
    // 将 s_key 转换为 QByteArray
    QByteArray s_key_bytes = s_key.toUtf8(); // 转换为 UTF-8 编码的 QByteArray
    KLresponser->sendData(s_key_bytes);
}

void OpenSocketResponser::dataRecv(const QByteArray &data)
{
    QString s_data = QString::fromUtf8(data);

    QString delimiter = "&*&"; // 分隔符
    QStringList parts = s_data.split(delimiter); // 按分隔符分割字符串

    if (parts.size() != 2) // 确保分割后有两部分
    {
        qDebug() << "Invalid data format. Expected two parts separated by" << delimiter;
        return;
    }

    QString key = parts[0]; // 前一部分作为 key
    QString t_data = parts[1]; // 后一部分作为 t_data
    qDebug() << "Key:" << key;
    qDebug() << "t_data:" << t_data;

    emit receivedData_a(t_data.toUtf8(),key);
    emit receivedData(t_data,key);
}

void OpenSocketResponser::sendBack(const QString &data,QString questionID)
{
    sendBack(data.toUtf8(),questionID);
}

void OpenSocketResponser::sendBack(const QByteArray &data,QString questionID)
{
    QString data_r = questionID + "&*&" + QString::fromUtf8(data);
    KLresponser->sendData(data_r.toUtf8());
}
