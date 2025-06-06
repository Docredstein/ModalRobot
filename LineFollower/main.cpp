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
// constexpr float pi = 3.1415;
/*
uint8_t lastphase[3] = {0, 0, 0};
long Position[3] = {0, 0, 0};
*/
/*int ENC_A[3] = {1, 21, 3};
int ENC_B[3] = {24, 22, 4};*/
int ENC_A[3] = {24, 22, 3};
int ENC_B[3] = {1, 21, 4};
float lastCommandAfterPID[3] = {0};
Encoder Encoderlist[3];
float angle = 0;
float angleDeg = 0;
bool lost = false;
bool valid_lock_near = false;
bool valid_lock_far = false;
int width = 1280;
int height = 720;
void EncoderHandler1() { Encoderlist[0].EncoderHandler(); }
void EncoderHandler2() { Encoderlist[1].EncoderHandler(); }
void EncoderHandler3() { Encoderlist[2].EncoderHandler(); }

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
PID pidDroite = PID(1, 0.001, 0, 100);
PID pidRot = PID(0.001, 0.001, 0, 200);

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
float consigne[3] = {400, 600, 800};
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
    motor1.sens = 0; // 0

    motorType motor2;
    motor2.driver = &driverA;
    motor2.side = MOTORB;
    motor2.sens = 0; // 0

    motorType motor3;
    motor3.driver = &driverB;
    motor3.side = MOTORB;
    motor3.sens = 0; // 0

    // swap
    motorlist[0] = motor2;
    motorlist[1] = motor1;
    motorlist[2] = motor3;
    return;
}
float CommandeAfterPidGlobal[2] = {0};
void *DroiteUpdateThread(void *argv)
{
    while (!stopFlag)
    {
        /*CommandeAfterPidGlobal[0] = pidDroite.update(90 - (180 - angleDeg));
        CommandeAfterPidGlobal[1] = pidRot.update(90 - (180 - angleDeg));*/
        // CommandeAfterPidGlobal[0] = pidDroite.update(90 - (angleDeg));
        CommandeAfterPidGlobal[0] = pidDroite.update((x_near - width / 2.0f) / width);
        CommandeAfterPidGlobal[1] = pidRot.update(90 - (angleDeg));
    }
    return nullptr;
}

