#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "hardware/i2c.h"
#include "i2c-bus.h"
#include "pid-pwm.h"
#include "APDS.h"

uint16_t lights[4]={0,0,0,0};
uint8_t tile = 0;
uint16_t usuwhite=0;

    void APDS9960_Init(){

        // check device id
        uint8_t dev_id = i2c_get_register(APDS9960_I2C_ADDR,APDS9960_REG_ID);
		printf("apds id: 0x%x\n", dev_id);
        // APDS9960 disable all features
        APDS9960_SetMode(APDS9960_MODE_ALL, false);

        // APDS9960 set default values for ambient light and proximity registers
        i2c_set_register(APDS9960_I2C_ADDR,APDS9960_REG_ATIME, APDS9960_DEFAULT_ATIME);
        i2c_set_register(APDS9960_I2C_ADDR,APDS9960_REG_WTIME, APDS9960_DEFAULT_WTIME);
        i2c_set_register(APDS9960_I2C_ADDR,APDS9960_REG_PPULSE, APDS9960_DEFAULT_PROX_PPULSE);
        i2c_set_register(APDS9960_I2C_ADDR,APDS9960_REG_POFFSET_UR, 127);
        i2c_set_register(APDS9960_I2C_ADDR,APDS9960_REG_POFFSET_DL, APDS9960_DEFAULT_POFFSET_DL);
        i2c_set_register(APDS9960_I2C_ADDR,APDS9960_REG_CONFIG1, APDS9960_DEFAULT_CONFIG1);
        APDS9960_SetLEDDrive(APDS9960_LED_DRIVE_12_5MA); 
        i2c_set_register(APDS9960_I2C_ADDR,APDS9960_REG_PERS, APDS9960_DEFAULT_PERS);
        i2c_set_register(APDS9960_I2C_ADDR,APDS9960_REG_CONFIG2, APDS9960_DEFAULT_CONFIG2);
        i2c_set_register(APDS9960_I2C_ADDR,APDS9960_REG_CONFIG3, APDS9960_DEFAULT_CONFIG3);
		//APDS9960_SetProxPhotoMask(7);
        //APDS9960_SetProxGainCompEnable(true);
		APDS9960_EnableLightSensor();
        APDS9960_EnableProximitySensor();

				

	}

    uint8_t getMode(){
        return i2c_get_register(APDS9960_I2C_ADDR,APDS9960_REG_ENABLE);
		}

    void APDS9960_SetMode(uint8_t mode, bool Enable){
        // read APDS9960_Enable register
        uint8_t reg_val = getMode();

        if(mode > APDS9960_MODE_ALL){
//            raise ADPS9960InvalidMode(mode);
				}

        // change bit(s) in APDS9960_Enable register */
        if( mode == APDS9960_MODE_ALL){
            if( Enable){
                reg_val = 0x7f;
						}
            else{
                reg_val = 0x00;
						}
				}
        else{
            if( Enable){
                reg_val |= (1 << mode);
						}
            else{
                reg_val &= ~(1 << mode);
						}
				}
        // write value to APDS9960_Enable register
        i2c_set_register(APDS9960_I2C_ADDR,APDS9960_REG_ENABLE, reg_val);

		}
    // start the light (R/G/B/Ambient) sensor
    void APDS9960_EnableLightSensor(){
        APDS9960_EnablePower();
        APDS9960_SetMode(APDS9960_MODE_AMBIENT_LIGHT, true);
		}

    // stop the light sensor
    void APDS9960_DisableLightSensor(){
        APDS9960_SetMode(APDS9960_MODE_AMBIENT_LIGHT, false);
		}	
				
		void APDS9960_EnablePower(){
				APDS9960_SetMode(APDS9960_MODE_POWER, true);
		}


    void APDS9960_DisablePower(){
        // turn the APDS-9960 off
        APDS9960_SetMode(APDS9960_MODE_POWER, false);		
		}



    // *******************************************************************************
    // proximity sensor controls
    // *******************************************************************************
    

    void APDS9960_SetProxPhotoMask(uint8_t mask){
        /*Sets the mask for enabling/disabling proximity photodiodes.

            Bit    Photodiode
             3       UP
             2       DOWN
             1       LEFT
             0       RIGHT

            1 = disabled, 0 = enabled

            Args:
                mask (int): 4-bit mask value
        */
        uint8_t val = i2c_get_register(APDS9960_I2C_ADDR,APDS9960_REG_CONFIG3);

        // set bits in register to given value
        mask &= 0b00001111;
        val &= 0b11110000;
        val |= mask;

        i2c_set_register(APDS9960_I2C_ADDR, APDS9960_REG_CONFIG3, val);
    }
    void APDS9960_SetProxGainCompEnable(bool enable){
        /*Sets the proximity gain compensation enable.

            Args:
                enable (bool): True to enable compensation, False to disable
        */
        uint8_t val = i2c_get_register(APDS9960_I2C_ADDR,APDS9960_REG_CONFIG3);

        // set bits in register to given value
        val &= 0b11011111;
        if(enable) val |= 0b00100000;

        i2c_set_register(APDS9960_I2C_ADDR, APDS9960_REG_CONFIG3, val);
    }
    void APDS9960_SetLEDDrive(uint8_t drive){
        /*Sets LED drive strength for proximity and ALS.

            Value    LED Current
              0        100 mA
              1         50 mA
              2         25 mA
              3         12.5 mA

            Args:
                drive (int): value for the LED drive strength
        */
        uint8_t val = i2c_get_register(APDS9960_I2C_ADDR,APDS9960_REG_CONTROL);

        // set bits in register to given value
        drive &= 0b00000011;
        drive = drive << 6;
        val &= 0b00111111;
        val |= drive;

        i2c_set_register(APDS9960_I2C_ADDR,APDS9960_REG_CONTROL, val);
    }
    void APDS9960_EnableProximitySensor(){
        APDS9960_EnablePower();
        APDS9960_SetMode(APDS9960_MODE_PROXIMITY, true);
    }

    void APDS9960_DisableProximitySensor(){
        APDS9960_SetMode(APDS9960_MODE_PROXIMITY, false);
    }
    uint8_t getProximity(){
        return i2c_get_register(APDS9960_I2C_ADDR, APDS9960_REG_PDATA);
    }
    // *******************************************************************************
    // ambient light and color sensor controls
    // *******************************************************************************

    // check if( there is new light data available
    bool isLightAvailable(){
        uint8_t val = i2c_get_register(APDS9960_I2C_ADDR,APDS9960_REG_STATUS);

        // shift and mask out AVALID bit
        val &= APDS9960_BIT_AVALID;

        return (val == APDS9960_BIT_AVALID);
		}

    // reads the ambient (clear) light level as a 16-bit value
    uint16_t readAmbientLight(){
        // read value from clear channel, low byte register
        uint8_t l = i2c_get_register(APDS9960_I2C_ADDR,APDS9960_REG_CDATAL);

        // read value from clear channel, high byte register
        uint8_t h = i2c_get_register(APDS9960_I2C_ADDR,APDS9960_REG_CDATAH);

        return l | (h << 8);
	}

    // reads the red light level as a 16-bit value
    uint16_t readRedLight(){
        // read value from red channel, low byte register
        uint8_t l = i2c_get_register(APDS9960_I2C_ADDR,APDS9960_REG_RDATAL);

        // read value from red channel, high byte register
        uint8_t h = i2c_get_register(APDS9960_I2C_ADDR,APDS9960_REG_RDATAH);

        return l | (h << 8);
		}

    // reads the green light level as a 16-bit value
    uint16_t readGreenLight(){
        // read value from green channel, low byte register
        uint8_t l = i2c_get_register(APDS9960_I2C_ADDR,APDS9960_REG_GDATAL);

        // read value from green channel, high byte register
        uint8_t h = i2c_get_register(APDS9960_I2C_ADDR,APDS9960_REG_GDATAH);

        return l | (h << 8);
		}

    // reads the blue light level as a 16-bit value
    uint16_t readBlueLight(){
        // read value from blue channel, low byte register
        uint8_t l = i2c_get_register(APDS9960_I2C_ADDR,APDS9960_REG_BDATAL);

        // read value from blue channel, high byte register
        uint8_t h = i2c_get_register(APDS9960_I2C_ADDR,APDS9960_REG_BDATAH);

        return l | (h << 8);
		}

	uint16_t* readLights(){
		if (isLightAvailable()){
			lights[0] = readAmbientLight();
			lights[1] = readBlueLight();
			lights[2] = readBlueLight();
			lights[3] = readBlueLight();
            //printf("lights-> clear:%d red:%d green:%d blue:%d\n", lights[0],lights[1],lights[2],lights[3]);
            
			return lights;
		}
		return 0;
	}
	uint8_t APDS9960_GetTile(){									//return: white,silver,blue,black
		uint16_t whiteLight = readAmbientLight();
		uint16_t blueLight = readBlueLight();
        //printf("ambient light: %d, blue light:%d, whiteLight/blueLight:%f\n",whiteLight,blueLight,(float)whiteLight/blueLight);
		if (whiteLight>(usuwhite/3.5)){
            if(getProximity()>240) return 1;
            return 0;
        }
		if(((float)whiteLight/blueLight)<2.5 && whiteLight>(usuwhite/15)){
            //printf("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa");
             return 2;
        }
		return 3;
        /*if (whiteLight>750) return 0;
		else if(((float)whiteLight/blueLight)<2) return 1;
		return 2;*/
	}