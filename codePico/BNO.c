
#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "hardware/i2c.h"
#include "i2c-bus.h"
#include "BNO.h"

uint8_t BNO055_Mode;
uint8_t count = 0;
float angles[3];
int accVects[3];
int facingDirection = 0;


bool BNO055_Init(uint8_t mode){
	//OPERATION_MODE_NDOF
	sleep_ms(100);
	// Make sure we have the right device
	if (i2c_get_register(BNO055_ADDRESS_A,BNO055_CHIP_ID_ADDR) != BNO055_ID){
		sleep_ms(1000);	// Wait for the device to boot up
		if (i2c_get_register(BNO055_ADDRESS_A,BNO055_CHIP_ID_ADDR) != BNO055_ID) return false;
	}
	// Switch to config mode
	BNO055_SetMode(OPERATION_MODE_CONFIG);
	// Trigger a reset and wait for the device to boot up again
	i2c_set_register(BNO055_ADDRESS_A,BNO055_SYS_TRIGGER_ADDR, 0x20);
	sleep_ms(100);
	while (i2c_get_register(BNO055_ADDRESS_A,BNO055_CHIP_ID_ADDR) != BNO055_ID)sleep_ms(10);
	sleep_ms(50);

	// Set to normal power mode
	i2c_set_register(BNO055_ADDRESS_A,BNO055_PWR_MODE_ADDR, POWER_MODE_NORMAL);
	sleep_ms(10);

	i2c_set_register(BNO055_ADDRESS_A,BNO055_PAGE_ID_ADDR,0);
	i2c_set_register(BNO055_ADDRESS_A,BNO055_SYS_TRIGGER_ADDR,0);
	sleep_ms(10);

	// Set the requested mode
	BNO055_SetMode(mode);
	sleep_ms(20);

	return true;
}
void BNO055_SetMode(uint8_t mode){
		BNO055_Mode=mode;
		i2c_set_register(BNO055_ADDRESS_A,BNO055_OPR_MODE_ADDR, mode);
		sleep_ms(30);
	}
void BNO055_SetExternalCrystalUse(bool useExternalCrystal){
		uint8_t prevMode = BNO055_Mode;
		BNO055_SetMode(OPERATION_MODE_CONFIG);
		sleep_ms(25);
		i2c_set_register(BNO055_ADDRESS_A,BNO055_PAGE_ID_ADDR,0);
		i2c_set_register(BNO055_ADDRESS_A,BNO055_SYS_TRIGGER_ADDR, useExternalCrystal? 0x80: 0);
		sleep_ms(10);
		BNO055_SetMode(prevMode);
		sleep_ms(20);
	}
float* BNO055_GetVector(uint8_t vectorType){
		int i;
		int16_t xyz[3];
		float realxyz[3];
		float scalingFactor;
		uint8_t buf[6];
		for(int i=0;i<6;i++)buf[i] = i2c_get_register(BNO055_ADDRESS_A, vectorType+i);
		for (i=0;i<3;i++) xyz[i] = buf[i*2] | (buf[i*2+1]<<8);
		switch(vectorType){
			case VECTOR_MAGNETOMETER:
				scalingFactor = 16.0;
				break;
			case VECTOR_GYROSCOPE:
				scalingFactor = 900.0;
				break;
			case VECTOR_EULER:
				scalingFactor = 16.0;
				break;
			case VECTOR_GRAVITY:
				scalingFactor = 100.0;
				break;
			default:
				scalingFactor = 1.0;
				break;
		}
		for(i=0;i<3;i++){
			angles[i]=xyz[i]/scalingFactor;
		//realxyz[2]-=(realxyz[2]>180)?4096:0;
		}
		if(vectorType==VECTOR_LINEARACCEL)for(i=0;i<3;i++)accVects[i]=xyz[i];
		return angles;
	}


double relative180(float degrees){
	double gradi_= degrees;
	while(gradi_<0)gradi_+=360;
	while(gradi_>=360) gradi_-=360;
	if(gradi_>=180) gradi_-=360;
	return gradi_;
}
