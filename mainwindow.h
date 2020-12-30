#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "pcancontrol.h"
#include "mTcpServer.h"
#include "headstruct.h"
#include <QDateTime>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    pcanControl pcc;

    //speed of the robot
    double fs = 10;
    double ts = 20;

    //diameter(cm) of the wheel
    double diameter = 15;

    //state of the controller
    /*
     * 0 normal<self control>
     * 1 second direct control<hands\voice\etc>
     * 2 direct control<cmd from remote control terminals>
     * 3 direct control<total control>
    */
    int state = 0;
    //time used to change the state
    qint64 time;

    //the speed of each wheels
    double rspeed;
    double lspeed;

    //the speed of ap and dp  <cm/s*s>加速度，设定两个轮子的加速减速加速度都是一样的
    double adspeed;

    //max speed 20cm/s
    double maxSpeed = 20;

    //tcpServer
    mTcpServer *TcpServer;

    //public functions
    void setSpeed(double leftSpeed, double rightSpeed);
    void setADSpeed(double speed);
    void setStop();

public slots:
    //get cmd from tcp server
    void GetCmd(const int handle, CMD cmd);

private:
    Ui::MainWindow *ui;
    void keyPressEvent(QKeyEvent *e);

};
#endif // MAINWINDOW_H
