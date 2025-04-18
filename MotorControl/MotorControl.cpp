#include "MotorControl.hpp"


MotorControl::MotorControl(uint8_t address)
{
    this->I2C_Addr=address;
    this->m_fd=wiringPiI2CSetup(address);
    this->_M1_direction=0;
    this->_M2_direction=0;
    this->_speed1=0;
    this->_speed2=0;
}
MotorControl::MotorControl() {
    this->I2C_Addr=0x0f;
    this->m_fd=wiringPiI2CSetup(0x0f);
    this->_M1_direction=0;
    this->_M2_direction=0;
    this->_speed1=0;
    this->_speed2=0;
}

MotorControl::~MotorControl()
{
}

void MotorControl::setDirection(bool dirA, bool dirB) {
    
    uint8_t DirLut[4] = {0x0a,0x06,0x09,0x05};
    //uint8_t Data[2] = {DirLut[(2*dirA+dirB)],0};
    //wiringPiI2CWriteBlockData(this->I2C_Addr,0xaa,Data,2);
    wiringPiI2CWriteReg16(this->m_fd,0xaa,DirLut[(dirA+2*dirB)]);

}
void MotorControl::setSpeed(bool Motor,int speed) {
    //printf("Salut\n");
    if (Motor == MOTORA) {
        if (speed >= 0) {
            this->_M1_direction = 1;
            speed = speed > 255 ? 255 : speed;
            this->_speed1 = speed;
        } else  {
            this->_M1_direction = 0;
            speed = speed < -255 ? 255 : -(speed);
            this->_speed1 = speed;
        }
    } else if (Motor == MOTORB) {
        if (speed >= 0) {
            this->_M2_direction = 1;
            speed = speed > 255 ? 255 : speed;
            this->_speed2 = speed;
        } else {
            this->_M2_direction = 0;
            speed = speed < -255 ? 255 : -(speed);
            this->_speed2 = speed;
        }
    }
    this->setDirection(this->_M1_direction,this->_M2_direction);
    //uint8_t Data[2] = {this->_speed1,this->_speed2};
    wiringPiI2CWriteReg16(this->m_fd,0x82,((uint16_t)this->_speed1)<<8|this->_speed2);
    //printf("a : %i, b:%i , value : %04x\n",this->_speed1,this->_speed2,((uint16_t)this->_speed1)<<8|this->_speed2);

}