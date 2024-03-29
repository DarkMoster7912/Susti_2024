/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

// Sweep through all 7-bit I2C addresses, to see if any slaves are present on
// the I2C bus. Print out a table that looks like this:
//
// I2C Bus Scan
//   0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F
// 0
// 1       @
// 2
// 3             @
// 4
// 5
// 6
// 7
//
// E.g. if slave addresses 0x12 and 0x34 were acknowledged.

#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "hardware/i2c.h"
#include "i2c-bus.h"
#include "VL6180X.h"
#include "BNO.h"
#include "APDS.h"




bool comunicating = false;


void initI2c(void){
    i2c_init(i2c_default, 100 * 1000);
    gpio_set_function(PICO_DEFAULT_I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(PICO_DEFAULT_I2C_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(PICO_DEFAULT_I2C_SDA_PIN);
    gpio_pull_up(PICO_DEFAULT_I2C_SCL_PIN);
    // Make the I2C pins available to picotool
    bi_decl(bi_2pins_with_func(PICO_DEFAULT_I2C_SDA_PIN,PICO_DEFAULT_I2C_SCL_PIN, GPIO_FUNC_I2C));
    for(int i=0; i<7; i++){
        gpio_init(shdnPins[i]);
        gpio_set_dir(shdnPins[i],true);
        gpio_put(shdnPins[i],false);
    }    
    finder();
    APDS9960_Init();
    gpio_put(shdnPins[6],true);
    if(!BNO055_Init(OPERATION_MODE_NDOF))printf("gyro non va\n");
	else printf("va da dio\n");
    sleep_ms(100);
	BNO055_SetExternalCrystalUse(true);
	sleep_ms(100);
    vl6180x_init(0,(0x20),shdnPins[0],true);
    for(int i=1;i<5;i++)vl6180x_init(i,(0x20+i),shdnPins[i], false);
    vl6180x_init(5,(0x25),shdnPins[5],true);
    printf("tutto ok\n");
	
}
// I2C reserves some addresses for special purposes. We exclude these from the scan.
// These are any addresses of the form 000 0xxx or 111 1xxx
bool reserved_addr(uint8_t addr) {
    return (addr & 0x78) == 0 || (addr & 0x78) == 0x78;
}

void finder() {

    
    printf("\nI2C Bus Scan\n");
    printf("    0   1   2   3   4   5   6   7   8   9   A   B   C   D   E   F\n");

    for (int addr = 0; addr < (1 << 7); ++addr) {
        if (addr % 16 == 0) {
            printf("%02x ", addr);
        }

        // Perform a 1-byte dummy read from the probe address. If a slave
        // acknowledges this address, the function returns the number of bytes
        // transferred. If the address byte is ignored, the function returns
        // -1.

        // Skip over any reserved addresses.
        int ret;
        uint8_t rxdata;
        if (reserved_addr(addr))
            ret = PICO_ERROR_GENERIC;
        else
            ret = i2c_read_blocking(i2c_default, addr, &rxdata, 1, false);

        printf(ret < 0 ? "--" : "%x",addr);
        printf(addr % 16 == 15 ? "\n" : "  ");
    }
    printf("Done.\n");
}

uint8_t i2c_get_register(uint8_t add, uint8_t reg){
    uint8_t data;
    while(comunicating);
    comunicating = true;
	i2c_write_blocking(i2c_default, add, &reg, 1,true);
	i2c_read_blocking(i2c_default, add, &data, 1,false);
    comunicating = false;
	return data;
}
void i2c_set_register(uint8_t add, uint16_t reg_addr, uint16_t data){
	uint8_t reg_data[2] = {(reg_addr & 0xFF), (data & 0xFF)};
    while(comunicating);
    comunicating = true;
	i2c_write_blocking(i2c_default, add, reg_data, 2, false);
    comunicating = false;
}