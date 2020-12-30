#ifndef PCANCONTROL_H
#define PCANCONTROL_H
#include "PCANBasic.h"
#include <QMutex>
#include <windows.h>

class pcanControl
{
public:
    uint mERROR = 0;

    int Mode = -1;

    int Mode1 = 1;
    int Mode2 = 2;
    int Mode3 = 3;

    double SlowDown = 1;
    //double SlowDown = 19.38;

    uint leftWheelID  = 0x602;
    uint rightWheelID = 0x601;

    pcanControl();

    TPCANMsg Command(uint ID, QByteArray cmd);
    bool SetMode(int mode);
    bool Start();
    bool Stop();
    bool SetSpeed(double leftRpm,double rightRpm);
    bool SetMaxSpeed(double leftRpm, double rightRpm);
    bool SetAcceleratedSpeed(double leftRps, double rightRps);
    bool SetDeceleratedSpeed(double leftRps, double rightRps);
    bool Reset();
};

#endif // PCANCONTROL_H
