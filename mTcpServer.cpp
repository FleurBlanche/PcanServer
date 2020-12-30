#include "mtcpserver.h"
#include <QObject>
#include <QWidget>
#include <QTcpServer>
#include <QTcpSocket>
#include <QThread>
#include "mtcpsocket.h"
#include "mainwindow.h"

/*
 * global var of mainwindow
*/
extern MainWindow * mw;

mTcpServer::mTcpServer(QObject *parent) : QTcpServer(parent)
{
    clientPool = new QMap<int, mTcpSocket *>();
}

void mTcpServer::incomingConnection(qintptr handle)
{
    //获取到新的连接
    mTcpSocket * tcpTemp = new mTcpSocket(handle);
    qDebug() << handle << " : create tcp socket";
    //把线程的父类设为连接的，防止内存泄漏
    QThread * thread = new QThread(tcpTemp);
    qDebug() << handle << " : create thread";

    //可以信号连接信号的，我要捕捉线程ID就独立出来函数了，使用中还是直接连接信号效率应该有优势
    //接受到数据
    connect(tcpTemp,&mTcpSocket::mreadData,this,&mTcpServer::readDataSlot);
    //断开连接的处理，从列表移除，并释放断开的Tcpsocket
    connect(tcpTemp,&mTcpSocket::sockDisConnect,this,&mTcpServer::sockDisConnectSlot);
    //发送数据
    connect(this,&mTcpServer::sentData,tcpTemp,&mTcpSocket::sentData);
    //断开连接时线程退出
    connect(tcpTemp,&mTcpSocket::disconnected,thread,&QThread::quit);
    qDebug() << handle << " : connect signals";

    //把tcp类移动到新的线程
    tcpTemp->moveToThread(thread);
    //线程开始运行
    thread->start();
    qDebug() << handle << " : start thread";

    //插入到连接信息中
    clientPool->insert(handle, tcpTemp);
    qDebug() <<"incomingConnection THREAD IS：" <<QThread::currentThreadId();
    //发送连接信号
    emit connectClient(tcpTemp->socketDescriptor(),tcpTemp->peerAddress().toString(),tcpTemp->peerPort());
}

void mTcpServer::setData(const QByteArray &data, const int handle)
{
    emit sentData(data,handle);
}

void mTcpServer::readDataSlot(const int handle, const QString & ip, const quint16 port, const QByteArray & data)
{
    //from handle (ip,port) read <data>
    qDebug() <<"mTcpServer: read data from " << handle << " " << ip << ":" << QString::number(port) << " :\n" << data;
    //read data and emit signal
    CMD *tmp = (CMD *)data.data();
    CMD cmd;
    cmd.codeId = tmp->codeId;
    cmd.left_speed = tmp->left_speed;
    cmd.right_speed = tmp->right_speed;
    cmd.time_ms = tmp->time_ms;

    //中文命令测试
//    指令 例子;
//    例子.优先级 = 1;

    emit sendCmd(handle, cmd);
}

void mTcpServer::sockDisConnectSlot(int handle, QString ip, quint16 prot)
{
    qDebug() <<"mTcpServer::sockDisConnectSlot thread is:" << QThread::currentThreadId();
    mTcpSocket * tcp = clientPool->value(handle);
    //连接管理中移除断开连接的socket
    clientPool->remove(handle);
    //释放断开连接的资源
    delete tcp;
    emit sockDisConnect(handle,ip,prot);
}
