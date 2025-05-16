#include <lccv.hpp>
#include <libcamera_app.hpp>
#include <opencv2/opencv.hpp>
#include <wiringPi.h>
#include "MotorControl.hpp"
#include "PID.hpp"
#include "Holonomic.hpp"
#include "Encoder.hpp"
#include <signal.h>
#include "config.h"
#include "Turret.hpp"
#include "vl53l1_api.h"
// constexpr float pi = 3.1415;
/*
uint8_t lastphase[3] = {0, 0, 0};
long Position[3] = {0, 0, 0};
*/
int ENC_A[3] = {1, 21, 3};
int ENC_B[3] = {24, 22, 4};
float lastCommandAfterPID[3] = {0};

Encoder Encoderlist[3];
Turret turret;
VL53L1_Dev_t Sensor;


float angle = 0;
float angleDeg = 0;
int width = 1280;
int height = 720;
void EncoderHandler1() { Encoderlist[0].EncoderHandler(); }
void EncoderHandler2() { Encoderlist[1].EncoderHandler(); }
void EncoderHandler3() { Encoderlist[2].EncoderHandler(); }
void doLogic(float[3]& commande);
// volatile float speed[SPEED_AVERAGE_K][3] = {0};
volatile float speed[3] = {0};
volatile float speedAverage[3] = {0};
volatile int speedCurrentIndex = 0;
volatile long long lastPos[3] = {0};
volatile long lastTime = 0;
float translation = 0;
float rotation = 0;
float avance = 0;
double x_near;
bool stopFlag = false;

int sgn(float t)
{
    return ((t >= 0) - (t < 0));
}

void *getSpeed(void *vargp)
{
    const int timedelay = 50;
    while (!stopFlag)
    {
        for (int i = 0; i < 3; i++)
        {
            // speed[speedCurrentIndex][i] = (Encoderlist[i].pos - lastPos[i]) * 1e6 / ((micros() - lastTime) * 1.0f);
            speed[i] = (Encoderlist[i].pos - lastPos[i]) * 1e3 / (timedelay * 1.0f);
            lastPos[i] = Encoderlist[i].pos;
        }
        lastTime = micros();
        // speedCurrentIndex = (speedCurrentIndex + 1) % SPEED_AVERAGE_K;

        // delay(100);
        delay(timedelay);
    }
    return nullptr;
}
void *speedAverageCalc(void *vargp)
{
    while (!stopFlag)
    {
        /* for (int i = 0; i < 3; i++)
         {
             speedAverage[i] = 0;
             for (int j = 0; j < SPEED_AVERAGE_K; j++)
             {
                 speedAverage[i] += speed[j][i];
             }
             speedAverage[i] = speedAverage[i] / SPEED_AVERAGE_K;
         }*/
    }
    return nullptr;
}
void EncoderInit()
{
    for (int i = 0; i < 3; i++)
    {
        Encoderlist[i] = Encoder(ENC_A[i], ENC_B[i]);
    }
    Encoderlist[0].init(&EncoderHandler1);
    Encoderlist[1].init(&EncoderHandler2);
    Encoderlist[2].init(&EncoderHandler3);
}

MotorControl driverA = MotorControl();
MotorControl driverB = MotorControl(0x0e);
PID dirPID = PID(0.01, 0.02);

PID PIDmotor[3];
float consigne[3] = {100, 150, 200};
typedef struct motorStruct
{
    MotorControl *driver;
    bool side;
    bool sens;
} motorType;

motorType motorList[3]; //={motorType({driverA, MOTORA}), motorType({driverA, MOTORB}), motorType({driverB, MOTORA})};
void clearScreen()
{
    std::cout << "\x1B[2J\x1B[H";
}
void motorListInit(motorType motorlist[3])
{

    PIDmotor[0] = PID(MOTOR_KP, MOTOR_KI, MOTOR_KD, MOTOR_INT_LIMIT);
    PIDmotor[1] = PID(MOTOR_KP, MOTOR_KI, MOTOR_KD, MOTOR_INT_LIMIT);
    PIDmotor[2] = PID(MOTOR_KP, MOTOR_KI, MOTOR_KD, MOTOR_INT_LIMIT);
    motorType motor1;
    motor1.driver = &driverA;
    motor1.side = MOTORA;
    motor1.sens = 1; // 1

    motorType motor2;
    motor2.driver = &driverA;
    motor2.side = MOTORB;
    motor2.sens = 0; // 0

    motorType motor3;
    motor3.driver = &driverB;
    motor3.side = MOTORB;
    motor3.sens = 1; // 1

    // swap
    motorlist[0] = motor2;
    motorlist[1] = motor1;
    motorlist[2] = motor3;
    return;
}
float CommandeAfterPidGlobal[2] = {0};

