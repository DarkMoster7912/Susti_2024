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
#include "BNO.h"
#include "APDS.h"
#include "serial.h"


void main1(){
    initSer();
    sleep_ms(5000);
    while (true){
        messageForRasp();
        sleep_ms(100);
    };
}
int main() {
    initDevice();
    multicore_launch_core1(main1);
    sleep_ms(500);
    initPwm();
    initI2c();
    initIsr();
    Serial_Send_String("xd\n");
    printf("xd\n");
    sleep_ms(10);
    while (true){
        //printf("ambient light: %d, red light:%d, green light:%d, blue light:%d\n",readAmbientLight(),readRedLight(),readGreenLight(),readBlueLight());
        //printf("LASER avanti: %dmm    ", distances[0]);
        //printf("LASER diestr: %dmm \n", distances[5]);
        //while(!(distances[0]%300>10 && distances[0]%300<120)||(distances[5]%300>10 && distances[5]%300<120)||(distances[0]==765 && distances[5]==765))sleep_ms(10);
        while(blue>0);
        if(black){
            sleep_ms(10);
            while(black);
            facingDirection-=(distances[3]>170 && distances[4]>170)?90:(distances[5]>100)?180:-90;
            sleep_ms(10);
            while(turning);
            while(distances[0]>70 && !black && ((distances[0]>150 && distances[0]<400) || (distances[0]>570 && distances[0]!=765) || (distances[5]>0 && distances[5]<250 && distances[5]!=765)||(distances[0]==765 && distances[5]>300)))sleep_ms(10);
        }
        else if((distances[1]>170 && distances[2]>170)){
            facingDirection+=90;
            sleep_ms(10);
            while(turning);
            while(distances[0]>70 && !black && ((distances[0]>150 && distances[0]<400) || (distances[0]>570 && distances[0]!=765) || (distances[5]>0 && distances[5]<250 && distances[5]!=765)||(distances[0]==765 && distances[5]>300)))sleep_ms(10);
        }
        else if((distances[0]<70)){
            facingDirection-=(distances[3]>170 && distances[4]>170)?90:180;
            sleep_ms(10);
            while(turning);
            while(distances[0]>70 && !black && ((distances[0]>150 && distances[0]<400) || (distances[0]>570 && distances[0]!=765) || (distances[5]>0 && distances[5]<250 && distances[5]!=765)||(distances[0]==765 && distances[5]>300)))sleep_ms(10);
        }

        sleep_ms(50);
        /*messageForRasp();
        sleep_ms(100);*/
        /*servo(0);
        sleep_ms(1000);
        servo(90);
        sleep_ms(1000);
        servo(0);
        sleep_ms(1000);
        servo(-90);
        sleep_ms(1000);*/
        //printf("facingDirection: %d     ",facingDirection);
        //printf("distance: 0:%dmm, 1:%dmm, 2:%dmm, 3:%dmm, 4:%dmm, 5:%dmm\n",distances[0],distances[1],distances[2],distances[3],distances[4],distances[5]);
        /*if(!turning){
            if (distances[0]<80)
            {
                if(distances[1]>170 && distances[2]>170)facingDirection +=90;
                else facingDirection-=90;
            }
            facingDirection+=(facingDirection>360)?-360:(facingDirection<0)?360:0;
        }*/
        //setpwm(1,0);
        //printf("distance: 0:%dmm, 1:%dmm, 2:%dmm, 3:%dmm, 4:%dmm,\n",get_distance(0),get_distance(1),get_distance(2),get_distance(3),get_distance(4));
        //BNO055_GetVector(VECTOR_EULER);
    //Serial_Send("xd\n");
    //facingDirection+=(facingDirection==90)?-90:90;
    //if(queue_try_remove(&led, &ledd))printf("led %s\n", (ledd)?"on":"off");
    //else printf("queue non vanno\n");
    //sleep_ms(1000);
//    tight_loop_contents();
    
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
