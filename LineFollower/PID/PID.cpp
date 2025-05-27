#include "PID.hpp"


PID::PID(float kp, float ki, float kd,float maxAccumulator)
{
    m_Kp = kp;
    m_Ki = ki;
    m_Kd = kd;
    m_accumulator =0;
    m_lastMicros=micros();
    m_maxAccumulator=maxAccumulator;
    m_initialized = false;
    m_lastError=0;
}

PID::~PID()
{
}
float PID::getInt() {
    return m_accumulator;
}
float PID::update(float const error) {

    //float error = target - measure;
    long time = micros();
    if (m_Ki!=0) {
    m_accumulator+=error*(time-m_lastMicros)*1e-6;
    m_accumulator = (m_accumulator>m_maxAccumulator)?m_maxAccumulator:m_accumulator;
    m_accumulator = (m_accumulator<-m_maxAccumulator)?-m_maxAccumulator:m_accumulator;
    }

    float out = m_Kp*error+m_Ki*m_accumulator+m_Kd*(error - m_lastError)/(time-m_lastMicros);
    //m_accumulator = m_accumulator/std::exp((time-m_lastMicros)*20*1e-6); mauvaise idée
    m_lastError=error;
    m_lastMicros=time;
    return out;

    

}
void PID::resetAcc() {
    m_accumulator = 0;
}