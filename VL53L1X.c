#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "hardware/i2c.h"
#include "i2c-bus.h"
#include "VL53L1X.h"
#include "VL6180X.h"


bool vl53l1x_Init(uint8_t N,uint8_t newAddress,uint8_t shdnPin){
  if(vl53l1x_init(N, newAddress, shdnPin)){
    vl53l1x_set_mode(N,2);
    vl53l1x_set_timing_budget(N,20);
    vl53l1x_set_inter_measurment(N,20);
    vl53l1x_set_register(N, SYSTEM__MODE_START, 0x40);
    vl53l1x_get_distance(N);
    vl53l1x_set_register(N, SYSTEM__INTERRUPT_CLEAR, 0x01);
    return 1;
  }
  return 0;
}
bool vl53l1x_init(uint8_t N,uint8_t newAddress,uint8_t shdnPin){
	address[N] = 0x29;

	gpio_put(shdnPin, 1);
    sleep_ms(1);
	//printf("0x%x\n",vl53l1x_get_register_16bit(N,VL53L1X_IDENTIFICATION__MODEL_ID));
	if (vl53l1x_get_register_16bit(N, VL53L1X_IDENTIFICATION__MODEL_ID) != VL53L1X_SENSOR_ID){
		printf ("ToF sensor %d reset failure.\n",N);
		ready[N] = false;
        return 0;
	}
    while(!vl53l1x_get_register(N, VL53L1X_FIRMWARE__SYSTEM_STATUS))sleep_ms(2);
    printf ("ToF sensor %d is ready.\n" ,N);
	ready[N] = true;
	// Required by datasheet
	for (uint8_t tmp_addr = 0x2D; tmp_addr <= 0x87; tmp_addr++){
        vl53l1x_set_register(N, tmp_addr, VL53L1X_DEFAULT_CONFIGURATION[tmp_addr - 0x2D]);
        //printf("addr:0x%x -> 0x%x\n",tmp_addr, vl53l1x_get_register(0, tmp_addr));
    }
    vl53l1x_set_register(N, SYSTEM__MODE_START, 0x40);
    while (!vl53l1x_data_ready(N));
    vl53l1x_set_register(N, SYSTEM__INTERRUPT_CLEAR, 0x01);
    vl53l1x_set_register(N, SYSTEM__MODE_START, 0x00);
    vl53l1x_set_register(N, VL53L1X_VHV_CONFIG__TIMEOUT_MACROP_LOOP_BOUND, 0x09);  // two bounds VHV
    vl53l1x_set_register(N, 0x0B, 0);  // start VHV from the previous temperature
	printf("%x\n",vl53l1x_change_address(N,newAddress));
    return 1;
}
//
//
uint16_t vl53l1x_get_timing_budget(uint8_t N){
    uint16_t Temp = vl53l1x_get_register_16bit(N,RANGE_CONFIG__TIMEOUT_MACROP_A_HI);
    printf("0x%x ",Temp);
    switch (Temp) {
    case 0x001D:
      return 15;
      break;
    case 0x0051:
    case 0x001E:
      return 20;
      break;
    case 0x00D6:
    case 0x0060:
      return 33;
      break;
    case 0x01AE:
    case 0x00AD:
      return 50;
      break;
    case 0x02E1:
    case 0x01CC:
      return 100;
      break;
    case 0x03E1:
    case 0x02D9:
      return 200;
      break;
    case 0x0591:
    case 0x048F:
      return 500;
      break;
    default:
      return 0;
  }

}
void vl53l1x_set_timing_budget(uint8_t N, uint16_t timingBudgetMs){
  uint16_t DM;
  uint32_t rangeA = 0, rangeB = 0;

  DM = vl53l1x_get_mode(N);
  printf("%d ",timingBudgetMs);
  
  // DM is 1 for short distance mode, 2 for long
  if (DM == 1) {
    switch (timingBudgetMs) {
      case 15:
        rangeA = 0x001D;
        rangeB = 0x0027;
        break;
      case 20:
        rangeA = 0x0051;
        rangeB = 0x006E;
        break;
      case 33:
        rangeA = 0x00D6;
        rangeB = 0x006E;
        break;
      case 50:
        rangeA = 0x01AE;
        rangeB = 0x01E8;
        break;
      case 100:
        rangeA = 0x02E1;
        rangeB = 0x0388;
        break;
      case 200:
        rangeA = 0x03E1;
        rangeB = 0x0496;
        break;
      case 500:
        rangeA = 0x0591;
        rangeB = 0x05C1;
        break;
    }
  } else {
    switch (timingBudgetMs) {
      case 20:
        rangeA = 0x001E;
        rangeB = 0x0022;
        break;
      case 33:
        rangeA = 0x0060;
        rangeB = 0x006E;
        break;
      case 50:
        rangeA = 0x00AD;
        rangeB = 0x00C6;
        break;
      case 100:
        rangeA = 0x01CC;
        rangeB = 0x01EA;
        break;
      case 200:
        rangeA = 0x02D9;
        rangeB = 0x02F8;
        break;
      case 500:
        rangeA = 0x048F;
        rangeB = 0x04A4;
        break;
    }
  }
  if(rangeA && rangeB){
    vl53l1x_set_register_16bit(N, RANGE_CONFIG__TIMEOUT_MACROP_A_HI, rangeA);
    vl53l1x_set_register_16bit(N, RANGE_CONFIG__TIMEOUT_MACROP_B_HI, rangeB);

    printf("0x%x, 0x%x, rangeA:0x%x, rangeB:0x%x",vl53l1x_get_register_16bit(N, RANGE_CONFIG__TIMEOUT_MACROP_A_HI),vl53l1x_get_register_16bit(N, RANGE_CONFIG__TIMEOUT_MACROP_B_HI), rangeA, rangeB);
  }
}
uint16_t vl53l1x_get_mode(uint8_t N){
    switch (vl53l1x_get_register(N,PHASECAL_CONFIG__TIMEOUT_MACROP))
    {
    case 0x14:
        return 1;
        break;
    case 0x0A:
        return 2;
        break;
    default:
        return 0;
        break;
    } 
}
void vl53l1x_set_mode(uint8_t N, uint8_t DM){
    uint16_t TB = vl53l1x_get_timing_budget(N);
    switch (DM) {
    case 1: // Short range
        vl53l1x_set_register(N, PHASECAL_CONFIG__TIMEOUT_MACROP, 0x14);
        vl53l1x_set_register(N, RANGE_CONFIG__VCSEL_PERIOD_A, 0x07);
        vl53l1x_set_register(N, RANGE_CONFIG__VCSEL_PERIOD_B, 0x05);
        vl53l1x_set_register(N, RANGE_CONFIG__VALID_PHASE_HIGH, 0x38);
        vl53l1x_set_register(N, SD_CONFIG__WOI_SD0, 0x0705);
        vl53l1x_set_register(N, SD_CONFIG__INITIAL_PHASE_SD0, 0x0606);
        break;
    case 2: // Long range
        vl53l1x_set_register(N, PHASECAL_CONFIG__TIMEOUT_MACROP, 0x0A);
        vl53l1x_set_register(N, RANGE_CONFIG__VCSEL_PERIOD_A, 0x0F);
        vl53l1x_set_register(N, RANGE_CONFIG__VCSEL_PERIOD_B, 0x0D);
        vl53l1x_set_register(N, RANGE_CONFIG__VALID_PHASE_HIGH, 0xB8);
        vl53l1x_set_register(N, SD_CONFIG__WOI_SD0, 0x0F0D);
        vl53l1x_set_register(N, SD_CONFIG__INITIAL_PHASE_SD0, 0x0E0E);
        break;
    }
    vl53l1x_set_timing_budget(N,TB);
    /*printf("0x%x,0x%x,0x%x,0x%x,0x%x,0x%x\n",
        vl53l1x_get_register(N, PHASECAL_CONFIG__TIMEOUT_MACROP),
        vl53l1x_get_register(N, RANGE_CONFIG__VCSEL_PERIOD_A),
        vl53l1x_get_register(N, RANGE_CONFIG__VCSEL_PERIOD_B),
        vl53l1x_get_register(N, RANGE_CONFIG__VALID_PHASE_HIGH),
        vl53l1x_get_register_16bit(N, SD_CONFIG__WOI_SD0),
        vl53l1x_get_register_16bit(N, SD_CONFIG__INITIAL_PHASE_SD0));*/
    
}

