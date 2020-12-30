#include "pcancontrol.h"
#include <QMutex>
#include <QDebug>
#include <windef.h>

pcanControl::pcanControl()
{

    TPCANStatus result;

    char strMsg[256];
    result = CAN_Initialize(PCAN_USBBUS1, PCAN_BAUD_500K);
    if (result != PCAN_ERROR_OK)
    {
        CAN_GetErrorText(result, 0, strMsg);
        qDebug() << strMsg;
    }
}

TPCANMsg pcanControl::Command(uint ID, QByteArray cmd)
{
    qDebug() << "Command Send:" << cmd.toHex();

    TPCANStatus resultWrite,resultRead;
    char strMsg[256];
    TPCANTimestamp timestamp;
    TPCANMsg msg;
    msg.ID = ID;
    msg.MSGTYPE = PCAN_MESSAGE_STANDARD;
    msg.LEN = 8;
    for (int i = 0; i < 8; i++)
    {
        msg.DATA[i] = cmd[i];
    }

    resultWrite = CAN_Write(PCAN_USBBUS1, &msg);
    if (resultWrite != PCAN_ERROR_OK)
    {
        CAN_GetErrorText(resultWrite, 0, strMsg);
    }
    resultRead = CAN_Read(PCAN_USBBUS1, &msg, &timestamp);
    if (resultRead != PCAN_ERROR_OK)
    {
        CAN_GetErrorText(resultRead, 0, strMsg);
    }

    if (resultWrite != PCAN_ERROR_OK ||
          resultRead != PCAN_ERROR_OK)
        msg.ID = mERROR;

    return msg;
}

bool pcanControl::SetMode(int mode)
{
    TPCANMsg msgLeft,msgRight;
    Mode = mode;
    if (mode == Mode1 || mode == Mode2)
    {
        msgLeft = Command(leftWheelID, QByteArrayLiteral("\x2F\x60\x60\x00\x01\x00\x00\x00"));
        msgRight = Command(rightWheelID, QByteArrayLiteral("\x2F\x60\x60\x00\x01\x00\x00\x00"));

        if (msgLeft.ID != mERROR && msgRight.ID != mERROR) return true;
    }
    else if (mode == Mode3)
    {
        msgLeft = Command(leftWheelID, QByteArrayLiteral("\x2F\x60\x60\x00\x03\x00\x00\x00"));
        msgRight = Command(rightWheelID, QByteArrayLiteral("\x2F\x60\x60\x00\x03\x00\x00\x00"));

        if (msgLeft.ID != mERROR && msgRight.ID != mERROR) return true;
    }
    return false;
}

bool pcanControl::Start()
{
    TPCANMsg msgLeft, msgRight;
    if (Mode == Mode1)
    {
        msgLeft = Command(leftWheelID, QByteArrayLiteral("\x2B\x40\x60\x00\x2F\x00\x00\x00"));
        msgRight = Command(rightWheelID, QByteArrayLiteral("\x2B\x40\x60\x00\x2F\x00\x00\x00"));

        if (msgLeft.ID == mERROR || msgRight.ID == mERROR) return false;

        msgLeft = Command(leftWheelID, QByteArrayLiteral("\x2B\x40\x60\x00\x3F\x00\x00\x00"));
        msgRight = Command(rightWheelID, QByteArrayLiteral("\x2B\x40\x60\x00\x3F\x00\x00\x00"));

        if (msgLeft.ID != mERROR && msgRight.ID != mERROR) return true;
    }
    else if (Mode == Mode2)
    {
        msgLeft = Command(leftWheelID, QByteArrayLiteral("\x2B\x40\x60\x00\x4F\x00\x00\x00"));
        msgRight = Command(rightWheelID, QByteArrayLiteral("\x2B\x40\x60\x00\x4F\x00\x00\x00"));

        if (msgLeft.ID == mERROR || msgRight.ID == mERROR) return false;

        msgLeft = Command(leftWheelID, QByteArrayLiteral("\x2B\x40\x60\x00\x5F\x00\x00\x00"));
        msgRight = Command(rightWheelID, QByteArrayLiteral("\x2B\x40\x60\x00\x5F\x00\x00\x00"));

        if (msgLeft.ID != mERROR && msgRight.ID != mERROR) return true;
    }
    else if (Mode == Mode3)
    {
        msgLeft = Command(leftWheelID, QByteArrayLiteral("\x2B\x40\x60\x00\x0F\x00\x00\x00"));
        msgRight = Command(rightWheelID, QByteArrayLiteral("\x2B\x40\x60\x00\x0F\x00\x00\x00"));

        if (msgLeft.ID != mERROR && msgRight.ID != mERROR) return true;
    }
    return false;
}

