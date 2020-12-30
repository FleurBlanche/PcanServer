#ifndef MTCPSOCKET_H
#define MTCPSOCKET_H

#include <QObject>
#include <QWidget>
#include <QTcpServer>
#include <QTcpSocket>
#include <QVector>

class mTcpSocket : public QTcpSocket
{
    Q_OBJECT
public:
    explicit mTcpSocket(qintptr socketDescriptor,QObject *parent = 0);

signals:
    //发送获得用户发过来的数据
    void mreadData(const int,const QString & ip,const quint16 port,const QByteArray & data);
    //断开连接的用户信息
    void sockDisConnect(const int ,const QString & ip,const quint16 port);

public slots:
    //处理接收到的数据
    void thisReadData();
    //发送信号的槽
    void sentData(const QByteArray & data,const int handle);

private:
    //保存id，== this->socketDescriptor()；但是this->socketDescriptor()客户端断开会被释放
    //断开信号用this->socketDescriptor()得不到正确值
    qintptr socketID;
};

#endif // MTCPSOCKET_H