void *MotorUpdateThread(void *argv)
{
    while (!stopFlag)
    {
        for (int i = 0; i < 3; i++)
        {
            float speedVal = PIDmotor[i].update(consigne[i] - speed[i]);
            speedVal = speedVal + consigne[i] * MOTOR_CONSTANT;
            speedVal += (std::abs(consigne[i]) < 150) ? sgn(consigne[i]) * FRICTION_CONSTANT : 0;
            // std::cout<<"command value of"<<i<<" = " << std::floor(speedVal) << std::endl;
            lastCommandAfterPID[i] = speedVal;
            motorList[i].driver->setSpeed(motorList[i].side, (motorList[i].sens ? -1 : 1) * std::floor(speedVal));
            delay(100);
        }
    }
    return nullptr;
}
pthread_t speedThread;
pthread_t motorThread;
pthread_t consigneThread;
pthread_t averageCalc;
void stop(int _)
{
    clearScreen();
    std::cout << "==========Stopping==========" << std::endl;
    stopFlag = true;
    pthread_cancel(speedThread);
    std::cout << ".";
    pthread_cancel(motorThread);
    std::cout << ".";
    pthread_cancel(consigneThread);
    std::cout << ".";
    // pthread_cancel(averageCalc);
    std::cout << ".";
    pthread_join(speedThread, NULL);
    std::cout << ".";
    // pthread_join(averageCalc, NULL);
    std::cout << ".";
    pthread_join(motorThread, NULL);
    std::cout << ".";
    pthread_join(consigneThread, NULL);
    for (int i = 0; i < 4; i++)
    {
        std::cout << ".";
        delay(250);
    }
    std::cout << std::endl;
    for (int i = 0; i < 3; i++)
    {
        motorList[i].driver->setSpeed(motorList[i].side, 0);
    }
    exit(_);
}

void printBuffer3(float input[3])
{
    for (int i = 0; i < 3; i++)
    {
        std::cout << input[i] << " ";
    }
    std::cout << std::endl;
}
void printBuffer3(int input[3])
{
    for (int i = 0; i < 3; i++)
    {
        std::cout << input[i] << " ";
    }
    std::cout << std::endl;
}

void displayAngle(float angle, int width)
{
    // angle en radian
    int delta = width * std::sin(pi / 2 - angle) / 2;
    // std::cout << delta;
    for (int i = 0; i < width; i++)
    {
        if (i == (delta + width / 2))
        {
            std::cout << "X";
        }
        else
        {
            std::cout << "-";
        }
    }
    std::cout << std::endl;
}

