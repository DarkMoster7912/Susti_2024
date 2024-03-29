#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "hardware/i2c.h"
#include "i2c-bus.h"
#include "VL6180X.h"

uint8_t address[6], 
    idModel[6],
    idModelRevMajor[6],
	idModelRevMinor[6],
	idModuleRevMajor[6],
	idModuleRevMinor[6],
	idDate[6],
	idTime[6];
int distances[]={-1,-1,-1,-1,-1,-1};
int	diffdistance = 0;

bool ready[6];
int shdnPins[] = {2,3,6,8,9,7,12};
int ScalerValues[] = {0,253,127,84};
void vl6180x_init(uint8_t N,uint8_t newAddress,uint8_t shdnPin, bool Long){
	address[N] = 0x29;
	// Module identification
	idModel[N] = 0x00;
	idModelRevMajor[N] = 0x00;
	idModelRevMinor[N] = 0x00;
	idModuleRevMajor[N] = 0x00;
	idModuleRevMinor[N] = 0x00;
	idDate[N] = 0x00;
	idTime[N] = 0x00;
	
	gpio_put(shdnPin, 1);
    sleep_ms(1);
	
	if (vl6180x_get_register(N, __VL6180X_SYSTEM_FRESH_OUT_OF_RESET) == 1){
		printf ("ToF sensor %d is ready.\n" ,N);
		ready[N] = true;
	}
	else{
		printf ("ToF sensor %d reset failure.\n",N);
		ready[N] = false;
	}
	// Required by datasheet
	// http://www.st.com/st-web-ui/static/active/en/resource/technical/document/application_note/DM00122600.pdf
	vl6180x_set_register(N, 0x0207, 0x01);
	vl6180x_set_register(N, 0x0208, 0x01);
	vl6180x_set_register(N, 0x0096, 0x00);
	vl6180x_set_register(N, 0x0097, 0xfd);
	vl6180x_set_register(N, 0x00e3, 0x00);
	vl6180x_set_register(N, 0x00e4, 0x04);
	vl6180x_set_register(N, 0x00e5, 0x02);
	vl6180x_set_register(N, 0x00e6, 0x01);
	vl6180x_set_register(N, 0x00e7, 0x03);
	vl6180x_set_register(N, 0x00f5, 0x02);
	vl6180x_set_register(N, 0x00d9, 0x05);
	vl6180x_set_register(N, 0x00db, 0xce);
	vl6180x_set_register(N, 0x00dc, 0x03);
	vl6180x_set_register(N, 0x00dd, 0xf8);
	vl6180x_set_register(N, 0x009f, 0x00);
	vl6180x_set_register(N, 0x00a3, 0x3c);
	vl6180x_set_register(N, 0x00b7, 0x00);
	vl6180x_set_register(N, 0x00bb, 0x3c);
	vl6180x_set_register(N, 0x00b2, 0x09);
	vl6180x_set_register(N, 0x00ca, 0x09);
	vl6180x_set_register(N, 0x0198, 0x01);
	vl6180x_set_register(N, 0x01b0, 0x17);
	vl6180x_set_register(N, 0x01ad, 0x00);
	vl6180x_set_register(N, 0x00ff, 0x05);
	vl6180x_set_register(N, 0x0100, 0x05);
	vl6180x_set_register(N, 0x0199, 0x05);
	vl6180x_set_register(N, 0x01a6, 0x1b);
	vl6180x_set_register(N, 0x01ac, 0x3e);
	vl6180x_set_register(N, 0x01a7, 0x1f);
	vl6180x_set_register(N, 0x0030, 0x00);
	printf("%x\n",vl6180x_change_address(N,newAddress));
	if(!Long)VL6180X_DefaultSettings(N);
	else{
		VL6180X_LongRangeSettings(N);
		vl6180x_setScaling(N,3);
	}
}
void VL6180X_LongRangeSettings(uint8_t N){
	// Recommended settings from datasheet;
        // http://www.st.com/st-web-ui/static/active/en/resource/technical/document/application_note/DM00122600.pdf;
        // Set GPIO1 high when sample complete;
        vl6180x_set_register(N,__VL6180X_SYSTEM_MODE_GPIO1, 0x10);
        // Set Avg sample period;
        vl6180x_set_register(N,__VL6180X_READOUT_AVERAGING_SAMPLE_PERIOD, 0x30);
        // Set the ALS gain;
        vl6180x_set_register(N,__VL6180X_SYSALS_ANALOGUE_GAIN, 0x46);
        // Set auto calibration period (Max = 255)/(OFF = 0);
        vl6180x_set_register(N,__VL6180X_SYSRANGE_VHV_REPEAT_RATE, 0xFF);
        // Set ALS integration time to 100ms;
        vl6180x_set_register(N,__VL6180X_SYSALS_INTEGRATION_PERIOD, 0x63);
        // perform a single temperature calibration;
        vl6180x_set_register(N,__VL6180X_SYSRANGE_VHV_RECALIBRATE, 0x01);
        // Optional settings from datasheet;
        // http://www.st.com/st-web-ui/static/active/en/resource/technical/document/application_note/DM00122600.pdf;
        // Set default ranging inter-measurement period to 100ms;
        vl6180x_set_register(N,__VL6180X_SYSRANGE_INTERMEASUREMENT_PERIOD, 0x09);
        // Set default ALS inter-measurement period to 100ms;
        vl6180x_set_register(N,__VL6180X_SYSALS_INTERMEASUREMENT_PERIOD, 0x31);
        // Configures interrupt on 'New Sample Ready threshold event';
        vl6180x_set_register(N,__VL6180X_SYSTEM_INTERRUPT_CONFIG_GPIO, 0x24);
        // Additional settings defaults from community;
        vl6180x_set_register(N,__VL6180X_SYSRANGE_MAX_CONVERGENCE_TIME, 0x32);
        vl6180x_set_register(N,__VL6180X_SYSRANGE_RANGE_CHECK_ENABLES, 0x10);
        vl6180x_set_register(N,__VL6180X_SYSRANGE_EARLY_CONVERGENCE_ESTIMATE, 0x57);
        vl6180x_set_register(N,__VL6180X_SYSALS_INTEGRATION_PERIOD, 0x64);
        vl6180x_set_register(N,__VL6180X_SYSALS_ANALOGUE_GAIN, 0x40);
        vl6180x_set_register(N,__VL6180X_FIRMWARE_RESULT_SCALER, 0x01);
}
void VL6180X_DefaultSettings(uint8_t N){
        // Recommended settings from datasheet;
        // http://www.st.com/st-web-ui/static/active/en/resource/technical/document/application_note/DM00122600.pdf;
        // Set GPIO1 high when sample complete;
        vl6180x_set_register(N,__VL6180X_SYSTEM_MODE_GPIO1, 0x10);
        // Set Avg sample period;
        vl6180x_set_register(N,__VL6180X_READOUT_AVERAGING_SAMPLE_PERIOD, 0x30);
        // Set the ALS gain;
        vl6180x_set_register(N,__VL6180X_SYSALS_ANALOGUE_GAIN, 0x46);
        // Set auto calibration period (Max = 255)/(OFF = 0);
        vl6180x_set_register(N,__VL6180X_SYSRANGE_VHV_REPEAT_RATE, 0xFF);
        // Set ALS integration time to 100ms;
        vl6180x_set_register(N,__VL6180X_SYSALS_INTEGRATION_PERIOD, 0x63);
        // perform a single temperature calibration;
        vl6180x_set_register(N,__VL6180X_SYSRANGE_VHV_RECALIBRATE, 0x01);
        // Optional settings from datasheet;
        // http://www.st.com/st-web-ui/static/active/en/resource/technical/document/application_note/DM00122600.pdf;
        // Set default ranging inter-measurement period to 100ms;
        vl6180x_set_register(N,__VL6180X_SYSRANGE_INTERMEASUREMENT_PERIOD, 0x09);
        // Set default ALS inter-measurement period to 100ms;
        vl6180x_set_register(N,__VL6180X_SYSALS_INTERMEASUREMENT_PERIOD, 0x31);
        // Configures interrupt on 'New Sample Ready threshold event';
        vl6180x_set_register(N,__VL6180X_SYSTEM_INTERRUPT_CONFIG_GPIO, 0x24);
        // Additional settings defaults from community;
        vl6180x_set_register(N,__VL6180X_SYSRANGE_MAX_CONVERGENCE_TIME, 0xA);
        vl6180x_set_register(N,__VL6180X_SYSRANGE_RANGE_CHECK_ENABLES, 0x10);
        vl6180x_set_register(N,__VL6180X_SYSRANGE_EARLY_CONVERGENCE_ESTIMATE, 0x7B);
        vl6180x_set_register(N,__VL6180X_SYSALS_INTEGRATION_PERIOD, 0x64);
        vl6180x_set_register(N,__VL6180X_SYSALS_ANALOGUE_GAIN, 0x40);
        vl6180x_set_register(N,__VL6180X_FIRMWARE_RESULT_SCALER, 0x01);

}
void get_identification(uint8_t N){

	idModel[N] = vl6180x_get_register(N,__VL6180X_IDENTIFICATION_MODEL_ID);
	idModelRevMajor[N] = vl6180x_get_register(N,__VL6180X_IDENTIFICATION_MODEL_REV_MAJOR);
	idModelRevMinor[N] = vl6180x_get_register(N,
		__VL6180X_IDENTIFICATION_MODEL_REV_MINOR);
	idModuleRevMajor[N] = vl6180x_get_register(N,
		__VL6180X_IDENTIFICATION_MODULE_REV_MAJOR);
	idModuleRevMinor[N] = vl6180x_get_register(N,
		__VL6180X_IDENTIFICATION_MODULE_REV_MINOR);

	idDate[N] = vl6180x_get_register_16bit(N, 
		__VL6180X_IDENTIFICATION_DATE);
	idTime[N] = vl6180x_get_register_16bit(N, 
		__VL6180X_IDENTIFICATION_TIME);
}
void vl6180x_setScaling(uint8_t N, uint8_t new_scaling)
{
	uint8_t const DefaultCrosstalkValidHeight = 20; // default value of SYSRANGE__CROSSTALK_VALID_HEIGHT

	// do nothing if scaling value is invalid
	if (new_scaling < 1 || new_scaling > 3) { return; }

	vl6180x_set_register(N, 0x0097, ScalerValues[new_scaling]);

	// apply scaling on part-to-part offset
	uint8_t ptp_offset = vl6180x_get_register(N, __VL6180X_SYSRANGE_PART_TO_PART_RANGE_OFFSET);
	vl6180x_set_register(N, __VL6180X_SYSRANGE_PART_TO_PART_RANGE_OFFSET, ptp_offset / new_scaling);

	// apply scaling on CrossTalkValidHeight
	vl6180x_set_register(N, __VL6180X_SYSRANGE_CROSSTALK_VALID_HEIGHT, DefaultCrosstalkValidHeight / new_scaling);

	// This function does not apply scaling to RANGE_IGNORE_VALID_HEIGHT.

	// enable early convergence estimate only at 1x scaling
	uint8_t rce = vl6180x_get_register(N,__VL6180X_SYSRANGE_RANGE_CHECK_ENABLES);
	vl6180x_set_register(N, __VL6180X_SYSRANGE_RANGE_CHECK_ENABLES, (rce & 0xFE) | (new_scaling == 1));
}

