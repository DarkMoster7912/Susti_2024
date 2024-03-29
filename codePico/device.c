#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "pico/binary_info.h"
#include "hardware/i2c.h"
#include "hardware/vreg.h"
#include "hardware/watchdog.h"
#include "pico/util/queue.h"
#include "pico/multicore.h"
#include "pico/stdlib.h"
#include "pico/stdio/driver.h"
#include "pico/stdio.h"
#include "pico/stdio_usb.h"

void initDevice(){
    vreg_set_voltage(VREG_VOLTAGE_1_30);
    set_sys_clock_khz(300*1000,true);
    gpio_init(25);
    gpio_set_dir(25,true);
    gpio_put(25,true);
    stdio_usb_init();
    sleep_ms(1000);
}

void resetDevice(){
    watchdog_enable(1,1);
    while(true)tight_loop_contents;
}