bool pcanControl::Stop()
{
    TPCANMsg msgLeft, msgRight;

    msgLeft = Command(leftWheelID, QByteArrayLiteral("\x2B\x40\x60\x00\x06\x00\x00\x00"));
    msgRight = Command(rightWheelID, QByteArrayLiteral("\x2B\x40\x60\x00\x06\x00\x00\x00"));

    if (msgLeft.ID != mERROR && msgRight.ID != mERROR) return true;
    return false;
}

QByteArray int2Byte(int index){
    QByteArray ret;
    //转hex
    QByteArray tmp = QByteArray::number(index,16).toUpper();
    //0补全
    while(tmp.size() < 8){
        tmp = tmp.prepend(QByteArray::number(0));
    }
    QString stmp = QString(tmp);
    ret.append(stmp.mid(6,2).toInt(nullptr, 16));
    ret.append(stmp.mid(4,2).toInt(nullptr, 16));
    ret.append(stmp.mid(2,2).toInt(nullptr, 16));
    ret.append(stmp.mid(0,2).toInt(nullptr, 16));
    return ret;
}

bool pcanControl::SetSpeed(double leftRpm, double rightRpm)
{
    TPCANMsg msgLeft, msgRight;

    //转化RPM为DEC速度<32位有符号数>
    int leftDec = -(int)(leftRpm * 512 * 10000 / 1875 * SlowDown);
    int rightDec = (int)(rightRpm * 512 * 10000 / 1875 * SlowDown);

    QByteArray leftBytes = int2Byte(leftDec);
    QByteArray rightBytes = int2Byte(rightDec);

    //速度数据发送时，先发送低位，再发送高位
    msgLeft = Command(leftWheelID, QByteArrayLiteral("\x23\xFF\x60\x00").append(leftBytes[0]).append(leftBytes[1]).append(leftBytes[2]).append(leftBytes[3]));
    msgRight = Command(rightWheelID, QByteArrayLiteral("\x23\xFF\x60\x00").append(rightBytes[0]).append(rightBytes[1]).append(rightBytes[2]).append(rightBytes[3]));

    if (msgLeft.ID != mERROR && msgRight.ID != mERROR) return true;

    return false;
}

bool pcanControl::SetMaxSpeed(double leftRpm, double rightRpm)
{
    TPCANMsg msgLeft, msgRight;

    int leftDec = (int)(leftRpm * 512 * 10000 / 1875 * SlowDown);
    int rightDec = (int)(rightRpm * 512 * 10000 / 1875 * SlowDown);

    QByteArray leftBytes = int2Byte(leftDec);
    QByteArray rightBytes = int2Byte(rightDec);

    //速度数据发送时，先发送低位，再发送高位
    msgLeft = Command(leftWheelID, QByteArrayLiteral("\x23\x81\x60\x00").append(leftBytes[0]).append(leftBytes[1]).append(leftBytes[2]).append(leftBytes[3]));
    msgRight = Command(rightWheelID, QByteArrayLiteral("\x23\x81\x60\x00").append(rightBytes[0]).append(rightBytes[1]).append(rightBytes[2]).append(rightBytes[3]));

    if (msgLeft.ID != mERROR && msgRight.ID != mERROR) return true;

    return false;
}

