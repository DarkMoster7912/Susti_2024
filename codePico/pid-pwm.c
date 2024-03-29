#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "pid-pwm.h"
#include "BNO.h"
#include "VL6180X.h"
#include "isr.h"

float intgh = 0, intgl = 0, intd=0;

void initPwm() {
    /// \tag::setup_pwm[]

    // Tell GPIO 0,1,2,3 they are allocated to the PWM
    gpio_set_function(PWM_SX, GPIO_FUNC_PWM);
    gpio_set_function(PWM_DX, GPIO_FUNC_PWM);
    gpio_set_function(PWM_SERVO, GPIO_FUNC_PWM);
    int Motor_Slice = pwm_gpio_to_slice_num(10);
    int Servo_Slice = pwm_gpio_to_slice_num(PWM_SERVO);
    pwm_set_clkdiv(Motor_Slice,  38.15628816);
    pwm_set_wrap(Motor_Slice, 65520);                         //in teria 50Hz
    pwm_set_clkdiv(Servo_Slice,  38.14697265625);
    pwm_set_wrap(Servo_Slice,65535);
    // Set channels output low
    pwm_set_chan_level(Motor_Slice, PWM_CHAN_A, 0);
    pwm_set_chan_level(Motor_Slice, PWM_CHAN_B, 0);
    servo(0);
    // Set the PWM running
    pwm_set_enabled(Motor_Slice, true);
    pwm_set_enabled(Servo_Slice, true);
    /// \end::setup_pwm[]


    // Note we could also use pwm_set_gpio_level(gpio, x) which looks up the
    // correct slice and channel for a given GPIO.

    for(int i=14; i<18; i++){
        gpio_init(i);
        gpio_set_dir(i,true);
        gpio_put(i,false);
    }
    gpio_init(EN_DRIVER);
    gpio_set_dir(EN_DRIVER,true);
    gpio_put(EN_DRIVER,true);
}
long calcpid(void){
    float relangle = relative180(facingDirection+((!turning)?diffdistance:0)*kpd-angles[0]);
    intgh+=(ABS(relangle)>5)?relangle*kigh:-intgh;
    intgl=(1/ABS(relangle))*intgh;
    intgl= !relangle?0: intgl>65000?65000:intgl<-65000?-65000:intgl;
    //printf("relangle:%f angle:%f\n",(long)(relangle*kpg+((ABS(relangle)>5)?intgh:intgl)),relative180(facingDirection+(diffdistance)*kpd-angles[0]),angles[0], diffdistance);
    //return (long)(relangle*kpg+((ABS(relangle)>5)?intgh:0));
    return (long)(relangle*kpg+intgl);
}
void setpwm(int8_t go, long correction){
    int duty = distances[0]*150;
    duty = duty>DUTY?DUTY:duty;
    long pwmdx = ((go>0)?duty:(!go)?0:-duty)-correction;
    long pwmsx = ((go>0)?duty:(!go)?0:-duty)+correction;
    pwmdx=(pwmdx>65520)?65520:(pwmdx<-65520)?-65520:pwmdx;
    pwmsx=(pwmsx>65520)?65520:(pwmsx<-65520)?-65520:pwmsx;
    //printf("duty:%d pwmd:%d pwms:%d\n ", duty, pwmdx, pwmsx);
    gpio_put((pwmdx>0)?14:15,true);
    gpio_put((pwmdx>0)?15:14,false); 
    gpio_put((pwmsx>0)?16:17,true);
    gpio_put((pwmsx>0)?17:16,false);
    pwm_set_gpio_level(PWM_SX, ABS(pwmsx));
    pwm_set_gpio_level(PWM_DX, ABS(pwmdx));
}
float ABS(float n){
    return (n>0)?n:-n;
}
float crop(float n,float up, float down){
    return (n>up)?up:(n<down)?down:n;
}
void servo(int Angle){
    Angle += 90;
    //Angle = crop(Angle,0,180);
    pwm_set_gpio_level(PWM_SERVO, SERVORIGHT+((SERVOLEFT-SERVORIGHT)/180*Angle));
}
void kit(bool senso, int num){
    bool victiM = true;
    queue_add_blocking(&victimQueue, &victiM);
    for(int i=0;i<num;i++){
        servo(senso?-90:90);
        busy_wait_ms(1000);
        servo(0);
        busy_wait_ms(1000);
    }
    busy_wait_ms(5000-2000*num);
    victiM = false;
    queue_add_blocking(&victimQueue, &victiM);
}