uint8_t vl6180x_change_address(uint8_t N, uint8_t new_address){
	// NOTICE:  IT APPEARS THAT CHANGING THE ADDRESS IS NOT STORED IN NON-
	// VOLATILE MEMORY POWER CYCLING THE DEVICE REVERTS ADDRESS BACK TO 0X29

	if(address[N] == new_address)
		return address[N];
	if( new_address > 127)
		return address[N];

	vl6180x_set_register(N, __VL6180X_I2C_SLAVE_DEVICE_ADDRESS, new_address);
	address[N] = new_address;
	return vl6180x_get_register(N, __VL6180X_I2C_SLAVE_DEVICE_ADDRESS);
}
int get_distance(uint8_t N){
	if(ready[N]){
		// Start Single shot mode
		vl6180x_set_register(N, __VL6180X_SYSRANGE_START, 0x01);
		distances[N] = 0;	
		while(!vl6180x_get_register(N, __VL6180X_RESULT_INTERRUPT_STATUS_GPIO));
		distances[N] = vl6180x_get_register(N, __VL6180X_RESULT_RANGE_VAL);
		vl6180x_set_register(N, __VL6180X_SYSTEM_INTERRUPT_CLEAR, 0x07);
		return distances[N];
	}
	else{
		printf("error \n");
		return -1;
	}
}
int get_longdistance(uint8_t N){
	if(ready[N]){
		// Start Single shot mode
		vl6180x_set_register(N, __VL6180X_SYSRANGE_START, 0x01);
		distances[N] = 0;	
		while(!vl6180x_get_register(N, __VL6180X_RESULT_INTERRUPT_STATUS_GPIO));
		distances[N] = 3*vl6180x_get_register(N, __VL6180X_RESULT_RANGE_VAL);
		vl6180x_set_register(N, __VL6180X_SYSTEM_INTERRUPT_CLEAR, 0x07);
		return distances[N];
	}
	else return -1;
}



