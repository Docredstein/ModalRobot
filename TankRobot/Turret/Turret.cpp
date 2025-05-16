#include "Turret.hpp"

Turret::Turret(int pinTheta,int pinPhi, float maxTheta, float maxPhi) {
    this->pinTheta=pinTheta;
    this->pinPhi=pinPhi;
    pinMode(pinTheta, OUTPUT);
    pinMode(pinPhi, OUTPUT);
    this->theta=0;
    this->phi=0;
    this->maxTheta=maxTheta;
    this->maxPhi=maxPhi;


}
void Turret::~Turret() {
    digitalWrite(pinTheta, LOW);
    digitalWrite(pinPhi, LOW);
    pinMode(pinTheta, INPUT);
    pinMode(pinPhi, INPUT);
    p_thread_cancel(phiThread);
    p_thread_cancel(thetaThread);
    p_thread_join(phiThread, NULL);
    p_thread_join(thetaThread, NULL);
}
    
void Turret::move(float theta,float phi){
    if (theta>maxTheta) {
        theta=maxTheta;
    } else if (theta<0) {
        theta=0;
    }
    if (phi>maxPhi) {
        phi=maxPhi;
    } else if (phi<0) {
        phi=0;
    }
    this->theta=theta;
    this->phi=phi;
}
void Turret::moveTheta() {
    while (true) {
        int upTime = (int)((theta/maxTheta)*1000)+1000;
        if (micros()- lastTickTheta>upTime) {
            digitalWrite(pinTheta, LOW);
            
        } else if (micros()- lastTickTheta>20000) {
            digitalWrite(pinTheta, HIGH);
            lastTickTheta = micros();
        }
        else {
            digitalWrite(pinTheta, HIGH);
        }
        delayMicroseconds(100);
    }
}
void Turret::movePhi() {
    while (true) {
        int upTime = (int)((phi/maxPhi)*1000)+1000;
        if (micros()- lastTickPhi>upTime) {
            digitalWrite(pinPhi, LOW);
            
        } else if (micros()- lastTickPhi>20000) {
            digitalWrite(pinPhi, HIGH);
            lastTickPhi = micros();
        }
        else {
            digitalWrite(pinPhi, HIGH);
        }
        delayMicroseconds(100);
    }
}