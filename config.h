constexpr float pi = 3.1415;
//#define NO_SHOW
#define MOTOR_CONSTANT 255 / (2*4 * 200)
#define BARY_ALGO
#define MOTOR_KP 0.07 //0.04 on air
#define MOTOR_KI 0.15 //0.015 on air
#define MOTOR_KD 0.01
#define MOTOR_INT_LIMIT 200
#define SPEED_CONSTANT 400
#define SPEED_AVERAGE_K 2

#define FRICTION_CONSTANT 0
//#define MANUAL_MODE
//#define PROP_ALGO
//#define WHATTHEMOTORDOIN
//#define WHATTHEPIDDOIN
//#define SHOW_MASK