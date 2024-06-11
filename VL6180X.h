

#define __VL6180X_IDENTIFICATION_MODEL_ID                0x0000
#define __VL6180X_IDENTIFICATION_MODEL_REV_MAJOR         0x0001
#define __VL6180X_IDENTIFICATION_MODEL_REV_MINOR         0x0002
#define __VL6180X_IDENTIFICATION_MODULE_REV_MAJOR        0x0003
#define __VL6180X_IDENTIFICATION_MODULE_REV_MINOR        0x0004
#define __VL6180X_IDENTIFICATION_DATE                    0x0006    // 16bit value
#define __VL6180X_IDENTIFICATION_TIME                    0x0008    // 16bit value

#define  __VL6180X_SYSTEM_MODE_GPIO0                     0x0010
#define __VL6180X_SYSTEM_MODE_GPIO1                      0x0011
#define __VL6180X_SYSTEM_HISTORY_CTRL                    0x0012
#define __VL6180X_SYSTEM_INTERRUPT_CONFIG_GPIO           0x0014
#define __VL6180X_SYSTEM_INTERRUPT_CLEAR                 0x0015
#define __VL6180X_SYSTEM_FRESH_OUT_OF_RESET              0x0016
#define __VL6180X_SYSTEM_GROUPED_PARAMETER_HOLD          0x0017

#define  __VL6180X_SYSRANGE_START                        0x0018
#define __VL6180X_SYSRANGE_THRESH_HIGH                   0x0019
#define __VL6180X_SYSRANGE_THRESH_LOW                    0x001A
#define __VL6180X_SYSRANGE_INTERMEASUREMENT_PERIOD       0x001B
#define __VL6180X_SYSRANGE_MAX_CONVERGENCE_TIME          0x001C
#define __VL6180X_SYSRANGE_CROSSTALK_COMPENSATION_RATE   0x001E
#define __VL6180X_SYSRANGE_CROSSTALK_VALID_HEIGHT        0x0021
#define __VL6180X_SYSRANGE_EARLY_CONVERGENCE_ESTIMATE    0x0022
#define __VL6180X_SYSRANGE_PART_TO_PART_RANGE_OFFSET     0x0024
#define __VL6180X_SYSRANGE_RANGE_IGNORE_VALID_HEIGHT     0x0025
#define __VL6180X_SYSRANGE_RANGE_IGNORE_THRESHOLD        0x0026
#define __VL6180X_SYSRANGE_MAX_AMBIENT_LEVEL_MULT        0x002C
#define __VL6180X_SYSRANGE_RANGE_CHECK_ENABLES           0x002D
#define __VL6180X_SYSRANGE_VHV_RECALIBRATE               0x002E
#define __VL6180X_SYSRANGE_VHV_REPEAT_RATE               0x0031

#define  __VL6180X_SYSALS_START                          0x0038
#define __VL6180X_SYSALS_THRESH_HIGH                     0x003A
#define __VL6180X_SYSALS_THRESH_LOW                      0x003C
#define __VL6180X_SYSALS_INTERMEASUREMENT_PERIOD         0x003E
#define __VL6180X_SYSALS_ANALOGUE_GAIN                   0x003F
#define __VL6180X_SYSALS_INTEGRATION_PERIOD              0x0040

#define  __VL6180X_RESULT_RANGE_STATUS                   0x004D
#define __VL6180X_RESULT_ALS_STATUS                      0x004E
#define __VL6180X_RESULT_INTERRUPT_STATUS_GPIO           0x004F
#define __VL6180X_RESULT_ALS_VAL                         0x0050
#define __VL6180X_RESULT_HISTORY_BUFFER                  0x0052
#define __VL6180X_RESULT_RANGE_VAL                       0x0062
#define __VL6180X_RESULT_RANGE_RAW                       0x0064
#define __VL6180X_RESULT_RANGE_RETURN_RATE               0x0066
#define __VL6180X_RESULT_RANGE_REFERENCE_RATE            0x0068
#define __VL6180X_RESULT_RANGE_RETURN_SIGNAL_COUNT       0x006C
#define __VL6180X_RESULT_RANGE_REFERENCE_SIGNAL_COUNT    0x0070
#define __VL6180X_RESULT_RANGE_RETURN_AMB_COUNT          0x0074
#define __VL6180X_RESULT_RANGE_REFERENCE_AMB_COUNT       0x0078
#define __VL6180X_RESULT_RANGE_RETURN_CONV_TIME          0x007C
#define __VL6180X_RESULT_RANGE_REFERENCE_CONV_TIME       0x0080

#define  __VL6180X_READOUT_AVERAGING_SAMPLE_PERIOD       0x010A
#define __VL6180X_FIRMWARE_BOOTUP                        0x0119
#define __VL6180X_FIRMWARE_RESULT_SCALER                 0x0120
#define __VL6180X_I2C_SLAVE_DEVICE_ADDRESS               0x0212
#define __VL6180X_INTERLEAVED_MODE_ENABLE                0x02A3



 
extern uint8_t address[6], 
    idModel[6],
    idModelRevMajor[6],
	idModelRevMinor[6],
	idModuleRevMajor[6],
	idModuleRevMinor[6],
	idDate[6],
	idTime[6];
extern int distances[6];
extern uint8_t status[6];
extern int diffdistance;
extern bool ready[6];
extern int shdnPins[8];
extern int ScalerValues[4];
void vl6180x_init(uint8_t N,uint8_t newAddress,uint8_t shdnPin, bool Long);
void get_identification(uint8_t N);
uint8_t vl6180x_change_address(uint8_t N, uint8_t new_address);
int vl6180x_get_distance(uint8_t N);
int8_t vl6180x_get_distance_nowait(uint8_t N);
int vl6180x_get_longdistance(uint8_t N);
int vl6180x_get_longdistance_nowait(uint8_t N);
void vl6180x_setScaling(uint8_t N, uint8_t new_scaling);
uint16_t vl6180x_get_register_16bit(uint8_t N, uint16_t reg);
uint8_t vl6180x_get_register(uint8_t N, uint16_t reg);
void vl6180x_set_register(uint8_t N, uint16_t reg_addr, uint16_t data);
void VL6180X_DefaultSettings(uint8_t N);
void VL6180X_LongRangeSettings(uint8_t N);
