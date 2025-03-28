#include "Encoder.hpp"

Encoder::Encoder(int pinA,int pinB) {
    this->pinA=pinA;
    this->pinB=pinB;
    
    

}
Encoder::~Encoder() {

}
uint8_t Encoder::identifier_phase() {
    uint8_t const LUT[] = {0, 1, 3, 2};
    return LUT[2 * digitalRead(pinA) + digitalRead(pinB)];
}
void Encoder::EncoderHandler() {




    uint8_t phase = identifier_phase();
    if (phase == (lastPhase + 1) % 4)
    {
        pos++;
    }
    else if (phase == (lastPhase - 1) % 4)
    {
        pos--;
    }
    lastPhase = phase;
}

void Encoder::init(void (*function)(void)) {
    this->externalHandler = function;

    pinMode(pinA, INPUT);
    pinMode(pinB, INPUT);
    lastPhase = identifier_phase();
    wiringPiISR(pinA, INT_EDGE_BOTH, this->externalHandler);
    wiringPiISR(pinB, INT_EDGE_BOTH, this->externalHandler);

}