uint16_t vl6180x_get_register_16bit(uint8_t N, uint16_t reg) {
	uint8_t buffer[2];
	buffer[0] = reg >> 8;   // high byte of register address
	buffer[1] = reg & 0xFF; // low byte of register address
	i2c_write_blocking(i2c_default, address[N], buffer, 2,true);
	i2c_read_blocking(i2c_default, address[N], buffer, 2,false);
	return (buffer[0] << 8) | buffer[1];
}

uint8_t vl6180x_get_register(uint8_t N, uint16_t reg) {
	uint8_t data;
	uint8_t buffer[2];
	buffer[0] = (reg >> 8)& 0xFF;   // high byte of register address
	buffer[1] = reg & 0xFF; // low byte of register address
	i2c_write_blocking(i2c_default, address[N], buffer, 2,true);
	i2c_read_blocking(i2c_default, address[N], &data, 1,false);
	return data;
}
void vl6180x_set_register(uint8_t N, uint16_t reg_addr, uint16_t data){
	uint8_t reg_data[4] = {(reg_addr >> 8), (reg_addr & 0xFF), ((data>0xFF)?((data >> 8) & 0xFF):(data & 0xFF)),(data & 0xFF)};
	uint8_t reg_len = (data>0xFF)?4:3;
	i2c_write_blocking(i2c_default, address[N], reg_data, reg_len, false);
}

