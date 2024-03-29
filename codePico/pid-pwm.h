#define kpg 1000   
#define kigh 1
#define kigl 20
#define kpd 0.250
#define PWM_SX 10
#define PWM_DX 11
#define PWM_SERVO 22
#define EN_DRIVER 28
#define SERVORIGHT 4500
#define SERVOCENTER 11500
#define SERVOLEFT 19300
#define DUTY 40000

extern float intg;

void initPwm(void);
long calcpid(void);
void setpwm(int8_t go, long correction);
void servo(int angle);
void kit(bool senso, int num);
float ABS(float n);