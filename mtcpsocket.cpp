#include "mtcpsocket.h"
#include <QThread>

mTcpSocket::mTcpSocket(qintptr socketDescriptor,QObject *parent)
{
    this->setSocketDescriptor(socketDescriptor);
    socketID = socketDescriptor;
    //转换系统信号到我们需要发送的数据、、直接用lambda表达式了，qdebug是输出线程信息
    //连接到收到数据的处理函数
    connect(this,&mTcpSocket::readyRead,this,&mTcpSocket::thisReadData);
    //转换收到的信息，发送信号
    connect(this,&mTcpSocket::readyRead,
                [this](){
                    qDebug() <<"mTcpSocket::mTcpSocket lambda readData thread is:" << QThread::currentThreadId();
                    //发送用户发过来的数据
                    emit mreadData(socketID,this->peerAddress().toString(),this->peerPort() ,this->readAll());
                });
    //断开连接的信号转换
    connect(this,&mTcpSocket::disconnected,
                [this](){
                    qDebug() <<"mTcpSocket::mTcpSocket lambda sockDisConnect thread is:" << QThread::currentThreadId();
                    //发送断开连接的用户信息
                    emit sockDisConnect(socketID,this->peerAddress().toString(),this->peerPort());
                });
    qDebug() << this->socketDescriptor() << " " << this->peerAddress().toString()
             << " " << this->peerPort() << "mTcpSocket::mTcpSocket thread is " <<QThread::currentThreadId();
}

void mTcpSocket::thisReadData()
{
    //
}

void mTcpSocket::sentData(const QByteArray &data, const int id)
{
    //如果是服务器判断好，直接调用write会出现跨线程的现象，所以服务器用广播，每个连接判断是否是自己要发送的信息
    //判断是否是此socket的信息
    if(id == socketID)
    {
        qDebug() << "mTcpSocket::sentData" << QThread::currentThreadId();
        write(data);
    }
}
