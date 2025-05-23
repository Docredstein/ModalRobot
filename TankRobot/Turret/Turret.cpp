#include "Turret.hpp"

Turret::Turret(int pinTheta, int pinPhi, float maxTheta, float maxPhi)
{
    this->pinTheta = pinTheta;
    this->pinPhi = pinPhi;
    this->theta = 0;
    this->phi = 0;
    this->maxTheta = maxTheta;
    this->maxPhi = maxPhi;
}
void Turret::init()
{
    std::cout << "Starting Turret" << std::endl;
    pinMode(pinTheta, OUTPUT);
    pinMode(pinPhi, OUTPUT);
    pthread_create(&phiThread, NULL, Turret::functionEntryPointPhi, this);
    pthread_create(&thetaThread, NULL, Turret::functionEntryPointTheta, this);
}
Turret::~Turret()
{
    digitalWrite(pinTheta, LOW);
    digitalWrite(pinPhi, LOW);
    pinMode(pinTheta, INPUT);
    pinMode(pinPhi, INPUT);
    pthread_cancel(phiThread);
    pthread_cancel(thetaThread);
    pthread_join(phiThread, NULL);
    pthread_join(thetaThread, NULL);
}

void Turret::move(float theta, float phi)
{
    if (theta > maxTheta)
    {
        theta = maxTheta;
    }
    else if (theta < 0)
    {
        theta = 0;
    }
    if (phi > maxPhi)
    {
        phi = maxPhi;
    }
    else if (phi < 0)
    {
        phi = 0;
    }
    this->theta = theta;
    this->phi = phi;
}
void *Turret::moveTheta()
{
    while (true)
    {
        int upTime = (int)((theta / maxTheta) * 1000) + 1000;
        if (micros() - lastTickTheta > 20000)
        {
            digitalWrite(pinTheta, HIGH);
            lastTickTheta = micros();
        }
        else if (micros() - lastTickTheta > upTime)
        {
            digitalWrite(pinTheta, LOW);
        }
        else
        {
            digitalWrite(pinTheta, HIGH);
        }
        delayMicroseconds(10);
    }
}
void *Turret::movePhi()
{
    std::cout << "Starting movePhi" << std::endl;
    unsigned int lastTickPhi = micros();
    while (true)
    {
        int upTime = (int)((phi / maxPhi) * 1000) + 1000;

        if (micros() - lastTickPhi > 20000)
        {
            digitalWrite(pinPhi, HIGH);
            lastTickPhi = micros();
            //std::cout << std::endl;
        }
        else if (micros() - lastTickPhi > upTime)
        {
            digitalWrite(pinPhi, LOW);
            //std::cout << ".";
        }
        else
        {
            digitalWrite(pinPhi, HIGH);
            //std::cout << "°";
        }
        delayMicroseconds(10);
    }
}