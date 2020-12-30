#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "pcancontrol.h"
#include <QKeyEvent>
#include <mTcpServer.h>
#include <mtcpsocket.h>
#include <QDateTime>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QVariantMap>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    //set can controller
    pcc = pcanControl();
    pcc.SetMode(pcc.Mode3);
    setADSpeed(0.2);

    //set TCP/IP socket server
    TcpServer = new mTcpServer();
    if(TcpServer->listen(QHostAddress::Any, TcpServer->listenPort)){
        qDebug() << "listen ok";
        connect(TcpServer, &mTcpServer::sendCmd, this, &MainWindow::GetCmd);
    }
    else{
        qDebug() << "listen failed";
    }

    time = QDateTime::currentMSecsSinceEpoch();

    //ui init
    this->setWindowTitle("机动控制");
    ui->lineEdit_port->setText("9999");
    ui->lineEdit_diameter->setText(QString::number(diameter));
    ui->lineEdit_aspeed->setText(QString::number(adspeed));
    ui->lineEdit_maxspeed->setText(QString::number(maxSpeed));

    ui->lineEdit_left_speed->setText(QString::number(lspeed));
    ui->lineEdit_right_speed->setText(QString::number(rspeed));
    ui->lineEdit_state->setText(QString::number(state));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setSpeed(double leftSpeed, double rightSpeed)
{
    //判断是否超出最大速度
    if(leftSpeed >= maxSpeed){
        leftSpeed = maxSpeed;
    }
    if(rightSpeed >= maxSpeed){
        rightSpeed = maxSpeed;
    }
    /*
     * 转化speed到rpm <cm/s & rpm>
     * 1 rpm = pi*d/60 cm/s
     * pi = 3.1415...
     * d = 直径(cm)
    */
    double leftRpm, rightRpm;
    leftRpm = leftSpeed * (60.0/M_PI)/diameter;
    rightRpm = rightSpeed * (60.0/M_PI)/diameter;
    pcc.SetSpeed(leftRpm,rightRpm);
    pcc.Start();
    rspeed = leftSpeed;
    lspeed = rightSpeed;

    ui->lineEdit_left_speed->setText(QString::number(lspeed));
    ui->lineEdit_right_speed->setText(QString::number(rspeed));
    ui->lineEdit_state->setText(QString::number(state));
}

void MainWindow::setADSpeed(double speed)
{
    /*
     * 转化speed/s到rpm/s
     * 1 rpm = pi*d/60 cm/s
     * pi = 3.1415...
     * d = 直径(cm)
    */
    double Rpm = speed * (60.0/M_PI)/diameter;
    pcc.SetAcceleratedSpeed(Rpm,Rpm);
    pcc.SetDeceleratedSpeed(-Rpm,-Rpm);
    adspeed = Rpm;

    ui->lineEdit_aspeed->setText(QString::number(adspeed));
}

void MainWindow::setStop()
{
    /*
     * set speed to zero
     * and set stop mode
    */
    pcc.SetSpeed(0,0);
    pcc.Stop();
    rspeed = 0;
    lspeed = 0;
}

void MainWindow::GetCmd(const int handle, CMD cmd)
{
    //get cmd from tcp server
    //test
    qDebug() << "Receive CMD:";
    qDebug() << cmd.codeId;
    qDebug() << cmd.left_speed;
    qDebug() << cmd.right_speed;
    qDebug() << cmd.time_ms;

    //判断是否为控制指令
    if(cmd.codeId < 0){
        //非控制指令
        if(cmd.codeId == -1){
            //获取当前状态参数,构造json字符串，然后发送
            QVariantMap map;
            map.insert("state",state);
            map.insert("left",lspeed);
            map.insert("right",rspeed);
            map.insert("diameter",diameter);
            map.insert("maxSpeed",maxSpeed);
            map.insert("adSpeed",adspeed);
            QJsonDocument json(QJsonObject::fromVariantMap(map));
            TcpServer->setData(json.toJson(),handle);
            return;
        }
        else if(cmd.codeId == -2){
            //设置参数
            if(cmd.left_speed > 0){
                setADSpeed(cmd.left_speed);
            }
            if(cmd.right_speed > 0){
                maxSpeed = cmd.right_speed;
            }
            if(cmd.time_ms > 0){
                diameter = cmd.time_ms;
            }
            ui->lineEdit_left_speed->setText(QString::number(lspeed));
            ui->lineEdit_right_speed->setText(QString::number(rspeed));
            ui->lineEdit_state->setText(QString::number(state));
            return;
        }
        else{
            //预留接口
            return;
        }
    }

    //判断当前状态有效性<是否到了恢复0状态的时间？>
    qint64 mtime = QDateTime::currentMSecsSinceEpoch();
    if(mtime > time){
        if(state < 3){
            state = 0;
            time = mtime;
        }
        else if(state > 3){
            //退出强制控制模式的指令
            state = 0;
            time = mtime;
            ui->lineEdit_state->setText(QString::number(state));
            return;
        }
    }

    //判断当前cmd能否执行<低于当前状态，不予执行>
    if(cmd.codeId < state){
        return;
    }

    //执行当前指令
    setSpeed(cmd.left_speed, cmd.right_speed);

    //更新状态
    if(cmd.codeId > state){
        state = cmd.codeId;
    }
    time += cmd.time_ms;

    ui->lineEdit_left_speed->setText(QString::number(lspeed));
    ui->lineEdit_right_speed->setText(QString::number(rspeed));
    ui->lineEdit_state->setText(QString::number(state));
}

void MainWindow::keyPressEvent(QKeyEvent *e)
{
    /*
     * listen : keyboard press event
    */
    if(e->key() == Qt::Key_W){
        setSpeed(fs,ts);
    }
    else if(e->key() == Qt::Key_S){
        setSpeed(-fs,-fs);
    }
    else if(e->key() == Qt::Key_A){
        setSpeed(-ts,ts);
    }
    else if(e->key() == Qt::Key_D){
        setSpeed(ts,-ts);
    }
    else if(e->key() == Qt::Key_Escape){
        setStop();
    }
    else{
        setStop();
    }

    ui->lineEdit_left_speed->setText(QString::number(lspeed));
    ui->lineEdit_right_speed->setText(QString::number(rspeed));
    ui->lineEdit_state->setText(QString::number(state));
}