bool pcanControl::SetAcceleratedSpeed(double leftRps, double rightRps)
{
    TPCANMsg msgLeft, msgRight;

    int leftDec = (int)(leftRps * 65536 * 10000 / 4000000 * SlowDown);
    int rightDec = (int)(rightRps * 65536 * 10000 / 4000000 * SlowDown);

    QByteArray leftBytes = int2Byte(leftDec);
    QByteArray rightBytes = int2Byte(rightDec);

    //速度数据发送时，先发送低位，再发送高位
    msgLeft = Command(leftWheelID, QByteArrayLiteral("\x20\x83\x60\x00").append(leftBytes[0]).append(leftBytes[1]).append(leftBytes[2]).append(leftBytes[3]));
    msgRight = Command(rightWheelID, QByteArrayLiteral("\x20\x83\x60\x00").append(rightBytes[0]).append(rightBytes[1]).append(rightBytes[2]).append(rightBytes[3]));

    if (msgLeft.ID != mERROR && msgRight.ID != mERROR) return true;

    return false;
}

bool pcanControl::SetDeceleratedSpeed(double leftRps, double rightRps)
{
    TPCANMsg msgLeft, msgRight;

    int leftDec = (int)(leftRps * 65536 * 10000 / 4000000 * SlowDown);
    int rightDec = (int)(rightRps * 65536 * 10000 / 4000000 * SlowDown);

    QByteArray leftBytes = int2Byte(leftDec);
    QByteArray rightBytes = int2Byte(rightDec);

    //速度数据发送时，先发送低位，再发送高位
    msgLeft = Command(leftWheelID, QByteArrayLiteral("\x20\x84\x60\x00").append(leftBytes[0]).append(leftBytes[1]).append(leftBytes[2]).append(leftBytes[3]));
    msgRight = Command(rightWheelID, QByteArrayLiteral("\x20\x84\x60\x00").append(rightBytes[0]).append(rightBytes[1]).append(rightBytes[2]).append(rightBytes[3]));

    if (msgLeft.ID != mERROR && msgRight.ID != mERROR) return true;

    return false;
}

bool pcanControl::Reset()
{
    TPCANMsg msgLeft, msgRight;

    msgLeft = Command(leftWheelID, QByteArrayLiteral("\x2F\x98\x60\x00\x23\x00\x00\x00"));
    msgRight = Command(rightWheelID, QByteArrayLiteral("\x2F\x98\x60\x00\x23\x00\x00\x00"));

    if (msgLeft.ID == mERROR || msgRight.ID == mERROR) return false;

    msgLeft = Command(leftWheelID, QByteArrayLiteral("\x2F\x60\x60\x00\x06\x00\x00\x00"));
    msgRight = Command(rightWheelID, QByteArrayLiteral("\x2F\x60\x60\x00\x06\x00\x00\x00"));

    if (msgLeft.ID == mERROR || msgRight.ID == mERROR) return false;

    msgLeft = Command(leftWheelID, QByteArrayLiteral("\x2B\x40\x60\x00\x0F\x00\x00\x00"));
    msgRight = Command(rightWheelID, QByteArrayLiteral("\x2B\x40\x60\x00\x0F\x00\x00\x00"));

    if (msgLeft.ID == mERROR || msgRight.ID == mERROR) return false;

    msgLeft = Command(leftWheelID, QByteArrayLiteral("\x2B\x40\x60\x00\x1F\x00\x00\x00"));
    msgRight = Command(rightWheelID, QByteArrayLiteral("\x2B\x40\x60\x00\x1F\x00\x00\x00"));

    if (msgLeft.ID == mERROR || msgRight.ID == mERROR) return false;

    Stop();
    SetMode(Mode);

    return true;
}