void vl53l1x_set_inter_measurment(uint8_t N, uint32_t IM) {
  printf("inter measurment before:%d\n",vl53l1x_get_inter_measurment(N));
  uint16_t clockPLL = vl53l1x_get_register_16bit(N,VL53L1X_RESULT__OSC_CALIBRATE_VAL);
  clockPLL = clockPLL&0x3FF;
  vl53l1x_set_register_32bit(N, VL53L1X_SYSTEM__INTERMEASUREMENT_PERIOD, (uint32_t)(clockPLL * IM * 1.075));
  printf("inter measurment after:%d\n",vl53l1x_get_inter_measurment(N));
}

uint16_t vl53l1x_get_inter_measurment(uint8_t N) {
  uint32_t tmp = vl53l1x_get_register_32bit(N, VL53L1X_SYSTEM__INTERMEASUREMENT_PERIOD);
  uint16_t clockPLL = vl53l1x_get_register_16bit(N,VL53L1X_RESULT__OSC_CALIBRATE_VAL);
  clockPLL = clockPLL & 0x3FF;
  return (uint16_t)(((uint16_t)tmp)/(clockPLL*1.065));
}
uint8_t vl53l1x_change_address(uint8_t N, uint8_t new_address){
	// NOTICE:  IT APPEARS THAT CHANGING THE ADDRESS IS NOT STORED IN NON-
	// VOLATILE MEMORY POWER CYCLING THE DEVICE REVERTS ADDRESS BACK TO 0X29

	if(address[N] == new_address)
		return address[N];
	if( new_address > 127)
		return address[N];

	vl53l1x_set_register(N, VL53L1X_I2C_SLAVE__DEVICE_ADDRESS, new_address);
	address[N] = new_address;	
    return vl53l1x_get_register(N, VL53L1X_I2C_SLAVE__DEVICE_ADDRESS);
}

