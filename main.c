/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

// Output PWM signals on pins 0 and 1
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "pico/binary_info.h"
#include "hardware/i2c.h"
#include "hardware/vreg.h"
#include "pico/util/queue.h"
#include "pico/multicore.h"
#include "pico/stdlib.h"
#include "pico/stdio/driver.h"
#include "pico/stdio.h"
#include "pico/stdio_usb.h"
#include "pico/stdio_uart.h"
#include "device.h"
#include "pid-pwm.h"
#include "isr.h"
#include "i2c-bus.h"
#include "VL6180X.h"
#include "VL53L1X.h"
#include "BNO.h"
#include "APDS.h"
#include "serial.h"


void main1(){
    initSer();
    Serial_Send_String("d\n");
    while(!start);
    sleep_ms(10);
    while(!go){messageForRasp();sleep_ms(100);}
    while (true){
        if(!justEntered)messageForRaspSV();
        sleep_ms(10);
    }
}

int main(){
    initDevice();
    multicore_launch_core1(main1);
    sleep_ms(1000);
    //sleep_ms(2500);
    initPwm();
    initI2c();
    initIsr();
    
    printf("xd\n");
    //APDS9960_setValues();
    //Serial_Send_String("dx\n");

    sleep_ms(10);
    //for(int i=0;i<5;i++){
    //    kit(true,1);
    //    kit(false,1);
    //}
    //uint32_t i=0;
    //while(!start)if(i++>200000L){start=true;go=true;}
    while (true){
        printf("go: %d\n",30);
        timertime = time_us_32();
        if(queue_try_remove(&goQueue, &go))printf("go: %d\n",go);
        if(queue_try_remove(&facingDirectionQueue, &facingDirection))printf("facingDirection: %d\n",facingDirection);
        queue_try_remove(&victimQueue, &victim);
        if(queue_try_remove(&startQueue, &start))printf("start: %d\n",start);
        if(!queue_try_remove(&pathQueue,&path[indeX++]))indeX=0;
        if(queue_try_peek(&impQueue, &imp));//printf("imp: %d\n",imp);
        if(!start){
            uint16_t amb = readAmbientLight();
            if(usuwhite==(usuwhite+amb)/2){Serial_Send_String("x\n");start=true;go=1;}
            usuwhite=(usuwhite+amb)/2;
            printf("white=%d\n",usuwhite);
            busy_wait_ms(100);
            //printf("avanti:%dmm,dietro:%dmm ",get_longdistance(0),get_longdistance(5));
            
            //0 caselle: davanti->117:12 dietro->0:102
            //1 casella: davanti->450:330 dietro->306:420
            
        }
        else{

            Go=go;
            //Go = (distances[0]<60)?0:Go;
            if(finecorsa){
                justFinecorsaed=true;
                Go=-1;
                facingDirection+=(finecorsa>0)?-1:1;
                finecorsa+=(finecorsa>0)?-1:1;
                //printf("facingDirecrtion:%d\n",facingDirection);
            }
            //else Go=1;
            if(black){
                if((tile!=3)&&(inTile||justEntered)){
                        Go = 0;
                        black=false;
                        facingDirection-=(distances[3]>170 && distances[4]>170)?90:(distances[5]>100)?180:-90;
                }
                else Go = -1;
                
            }
            printf("go: %d\n",31);
            //BNO055_GetVector(VECTOR_LINEARACCEL);
            BNO055_GetVector(VECTOR_EULER);
            //printf("%d,%d,%d,%f,%d\n",accVects[0],accVects[1],accVects[2],angles[0],(unsigned long)((timertime-oldtime)/1000));
            absRelAngle = ABS(relative180(facingDirection-angles[0]));
            justTurned=turning && !(absRelAngle>20);
            turning=(absRelAngle>20);
            //while(!queue_try_add(&turningQueue,&turning))queue_try_remove(&turningQueue,&tile);
            tile = 0;//APDS9960_GetTile();
            printf("go: %d\n",32);

            blue=(tile==2);
            if(tile!=3){
                if(justTurned){
                    vl6180x_get_longdistance(0); 
                    vl6180x_get_longdistance(5); 
                }
                else{
                    vl6180x_get_longdistance_nowait(0);
                    vl6180x_get_longdistance_nowait(5);
                }
            }
            else{
                Go=-1;
                black=true;
            }
            printf("go: %d\n",33);
            if(!victim && !finecorsa && !black){
                if(justTurned)for(int i=1;i<5;i++)vl6180x_get_distance(i); 
                else for(int i=1;i<5;i++)vl6180x_get_distance_nowait(i);
                if(distances[1]<170 && distances[2]<170 && distances[3]<170 && distances[4]<170 && ready[1] && ready[2] && ready[3] && ready[4])
                    provDist = (-distances[3]+distances[1]-distances[4]+distances[2])/2-5;
                else if(distances[1]<170 && ready[1])provDist = (-70+distances[1]);
                else if(distances[2]<170 && ready[2])provDist = (-70+distances[2]);
                else if(distances[3]<170 && ready[3])provDist = (-distances[3]+60);
                else if(distances[4]<170 && ready[4])provDist = (-distances[4]+60);
                else provDist = 0;
                diffdistance = absRelAngle>30?0:provDist;
            }
            else diffdistance = 0;
            
            //Go = (victim)?0:Go;
            //inTile=(((distances[0]>9 && distances[0]<117)||(distances[0]>330 && distances[0]<450)||(distances[5]>0 && distances[5]<102)||(distances[5]>306 && distances[5]<420)||(absRelAngle>30)))||(space==tileImpulse);
            printf("go: %d\n",34);
            inTile=((distances[0]<distances[5] || ABS(distances[0]-distances[5])<30)?((distances[0]>60 && distances[0]<75)||(distances[0]>375 && distances[0]<405)):((distances[5]>60 && distances[5]<75)||(distances[5]>345 && distances[5]<375)));
            if(justTurned){
                reset_imp();
                oldtimertime=timertime;
                stop=true;
            }
            
            /*if(inTile && imp>(TILE_LENGTH_IMP/2)){
                justEntered=true;
                reset_imp();
                stop = true;
                oldtimertime=timertime;
                printf("cose bellissime\n");
            }

            else if((distances[0]>525 && distances[5]>525) && (imp>TILE_LENGTH_IMP)){
                justEntered=true;
                reset_imp();
                stop = true;
                oldtimertime=timertime;
                printf("cose bellissimissime\n");

            }*/
            printf("go: %d\n",35);
            if(inTile && (timertime-oldtimertime)>(((blue && !justTurned)?5000000L:100000L)+1000000L)){
                reset_imp();
                stop = true;
                oldtimertime=timertime;
            }
            if(stop)if((timertime-oldtimertime)>((blue && !justTurned)?5000000L:100000L)){
                stop=false;
                justEntered=true;
            }
            gpio_put(25,inTile);  
            //printf("inTile:%d, justentered:%d, disav:%dmm, disdi:%d, turning:%d. stop:%d, time:%d\n",inTile,justEntered,distances[0],distances[5], turning,stop, (int)(timertime-oldtimertime));
            if((absRelAngle>15 || stop)&&Go==1)Go=0;
            //printf("facingDirection:%d, diffdist:%d\n",facingDirection, diffdistance);
            int8_t goo;
            while(!queue_try_add(&GoQueue,&Go))queue_try_remove(&GoQueue,&goo);
            
        

            if(justEntered && !stop){
                if(!justFinecorsaed)messageForRasp();
                else justFinecorsaed=false;
                justEntered=false;
                if (path[0]!='e'){
                    facingDirection = path[0]*90;
                    int* p=path;
                    p++;  
                    for(int i=0;i<29;i++)path[i]=p[i];
                    printf("out\n");
                }
                else if((distances[1]>170 && distances[2]>170)){    
                    facingDirection+=90;
                }
                else if(distances[0]<170){
                    facingDirection-=90;
                }
            }
            if(justTurned && distances[0]<170)facingDirection-=90;
            setpwm(Go, calcpid());
            printf("distance: 0:%dmm, 1:%dmm, 2:%dmm, 3:%dmm, 4:%dmm, 5:%dmm\n",distances[0],distances[1],distances[2],distances[3],distances[4],distances[5]);
            //setpwm(1,0);
            //printf("distance: 0:%dmm, 1:%dmm, 2:%dmm, 3:%dmm, 4:%dmm,\n",vl6180x_get_distance(0),vl6180x_get_distance(1),vl6180x_get_distance(2),vl6180x_get_distance(3),vl6180x_get_distance(4));
            //BNO055_GetVector(VECTOR_EULER);
            //Serial_Send("xd\n");
            //facingDirection+=(facingDirection==90)?-90:90;
            //if(queue_try_remove(&led, &ledd))printf("led %s\n", (ledd)?"on":"off");
            //else printf("queue non vanno\n");
            //sleep_ms(1000);
            tight_loop_contents();
        
            } 
    }
}
/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause


#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/util/queue.h"
#include "pico/multicore.h" 

#define FLAG_VALUE 123

typedef struct
{
    void *func;
    int32_t data;
} queue_entry_t;

queue_t call_queue;
queue_t results_queue;

void core1_entry() {
    while (1) {
        // Function pointer is passed to us via the queue_entry_t which also
        // contains the function parameter.
        // We provide an int32_t return value by simply pushing it back on the
        // return queue which also indicates the result is ready.

        queue_entry_t entry;

        
        while(true){
            queue_remove_blocking(&call_queue, &entry);

            int32_t (*func)() = (int32_t(*)())(entry.func);
            int32_t result = (*func)(entry.data);

            queue_add_blocking(&results_queue, &result);
        }

    }
}

int32_t factorial(int32_t n) {
    int32_t f = 1;
    for (int i = 2; i <= n; i++) {
        f *= i;
    }
    return f;
}

int32_t fibonacci(int32_t n) {
    if (n == 0) return 0;
    if (n == 1) return 1;

    int n1 = 0, n2 = 1, n3 = 0;

    for (int i = 2; i <= n; i++) {
        n3 = n1 + n2;
        n1 = n2;
        n2 = n3;
    }
    return n3;
}

#define TEST_NUM 10

int main() {
    int32_t res;
    vreg_set_voltage(VREG_VOLTAGE_1_30);
    set_sys_clock_khz(300*1000,true);
    stdio_init_all();
    sleep_ms(1000);
    printf("Hello, multicore_runner using queues!\n");

    // This example dispatches arbitrary functions to run on the second core
    // To do this we run a dispatcher on the second core that accepts a function
    // pointer and runs it. The data is passed over using the queue library from
    // pico_utils

    queue_init(&call_queue, sizeof(queue_entry_t), 2);
    queue_init(&results_queue, sizeof(int32_t), 2);

    multicore_launch_core1(core1_entry);

    queue_entry_t entry = {&factorial, TEST_NUM};
    queue_add_blocking(&call_queue, &entry);

    // We could now do a load of stuff on core 0 and get our result later

    queue_remove_blocking(&results_queue, &res);

    printf("Factorial %d is %d\n", TEST_NUM, res);

    // Now try a different function
    queue_entry_t entry1 = {&fibonacci, TEST_NUM};
    //entry.func = &fibonacci;
    queue_add_blocking(&call_queue, &entry1);
    queue_remove_blocking(&results_queue, &res);

    printf("Fibonacci %d is %d\n", TEST_NUM, res);
    while(true)tight_loop_contents();
}
*/