#pragma once
#include <wiringPi.h>
class PID
{
private:
    float   m_Kp;
    float   m_Ki;
    float   m_Kd;
    float   m_lastError;
    long    m_lastMicros;
    float   m_accumulator;
    bool    m_initialized;
    float   m_maxAccumulator;
public:
    PID(float kp=1.0f,float ki =0.0f, float kd=0.0f,float maxAccumulator=20000);
    ~PID();
   float update(float error);
};


