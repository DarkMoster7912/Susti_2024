

#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "math.h"
#include "hardware/gpio.h"
#include "hardware/timer.h"
#include "hardware/pwm.h"
#include "pid-pwm.h"
#include "isr.h"
#include "i2c-bus.h"
#include "VL6180X.h"
#include "APDS.h"
#include "BNO.h"
#include "serial.h"
#include "hardware/i2c.h"
#include "pico/util/queue.h"


volatile uint64_t timertime, oldtimertime=0;
double time,oldtime;
int8_t sensnum=0, go=0, Go=0, finecorsa=0;
int provDist, blue=0, space=0;
bool turning = false, center = false, black=false, victim=false, start=true;
double absRelAngle;

queue_t facingDirectionQueue,goQueue,victimQueue, startQueue;
int delay;

bool gyro(struct repeating_timer *t) {
    timertime = time_us_64();
    if(queue_try_remove(&goQueue, &go))printf("go: %d\n",go);
    if(queue_try_remove(&facingDirectionQueue, &facingDirection))printf("%d\n",facingDirection);
    queue_try_remove(&victimQueue, &victim);
    queue_try_remove(&startQueue, &start);
    if(victim || !start){
        gpio_put(14,true);
        gpio_put(15,true); 
        gpio_put(16,true);
        gpio_put(17,true);
        pwm_set_gpio_level(PWM_SX, 0);
        pwm_set_gpio_level(PWM_DX, 0);
        //printf("ambient light: %d, blue light:%d, whiteLight/blueLight:%f\n",readAmbientLight(),readBlueLight(),(float)readAmbientLight()/readBlueLight());
    }
    else{
        Go=1;
        if(black){
            
            if(tile==0){
                if((distances[0]>180 && distances[0]<400) || (distances[0]>540 && distances[0]!=765)|| (distances[5]>0 && distances[5]<250 && distances[5]!=765)||(distances[0]==765 && distances[5]>300)){
                    Go = 0;
                    black=false;
                }
            }
                else Go = -1;
            
        }
        else if(tile==2){
            black=true;
        }
        BNO055_GetVector(VECTOR_LINEARACCEL);
        BNO055_GetVector(VECTOR_EULER);
        //printf("%d,%d,%d,%f,%d\n",accVects[0],accVects[1],accVects[2],angles[0],(unsigned long)((timertime-oldtime)/1000));
        absRelAngle = ABS(relative180(facingDirection-angles[0]));
        turning=(absRelAngle>10)?true:false;

        
        if(!turning){
            tile = APDS9960_GetTile();
            switch (tile)
            {
                case 0:
                    blue=0;
                    break;
                case 1:
                    if(blue!=-1){
                        blue++;
                        if(blue>50){
                            blue = -1;
                        }                    
                        if(blue>5){
                            Go = 0; 
                        }
                    }
                    break;

                case 2:
                    Go=-1;
                    break;
                
                default:
                    break;
                }
            printf("sens 0: %dmm\n",get_longdistance(0));
            for(int i=1;i<5;i++)printf("sens %d: %dmm\n",i,get_distance(i));
            printf("sens 5: %dmm\n",get_longdistance(5));
            if(distances[1]<170 && distances[2]<170 && distances[3]<170 && distances[4]<170 && ready[1] && ready[2] && ready[3] && ready[4])
                provDist = (-distances[3]+distances[1]-distances[4]+distances[2])/2;
            else if(distances[1]<170 && ready[1])provDist = (-75+distances[1]);
            else if(distances[2]<170 && ready[2])provDist = (-75+distances[2]);
            else if(distances[3]<170 && ready[3])provDist = (-distances[3]+75);
            else if(distances[4]<170 && ready[4])provDist = (-distances[4]+75);
            else provDist = 0;

        }
        if(absRelAngle>45)oldtime = timertime;
        diffdistance = turning?0:provDist;
        
        Go = (distances[0]<60)?0:Go;
        if(finecorsa){
            Go=-1;
            finecorsa--;
        }

        setpwm(Go, calcpid());
    }
    return true;
}
bool laser(struct repeating_timer *t){
    time=(double)time_us_64()/1000000L;
    BNO055_GetVector(VECTOR_LINEARACCEL)[1];
    return true;
}
bool apds(struct repeating_timer *t){
	uint16_t* lghts = readLights();
    if(lghts!=0){
        for(int i;i<4;i++)lights[i] = lghts[i];
        printf("lights-> gen:%d red:%d green:%d blue%d\n", lights[0],lights[1],lights[2],lights[3]);
    }
    return true;
}


void initIsr(void){
    sleep_ms(10);
    queue_init(&facingDirectionQueue,2,1);
    queue_init(&goQueue,1,1);
    queue_init(&victimQueue,1,1);
    queue_init(&startQueue,1,1);
    for(int i=26;i<28;i++){
        gpio_init(i);
        gpio_set_dir(i, GPIO_IN);
        gpio_pull_up(i);
        gpio_set_irq_enabled_with_callback(i, GPIO_IRQ_EDGE_FALL, true, &finecorsa_callback);
    }
    //gpio_set_irq_enabled_with_callback(28, GPIO_IRQ_EDGE_FALL, true, &encoder_callback);
    static repeating_timer_t timer;
    // Create the repeating timer
    add_repeating_timer_us(100, gyro, NULL, &timer);
    //add_repeating_timer_us(100, laser, NULL, &timer);
    //add_repeating_timer_ms(100, messageForRasp, NULL, &timer);
    printf("fine\n");

}
// Interrupt handler function for the timer




void finecorsa_callback(uint gpio, uint32_t events){
    finecorsa = 10;
}

void encoder_callback(uint gpio, uint32_t events){
    timertime = time_us_64() ;
    if(!finecorsa)printf("space:%d, time:%d\n",space++, timertime-oldtimertime);
    oldtimertime = timertime;
    
}




void gpio_event_string(char *buf, uint32_t events) {
    for (uint i = 0; i < 4; i++) {
        uint mask = (1 << i);
        if (events & mask) {
            // Copy this event string into the user string
            const char *event_str = gpio_irq_str[i];
            while (*event_str != '\0') {
                *buf++ = *event_str++;
            }
            events &= ~mask;

            // If more events add ", "
            if (events) {
                *buf++ = ',';
                *buf++ = ' ';
            }
        }
    }
    *buf++ = '\0';
}

bool messageForRasp(){
    Serial_Send_Int((distances[1]>170 && distances[2]>170)?0:1);
    Serial_Send_Int((distances[3]>170 && distances[4]>170)?0:1);
    Serial_Send_Int(distances[0]>170?0:1);
    //Serial_Send_Int(distances[5]>450?2:distances[5]>150?1:0);
    Serial_Send_Int(tile);
    Serial_Send_Int((distances[0]>150 && distances[0]<400) || (distances[0]>570 && distances[0]!=765) || (distances[5]>0 && distances[5]<250 && distances[5]!=765)||(distances[0]==765 && distances[5]>300));
    Serial_Send_Int(go);
    Serial_Send_Int((facingDirection/90)%4); 
    //Serial_Send_String("d");                    //message[message.index('d'):]=angle -> round(angle/90)=actualFacingDirection
    //Serial_Send_Float(angles[0]); 
    //Serial_Send_String("e");
    return true;
}