int vl53l1x_get_distance(uint8_t N){
	if(ready[N]){
		// Start Single shot mode
		distances[N] = 0;	
		while(!vl53l1x_data_ready(N));
		distances[N] = vl53l1x_get_register_16bit(N, VL53L1X_RESULT__FINAL_CROSSTALK_CORRECTED_RANGE_MM_SD0);
    uint8_t rgSt = vl53l1x_get_register(N, VL53L1X_RESULT__RANGE_STATUS);
    rgSt = rgSt & 0x1F;
    if (rgSt < 24)printf("status:%d",status_rtn[rgSt]);

    
		vl53l1x_set_register(N, SYSTEM__INTERRUPT_CLEAR, 0x01);
		return distances[N];
	}
	else{
		return -1;
	}
}
uint8_t vl53l1x_get_status(uint8_t N){
  uint8_t rgSt = 9;//= vl53l1x_get_register(N, VL53L1X_RESULT__RANGE_STATUS);
  rgSt = rgSt & 0x1F;
  if (rgSt < 24) return status_rtn[rgSt];
  return 255;
}
int8_t vl53l1x_get_distance_nowait(uint8_t N){
	if(ready[N]){
	  if(vl53l1x_data_ready(N)){
      status[N]=vl53l1x_get_status(N);
      if(!status[N])distances[N] = vl53l1x_get_register_16bit(N, VL53L1X_RESULT__FINAL_CROSSTALK_CORRECTED_RANGE_MM_SD0) + 20;
      //printf("status:%d",vl53l1x_get_register(N, VL53L1X_RESULT__RANGE_STATUS));
      vl53l1x_set_register(N, SYSTEM__INTERRUPT_CLEAR, 0x01);
      return 1;
    }
    return 0;
	}
	else{
		return -1;
	}
}
bool vl53l1x_data_ready(uint8_t N){
    uint8_t tio = vl53l1x_get_register(N, GPIO__TIO_HV_STATUS);
    uint8_t hv = vl53l1x_get_register(N, GPIO_HV_MUX__CTRL);
    //printf("tio:%d, tio & 0x01:%d, hv:%d, !((hv&0x10)>>4):%d\n", tio,tio&0x01,hv,!((hv&0x10)>>4));
    return ((tio&0x01) == (!((hv&0x10)>>4)));
    //return (vl53l1x_get_register(N, GPIO__TIO_HV_STATUS)&0x01)==(!((vl53l1x_get_register(N, GPIO_HV_MUX__CTRL)&0x10)>>4));
}