void *MotorUpdateThread(void *argv)
{
    while (!stopFlag)
    {
        for (int i = 0; i < 3; i++)
        {
            float speedVal = PIDmotor[i].update(REDUCTION_RATIO*consigne[(i + SIDE) % 3] - speed[i]);
            speedVal = speedVal + consigne[i] * MOTOR_CONSTANT;
            speedVal += (std::abs(consigne[i]) < 400 && std::abs(consigne[i])>5) ? sgn(consigne[i]) * FRICTION_CONSTANT : 0;
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
/*   while (true)
    {
        for (int i = 0; i < 3; i++)
        {

            motorList[i].driver->setSpeed(motorList[i].side, (motorList[i].sens ? -1 : 1) * 100);
                motorList[(i+1)%3].driver->setSpeed(motorList[(i+1)%3].side, 0);
                motorList[(i+2)%3].driver->setSpeed(motorList[(i+2)%3].side, 0);
                std::cout<<(motorList[i].sens ? -1 : 1)  * 100<<std::endl;


            std::cout<<"Motor"<<i<<std::endl;
                    delay(5000);
        }
    }*/
    pthread_create(&speedThread, NULL, &getSpeed, NULL);
    pthread_create(&motorThread, NULL, &MotorUpdateThread, NULL);
    // pthread_create(&averageCalc, NULL, &speedAverageCalc, NULL);

#ifdef BARY_ALGO
    pthread_create(&consigneThread, NULL, &DroiteUpdateThread, NULL);
#endif

    int totalpixel = width * height;
    float validlock = 0.02;

    cv::Mat image = cv::Mat(height, width, CV_8UC3);
    cv::Mat imageHSV = cv::Mat(height, width, CV_8UC3);
    cv::Mat mask_red, mask_blue, out_red, out_blue, mask_color_up, mask_color_down;
    cv::Mat mask_up(height, width, CV_8U, cv::Scalar(0)), mask_down(height, width, CV_8U, cv::Scalar(0));
    mask_up(cv::Rect(0, 0, width, height / 2)) = 255;
    mask_down(cv::Rect(0, height / 2, width, height / 2)) = 255;

    lccv::PiCamera cam;
    cam.options->video_width = width;
    cam.options->video_height = height;
    cam.options->framerate = 60;
    cam.options->verbose = true;

// cv::namedWindow("red",cv::WINDOW_NORMAL);
// cv::namedWindow("blue",cv::WINDOW_NORMAL);
#ifndef NO_SHOW
    cv::namedWindow("Video2", cv::WINDOW_NORMAL);
#ifdef SHOW_MASK
    cv::namedWindow("masked blue", cv::WINDOW_NORMAL);
    cv::namedWindow("masked red", cv::WINDOW_NORMAL);
#endif
#endif
    std::cout << "Starting" << std::endl;
    cam.startVideo();

    cv::Scalar lowerbound_red = cv::Scalar(160, 100, 0);
    cv::Scalar higherbound_red = cv::Scalar(180, 255, 255);
    cv::Scalar lowerbound2_red = cv::Scalar(0, 100, 0);
    cv::Scalar higherbound2_red = cv::Scalar(20, 255, 255);

    cv::Scalar lowerbound_blue = cv::Scalar(100, 80, 0);
    cv::Scalar higherbound_blue = cv::Scalar(150, 255, 255);

    int ch = 0;
    bool correct_reading = false;
    bool followingblue = false;

    // motorList[0].driver->setSpeed(motorList[0].side, 255);
    // motorList[1].driver->setSpeed(motorList[1].side, 255);
#ifndef NO_SHOW
    while (ch != 27)
#else
    while (true)
#endif

    {
        clearScreen();
#ifdef SHOW_MASK
        out_blue.setTo(cv::Scalar(0, 0, 0));
        out_red.setTo(cv::Scalar(0, 0, 0));
#endif
        /*out_red = cv::Mat::zeros(width,height,CV_8UC3);
        out_blue = cv::Mat::zeros(width,height,CV_8UC3);*/
        correct_reading = false;
        double x_blue_near = 0;
        double y_blue_near = 0;
        double x_red_near = 0;
        double y_red_near = 0;
        double x_blue_far = 0;
        double y_blue_far = 0;
        double x_red_far = 0;
        double y_red_far = 0;
        int lostTime = -1;
        int searchTime = 1000;
        bool turnSide = false; // false = right, true = left
        cv::Mat maskRed2;
        cam.getVideoFrame(image, 2000);
        cv::flip(image, image, -1);
        cv::cvtColor(image, imageHSV, cv::COLOR_BGR2HSV);
        cv::inRange(imageHSV, lowerbound_red, higherbound_red, mask_red); //==>mask
        cv::inRange(imageHSV, lowerbound2_red, higherbound2_red, maskRed2);
        cv::bitwise_or(mask_red, maskRed2, mask_red);                        //==>mask
        cv::inRange(imageHSV, lowerbound_blue, higherbound_blue, mask_blue); //==>mask
        // std::cout << "blue : " << mask_blue.size() << "mask" << mask_up.size() << std::endl;
        float commande[3] = {0.0f, 0, 0};
#if defined(BARY_ALGO) || defined(PROP_ALGO)
        cv::bitwise_and(mask_up, mask_blue, mask_color_up);
        cv::bitwise_and(mask_down, mask_blue, mask_color_down);
        cv::Moments blue_moment_up = cv::moments(mask_color_up, true);
        cv::Moments blue_moment_down = cv::moments(mask_color_down, true);
#ifdef SHOW_MASK
        cv::bitwise_and(image, image, out_blue, mask_color_down);
        cv::imshow("masked blue", out_blue);
#endif
        cv::bitwise_and(mask_up, mask_red, mask_color_up);
        cv::bitwise_and(mask_down, mask_red, mask_color_down);
        cv::Moments red_moment_up = cv::moments(mask_color_up, true);
        cv::Moments red_moment_down = cv::moments(mask_color_down, true);
#ifdef SHOW_MASK
        cv::bitwise_and(image, image, out_red, mask_color_up);
        cv::imshow("masked red", out_red);
#endif
        // cv::Moments red_moment = cv::moments(mask_red, true);

        /* OLD
                if (blue_moment.m00 > 0)
                {
                    x_blue = blue_moment.m10 / blue_moment.m00;
                    y_blue = blue_moment.m01 / blue_moment.m00;
                }

                if (red_moment.m00 > 0)
                {
                    x_red = red_moment.m10 / red_moment.m00;
                    y_red = red_moment.m01 / red_moment.m00;
                }

                if (red_moment.m00 > 100 || blue_moment.m00 > 100)
                {
                    correct_reading = true;
                }
        */
        /*bool correct_blue = blue_moment.m00 / totalpixel > validlock;
        bool correct_red = red_moment.m00 / totalpixel > validlock;*/
        if (blue_moment_down.m00 > 0)
        {
            x_blue_near = blue_moment_down.m10 / blue_moment_down.m00;
            y_blue_near = blue_moment_down.m01 / blue_moment_down.m00;
        }

        if (red_moment_down.m00 > 0)
        {
            x_red_near = red_moment_down.m10 / red_moment_down.m00;
            y_red_near = red_moment_down.m01 / red_moment_down.m00;
        }

        if (red_moment_down.m00 > 100 || blue_moment_down.m00 > 100)
        {
            correct_reading = true;
        }
        if (blue_moment_up.m00 > 0)
        {
            x_blue_far = blue_moment_up.m10 / blue_moment_up.m00;
            y_blue_far = blue_moment_up.m01 / blue_moment_up.m00;
        }

        if (red_moment_up.m00 > 0)
        {
            x_red_far = red_moment_up.m10 / red_moment_up.m00;
            y_red_far = red_moment_up.m01 / red_moment_up.m00;
        }

        if (red_moment_up.m00 > 100 || blue_moment_up.m00 > 100)
        {
            correct_reading = true;
        }

        bool correct_red_near = red_moment_down.m00 / totalpixel > validlock;
        bool correct_red_far = red_moment_up.m00 / totalpixel > validlock;
        bool correct_blue_near = blue_moment_down.m00 / totalpixel > validlock;
        bool correct_blue_far = blue_moment_up.m00 / totalpixel > validlock;

        if (!correct_blue_far && !correct_blue_near && (correct_red_far || correct_red_near))
        {
            followingblue = false;
            lost = false;
        }
        else if (!correct_red_far && !correct_red_near && (correct_blue_far || correct_blue_near))
        {
            followingblue = true;
            lost = false;
        }
        else if (!(correct_blue_far || correct_red_far || correct_blue_near || correct_red_near))
        {
            lost = true;
        }

        valid_lock_near = (correct_blue_near && followingblue) || (correct_red_near && !followingblue);
        valid_lock_far = (correct_blue_far && followingblue) || (correct_red_far && !followingblue);

        if (followingblue)
        {
            x_near = x_blue_near;
        }
        else
        {
            x_near = x_red_near;
        }
        cv::Point bary_red_near = cv::Point(std::floor(x_red_near), std::floor(y_red_near));
        cv::Point bary_blue_near = cv::Point(std::floor(x_blue_near), std::floor(y_blue_near));
        cv::Point bary_red_far = cv::Point(std::floor(x_red_far), std::floor(y_red_far));
        cv::Point bary_blue_far = cv::Point(std::floor(x_blue_far), std::floor(y_blue_far));
#ifndef NO_SHOW
        cv::circle(image, bary_red_far, 25, cv::Scalar(0, 0, 255), (correct_red_far ? -1 : 5));
        cv::circle(image, bary_red_near, 25, cv::Scalar(0, 0, 255), (correct_red_near ? -1 : 5));
        cv::circle(image, bary_blue_far, 25, cv::Scalar(255, 0, 0), (correct_blue_far ? -1 : 5));   // BGR
        cv::circle(image, bary_blue_near, 25, cv::Scalar(255, 0, 0), (correct_blue_near ? -1 : 5)); // BGR
#endif
                                                                                                    // cv::circle(image,cv::Point(0,0),25,(correct_reading?cv::Scalar(0,255,0):cv::Scalar(0,0,255)),-1);
// cv::bitwise_and(image,image,out_red,mask_red);
// cv::bitwise_and(image,image,out_blue,mask_blue);
//  out = image*mask;
//  cv::threshold(image,out,)
//  cv::cvtColor(imageHSV,image,cv::COLOR_HSV2RGB);
//  cv::imshow("red",out_red);
//  cv::imshow("blue",out_blue);

// float res = dirPID.update(width / 2.0f - x_red);
// motorList[2].driver.setSpeed(motorList[2].side, std::floor(res));
#ifdef WHATTHEPIDDOIN
        while (true)
        {
            clearScreen();
            for (int i = 0; i < 3; i++)
            {
                std::cout << i << " : POS=" << Encoderlist[i].pos << " COMMANDE)"<< consigne[i] << " ;SPEED=" << speed[i] << " ;PID=" << std::floor(lastCommandAfterPID[i]) << " ;error:" << consigne[i] - speed[i] << ";Integral :" << PIDmotor[i].getInt() << std::endl;
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

        if (followingblue)
        {
            angle = std::atan2(height - y_blue_far, x_blue_far - (width / 2));
        }
        else
        {
            angle = std::atan2(height - y_red_far, x_red_far - (width / 2));
        }
#ifndef NO_SHOW
        cv::line(image, cv::Point(width / 2, height), cv::Point(width / 2 + 1000 * std::cos(angle), height - 1000 * std::sin(angle)), cv::Scalar(0, 255, 0), 5);
        std::cout << "angle=" << angle << " y red : " << y_red_far << std::endl;
#endif

#ifndef NO_SHOW
        cv::imshow("Video2", image);
        ch = cv::waitKey(5);
#endif
        angleDeg = angle * 180 / pi;

#ifdef BARY_ALGO
#ifndef MANUAL_MODE
        /*if (std::abs(angleDeg - 90) < 10)
        {
            commande[0] = 1;
            commande[1] = 0;
            commande[2] = 0;
        }

        else if (std::abs(angleDeg - 90) < 30)
        {
            commande[0] = 0.5;
            commande[1] = CommandeAfterPidGlobal[1];
            commande[2] = 0;
        }
        else
        {
            commande[0] = 0.5;
            commande[1] = 0;
            commande[2] = CommandeAfterPidGlobal[0];
        }*/
        // L'idée, c'est de centrer juste l'x en bas et de s'orienter pour que l'angle soit de ~90°
        if (!lost)
        {
            lostTime = -1;
            commande[0] = 0.5;
            if (valid_lock_near)
            {
                commande[1] = CommandeAfterPidGlobal[0]; // centrage de x_near
            }
            else
            {
                commande[1] = 0;
            }
            if (valid_lock_far)
            {
                commande[2] = CommandeAfterPidGlobal[1]; // alignement de angle
            }
            else
            {
                commande[2] = 0;
            }
        }
        else
        {
            if (lostTime < 0)
            {
                lostTime = millis();
            }
            if ((millis() - lostTime) > searchTime)
            {
                searchTime += 1000;
                lostTime = millis();
                turnSide = !turnSide;
            }
            if (turnSide)
            {
                commande[0] = 0;
                commande[1] = 0;
                commande[2] = -0.25;
            }
            else
            {
                commande[0] = 0;
                commande[1] = 0;
                commande[2] = 0.25;
            }
        }
#else
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

#endif

        /*if (angleDeg - 90>0) {
         commande[1] = -0.5;
        }else {
         commande[1] = 0.5;
        }*/
#endif
#endif // calcul de l'angle des barycentre

#ifdef PROP_ALGO
        commande[2] = (90 - angleDeg) / 90;
#endif

        float consigneMid[3] = {0};

        Holonomic::Convert(commande, consigneMid, false);

        for (int i = 0; i < 3; i++)
        {
            consigne[i] = (int)(SPEED_CONSTANT * consigneMid[i]);
        }
        std::cout << angleDeg << std::endl;
        displayAngle(angle, 20);
        std::cout << "commande : ";
        printBuffer3(commande);
        std::cout << "consigneMid : ";
        printBuffer3(consigneMid);
        std::cout << "consigne : ";
        printBuffer3(consigne);
        std::cout << "Following : ";
        if (followingblue)
        {
            std::cout << "Blue";
        }
        else
        {
            std::cout << "Red";
        }

        std::cout << std::endl;
        if (valid_lock_near)
        {
            std::cout << "->Locked Near<-" << std::endl;
        }
        else
        {
            std::cout << " |No Lock Near|" << std::endl;
        }
        if (valid_lock_far)
        {
            std::cout << "->Locked Far<-" << std::endl;
        }
        else
        {
            std::cout << " |No Lock Far|" << std::endl;
        }
        // std::cout << ch << std::endl;
        delay(10);
    }

    cam.stopVideo();
    stop(0);
#ifndef NO_SHOW
    cv::destroyAllWindows();
#endif
    return 0;
}