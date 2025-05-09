constexpr float pi = 3.1415;
//#define NO_SHOW
#define MOTOR_CONSTANT 255 / (2*4 * 200)
#define BARY_ALGO
#define MOTOR_KP 0.08 //0.04 on air
#define MOTOR_KI 0.05 //0.015 on air
#define MOTOR_KD 0
#define MOTOR_INT_LIMIT 2000
#define SPEED_CONSTANT 200
#define SPEED_AVERAGE_K 2

#define FRICTION_CONSTANT 20
//#define PROP_ALGO
//#define WHATTHEMOTORDOIN
//#define WHATTHEPIDDOIN