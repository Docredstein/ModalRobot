#pragma once
#include <wiringPi.h>

class Turret
{
private:
    pthread_t phiThread;
    pthread_t thetaThread;
    void *moveTheta(void *vargp);
    void *movePhi(void *vargp);
    float maxTheta = 180;
    float maxPhi = 180;
    int lastTickTheta = 0;
    int lastTickPhi = 0;
public:
    float theta, phi = 0;
    int pinTheta,pinPhi;

    Turret(int pinTheta,int pinPhi,float maxTheta=180,float maxPhi=180);
    ~Turret();
    void move(float theta, float phi);
    
};

