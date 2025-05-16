#pragma once
#include <stdint.h>
#include <wiringPi.h>
#include <wiringPiI2C.h>
#include <stdio.h>
#define MOTORA 0
#define MOTORB 1
class MotorControl
{
private:
    uint8_t I2C_Addr;
    bool _M1_direction;
    bool _M2_direction;
    uint8_t _speed1;
    uint8_t _speed2;
    int m_fd;
public:
    MotorControl(uint8_t address);
    MotorControl();
    ~MotorControl();
    void setDirection(bool dirA, bool dirB);
    void setSpeed(bool Motor,int speed);
};