uint32_t vl53l1x_get_register_32bit(uint8_t N, uint16_t reg) {
	uint8_t buffer[4];
	buffer[0] = reg >> 8;   // high byte of register address
	buffer[1] = reg & 0xFF; // low byte of register address
	i2c_write_blocking(i2c_default, address[N], buffer, 2,true);
	i2c_read_blocking(i2c_default, address[N], buffer, 4,false);
	return (buffer[0] << 24) | (buffer[1] << 16) | (buffer[2] << 8) | buffer[3];
}

uint16_t vl53l1x_get_register_16bit(uint8_t N, uint16_t reg) {
	uint8_t buffer[2];
	buffer[0] = reg >> 8;   // high byte of register address
	buffer[1] = reg & 0xFF; // low byte of register address
	i2c_write_blocking(i2c_default, address[N], buffer, 2,true);
	i2c_read_blocking(i2c_default, address[N], buffer, 2,false);
	return (buffer[0] << 8) | buffer[1];
}

uint8_t vl53l1x_get_register(uint8_t N, uint16_t reg) {
	uint8_t data;
	uint8_t buffer[2];
	buffer[0] = (reg >> 8)& 0xFF;   // high byte of register address
	buffer[1] = reg & 0xFF; // low byte of register address
	i2c_write_blocking(i2c_default, address[N], buffer, 2,false);
	i2c_read_blocking(i2c_default, address[N], &data, 1,false);
	return data;
}
void vl53l1x_set_register(uint8_t N, uint16_t reg_addr, uint32_t data){
	uint8_t reg_data[4] = {(reg_addr >> 8), (reg_addr & 0xFF), ((data>0xFF)?((data >> 8) & 0xFF):(data & 0xFF)),(data & 0xFF)};
	uint8_t reg_len = (data>0xFF)?4:3;
	i2c_write_blocking(i2c_default, address[N], reg_data, reg_len, false);
}
void vl53l1x_set_register_16bit(uint8_t N, uint16_t reg_addr, uint16_t data){
  uint8_t reg_data[4] = {(reg_addr >> 8), (reg_addr & 0xFF),(data>>8)&0xFF,(data)&0xFF};
  i2c_write_blocking(i2c_default, address[N], reg_data, 4, false);
}
void vl53l1x_set_register_32bit(uint8_t N, uint16_t reg_addr, uint32_t data){
  uint8_t reg_data[6] = {(reg_addr >> 8), (reg_addr & 0xFF), (data>>24)&0xFF,(data>>16)&0xFF,(data>>8)&0xFF,(data)&0xFF};
  i2c_write_blocking(i2c_default, address[N], reg_data, 6, false);
}
