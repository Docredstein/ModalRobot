#pragma once
#include <wiringPi.h>
#include <pthread.h>
#include <iostream>
class Turret
{
private:
    pthread_t phiThread;
    pthread_t thetaThread;
    void *moveTheta();
    void *movePhi();
    float maxTheta = 180;
    float maxPhi = 180;
    int lastTickTheta = 0;
    int lastTickPhi = 0;

public:
    float theta, phi = 0;
    int pinTheta, pinPhi;
    void init();
    static void *functionEntryPointTheta(void *p)
    {
        ((Turret *)p)->moveTheta();
        return nullptr;
    }
    static void *functionEntryPointPhi(void *p)
    {
        ((Turret *)p)->movePhi();
        return nullptr;
    }
    Turret(int pinTheta=0, int pinPhi=1, float maxTheta = 180, float maxPhi = 180);
    ~Turret();
    void move(float theta, float phi);
};
