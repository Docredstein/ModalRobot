#include <lccv.hpp>
#include <libcamera_app.hpp>
#include <opencv2/opencv.hpp>
#include <wiringPi.h>
#include "MotorControl.hpp"
#include "PID.hpp"



uint8_t lastphase[3] = {0, 0, 0};
int Position[3] = {0, 0, 0};
int ENC_A[3] = {21,0,0};
int ENC_B[3] = {22,0,0};


MotorControl driverA = MotorControl();
MotorControl driverB = MotorControl(0x0e);

typedef struct motorStruct {
    MotorControl driver;
    bool side;
} motorType;
motorType motorList[3] = { motorType({driverA,MOTORA}),motorType({driverA,MOTORB}),motorType({driverB,MOTORA})};





uint8_t identifier_phase(int pinA, int pinB)
{
    uint8_t const LUT[] = {0, 1, 3, 2};
    return LUT[2 * digitalRead(pinA) + digitalRead(pinB)];
}

void generalEncoderHandler(uint motor)
{
    uint phase = identifier_phase(ENC_A[motor], ENC_B[motor]);
    if (phase == (lastphase[motor] + 1) % 4)
    {
        Position[motor]++;
    }
    else if (phase == (lastphase[motor] - 1) % 4)
    {
        Position[motor]--;
    }
    lastphase[motor] = phase;
}
void EncoderHandler0() {generalEncoderHandler(0);}
void EncoderHandler1() {generalEncoderHandler(1);}
void EncoderHandler2() {generalEncoderHandler(2);}


int main(int argc, char **argv)
{

        for (int i =0;i<3;i++) {
        pinMode(ENC_A[i],INPUT);
        pinMode(ENC_B[i],INPUT);
        lastphase[i]=identifier_phase(ENC_A[i],ENC_B[i]);
        }
        wiringPiISR(ENC_A[0],INT_EDGE_BOTH, &EncoderHandler0 );
        wiringPiISR(ENC_B[0],INT_EDGE_BOTH, &EncoderHandler0 );
        wiringPiISR(ENC_A[1],INT_EDGE_BOTH, &EncoderHandler1 );
        wiringPiISR(ENC_B[1],INT_EDGE_BOTH, &EncoderHandler1 );
        wiringPiISR(ENC_A[2],INT_EDGE_BOTH, &EncoderHandler2 );
        wiringPiISR(ENC_B[2],INT_EDGE_BOTH, &EncoderHandler2 );
    
    int width = 1280;
    int height = 720;
    int totalpixel = width * height;
    float validlock = 0.02;

    cv::Mat image = cv::Mat(width, height, CV_8UC3);
    cv::Mat imageHSV = cv::Mat(width, height, CV_8UC3);
    cv::Mat mask_red, mask_blue, out_red, out_blue;

    lccv::PiCamera cam;
    cam.options->video_width = width;
    cam.options->video_height = height;
    cam.options->framerate = 60;
    cam.options->verbose = true;

    // cv::namedWindow("red",cv::WINDOW_NORMAL);
    // cv::namedWindow("blue",cv::WINDOW_NORMAL);
    cv::namedWindow("Video2", cv::WINDOW_NORMAL);
    std::cout << "Starting" << std::endl;
    cam.startVideo();

    cv::Scalar lowerbound_red = cv::Scalar(160, 200, 0);
    cv::Scalar higherbound_red = cv::Scalar(255, 255, 255);
    cv::Scalar lowerbound_blue = cv::Scalar(100, 120, 0);
    cv::Scalar higherbound_blue = cv::Scalar(150, 255, 255);

    int ch = 0;
    bool correct_reading = false;


   

        while (ch != 27)
    {
        /*out_red = cv::Mat::zeros(width,height,CV_8UC3);
        out_blue = cv::Mat::zeros(width,height,CV_8UC3);*/
        correct_reading = false;
        double x_blue = 0;
        double y_blue = 0;
        double x_red = 0;
        double y_red = 0;

        cam.getVideoFrame(image, 2000);
        cv::cvtColor(image, imageHSV, cv::COLOR_BGR2HSV);
        cv::inRange(imageHSV, lowerbound_red, higherbound_red, mask_red);    //==>mask
        cv::inRange(imageHSV, lowerbound_blue, higherbound_blue, mask_blue); //==>mask
        cv::Moments blue_moment = cv::moments(mask_blue, true);
        cv::Moments red_moment = cv::moments(mask_red, true);

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
#ifdef DEBUG
        cv::Point bary_red = cv::Point(std::floor(x_red), std::floor(y_red));
        cv::Point bary_blue = cv::Point(std::floor(x_blue), std::floor(y_blue));
        cv::circle(image, bary_red, 25, cv::Scalar(0, 0, 255), (red_moment.m00 / totalpixel > validlock ? -1 : 5));
        cv::circle(image, bary_blue, 25, cv::Scalar(255, 0, 0), (blue_moment.m00 / totalpixel > validlock ? -1 : 5)); // BGR
        // cv::circle(image,cv::Point(0,0),25,(correct_reading?cv::Scalar(0,255,0):cv::Scalar(0,0,255)),-1);
        /*cv::bitwise_and(image,image,out_red,mask_red);
        cv::bitwise_and(image,image,out_blue,mask_blue);*/
        // out = image*mask;
        // cv::threshold(image,out,)
        // cv::cvtColor(imageHSV,image,cv::COLOR_HSV2RGB);
        // cv::imshow("red",out_red);
        // cv::imshow("blue",out_blue);

        cv::imshow("Video2", image);
        ch = cv::waitKey(5);
#endif
        









    }
    cam.stopVideo();
    cv::destroyAllWindows();

    return 0;
}