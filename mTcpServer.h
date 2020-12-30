#ifndef SOCKETSERVER_H
#define SOCKETSERVER_H

#include <QObject>
#include <QWidget>
#include <QTcpServer>
#include <QTcpSocket>
#include <QMap>
#include "mtcpsocket.h"
#include "headstruct.h"

class mTcpServer : public QTcpServer
{
    Q_OBJECT
public:
    explicit mTcpServer(QObject *parent = nullptr);
    quint16 listenPort = 9999;

protected:
    void incomingConnection(qintptr handle);

signals:
    //发送 新用户连接信息
    void connectClient(const int , const QString & ,const quint16 );
    //发送 获得用户发过来的数据
    void readData(const int,const QString &, quint16, const QByteArray &);
    //发送 断开连接的用户信息
    void sockDisConnect(const int ,const QString &,const quint16 );
    //向某一socket发送消息
    void sentData(const QByteArray &,const int);
    //发送 发出收到的CMD信息
    void sendCmd(const int, CMD);

public slots:
    //向用户发送消息
    void setData(const QByteArray & data, const int  handle);
    //发送获得用户发过来的数据
    void readDataSlot(const int, const QString &, const quint16,const QByteArray &);
    //断开连接的用户信息
    void sockDisConnectSlot(int handle, QString ip, quint16 prot);

private:
    QMap<int, mTcpSocket *> * clientPool;

};

#endif // SOCKETSERVER_H
