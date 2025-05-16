#pragma once
#include <wiringPi.h>
#include <stdint.h>

class Encoder
{
private:
    
    int pinA;
    int pinB;
    float lastSpeed[10] = {0};
    int lastIndexSpeed=0;
    uint8_t lastPhase;
    
    /* data */
    uint8_t identifier_phase();
    void  (* externalHandler)(void);
public:
    // you need to encapsulate the instanciated EncoderHandler in externalHandler;
    long long pos=0;
    void  EncoderHandler();
    Encoder(int pinA,int pinB);
    Encoder();
    ~Encoder();
    void init(void (*function)(void));
    
};