int main(int argc, char **argv)
{

    signal(SIGINT, stop);

    motorListInit(motorList);
    wiringPiSetup();
    EncoderInit();
    turret = Turret(5, 6);
    VL53L1_DataInit(&Sensor);
#ifdef WHATTHEMOTORDOIN
    while (true)
    {
        std::cout << "A" << std::endl;
        motorList[0].driver->setSpeed(motorList[0].side, 255);
        motorList[1].driver->setSpeed(motorList[1].side, 255);
        delay(10000);
        std::cout << "B" << std::endl;
        motorList[0].driver->setSpeed(motorList[0].side, 255);
        motorList[1].driver->setSpeed(motorList[1].side, -255);
        delay(10000);
        std::cout << "C" << std::endl;
        motorList[0].driver->setSpeed(motorList[0].side, -255);
        motorList[1].driver->setSpeed(motorList[1].side, 255);
        delay(10000);
        std::cout << "D" << std::endl;
        motorList[0].driver->setSpeed(motorList[0].side, -255);
        motorList[1].driver->setSpeed(motorList[1].side, -255);
        delay(10000);
    }
#endif
    /*while (true)
    {
        for (int i = 0; i < 3; i++)
        {

                motorList[i].driver->setSpeed(motorList[i].side, (motorList[i].sens ? -1 : 1)  * 100);
                std::cout<<(motorList[i].sens ? -1 : 1)  * 100<<std::endl;


            std::cout<<"Motor"<<i<<std::endl;

        }
        delay(1000);
    }*/
    pthread_create(&speedThread, NULL, &getSpeed, NULL);
    pthread_create(&motorThread, NULL, &MotorUpdateThread, NULL);
    // pthread_create(&averageCalc, NULL, &speedAverageCalc, NULL);

    int totalpixel = width * height;
    float validlock = 0.02;

    cv::Mat image = cv::Mat(width, height, CV_8UC3);
    cv::Mat imageHSV = cv::Mat(width, height, CV_8UC3);

    lccv::PiCamera cam;
    cam.options->video_width = width;
    cam.options->video_height = height;
    cam.options->framerate = 60;
    cam.options->verbose = true;

// cv::namedWindow("red",cv::WINDOW_NORMAL);
// cv::namedWindow("blue",cv::WINDOW_NORMAL);
#ifndef NO_SHOW
    cv::namedWindow("Video2", cv::WINDOW_NORMAL);
#endif
    std::cout << "Starting" << std::endl;
    cam.startVideo();

    int ch = 0;

#ifndef NO_SHOW
    while (ch != 27)
#else
    while (true)
#endif

    {
        clearScreen();

        cam.getVideoFrame(image, 2000);
        cv::flip(image, image, -1);
        cv::cvtColor(image, imageHSV, cv::COLOR_BGR2HSV);

        float commande[3] = {0.0f, 0, 0};


#ifdef WHATTHEPIDDOIN
        while (true)
        {
            clearScreen();
            for (int i = 0; i < 3; i++)
            {
                std::cout << i << " : POS=" << Encoderlist[i].pos << " ;SPEED=" << speed[i] << " ;PID=" << std::floor(lastCommandAfterPID[i]) << " ;error:" << consigne[i] - speed[i] << ";Integral :" << PIDmotor[i].getInt() << std::endl;
                // std::cout <<"lastPos : " <<lastPos[i] <<  ";lastTime : " << lastTime<<";CurrentPos : "<<Encoderlist[i].pos<<std::endl;
            }
            delay(100);
        }
#endif
        std::cout << "-----------------------" << std::endl;
        for (int i = 0; i < 3; i++)
        {
            std::cout << i << " : POS=" << Encoderlist[i].pos << " ;SPEED=" << speed[i] << " ;PID=" << std::floor(lastCommandAfterPID[i]) << " ;error:" << consigne[i] - speed[i] << ";Integral :" << PIDmotor[i].getInt() << std::endl;
        }

#ifndef NO_SHOW
        cv::imshow("Video2", image);
        ch = cv::waitKey(5);
#endif

#ifdef MANUAL_MODE

        switch (ch)
        {
        case 122:
            commande[0] = 1;
            commande[1] = 0;
            commande[2] = 0;
            break;
        case 115:
            commande[0] = -1;
            commande[1] = 0;
            commande[2] = 0;
            break;
        case 100:
            commande[0] = 0;
            commande[1] = 1;
            commande[2] = 0;
            break;
        case 113:
            commande[0] = 0;
            commande[1] = -1;
            commande[2] = 0;
            break;
        case 101:
            commande[0] = 0;
            commande[1] = 0;
            commande[2] = 1;
            break;
        case 97:
            commande[0] = 0;
            commande[1] = 0;
            commande[2] = -1;
            break;
        default:
            commande[0] = 0;
            commande[1] = 0;
            commande[2] = 0;
            break;
        }
#else 
        doLogic(commande);
#endif
        

        float consigneMid[3] = {0};

        Holonomic::Convert(commande, consigneMid, true);

        for (int i = 0; i < 3; i++)
        {
            consigne[i] = (int)(SPEED_CONSTANT * consigneMid[i]);
        }

        std::cout << "commande : ";
        printBuffer3(commande);
        std::cout << "consigneMid : ";
        printBuffer3(consigneMid);
        std::cout << "consigne : ";
        printBuffer3(consigne);

        std::cout << std::endl;
        std::cout << ch << std::endl;
        delay(10);
    }

    cam.stopVideo();
    stop(0);
#ifndef NO_SHOW
    cv::destroyAllWindows();
#endif
    return 0;
}



void doLogic(float[3] & commande) {
    vl53l1_RangingMeasurementData_t measure;
    VL53L1_ll_version_t version;
    VL53L1_get_version(&Sensor,&version)
    std::cout<<version.ll_major<<"."<<version.ll_minor<<"."<<version.ll_build<<std::endl;
    turret.move(0, 0);
    delay(1000);
    turret.move(90, 180);
    delay(1000);

}