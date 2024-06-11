#define kpg 1000   
#define kigh 2
#define kigl 20
#define kpd 0.25
#define PWM_SX 10
#define PWM_DX 11
#define PWM_SERVO 22
#define EN_DRIVER 28
#define SERVORIGHT 4000     //in realta è left
#define SERVOCENTER 13500
#define SERVOLEFT 20000
#define DUTY 45000

extern float intg;

void initPwm(void);
long calcpid(void);
void setpwm(int8_t go, long correction);
void servo(int angle);
void kit(bool senso, int num);
float ABS(float n);