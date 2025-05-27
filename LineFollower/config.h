constexpr float pi = 3.1415;
//#define NO_SHOW
#define MOTOR_CONSTANT 255 / (2600.0f)
#define BARY_ALGO
#define MOTOR_KP 0.1//0.07 On steroid //0.04 on air
#define MOTOR_KI 0.05//0.15 //0.015 on air
#define MOTOR_KD 0.2//0.01
#define MOTOR_INT_LIMIT 5000
#define SPEED_CONSTANT 2600
#define SPEED_AVERAGE_K 2
#define REDUCTION_RATIO 1
#define FRICTION_CONSTANT 56
//#define MANUAL_MODE
//#define PROP_ALGO
//#define WHATTHEMOTORDOIN
//#define WHATTHEPIDDOIN
//#define SHOW_MASK
#define SIDE 2




#if defined(SHOW_MASK)&&defined(NO_SHOW)
#error "If SHOW_MASK is used, there must be a desktop environement enabled (NO_SHOW must be undefined)"
#endif