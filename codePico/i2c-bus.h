extern bool comunicating;
void initI2c(void);
bool reserved_addr(uint8_t addr);
void finder(void);
uint8_t i2c_get_register(uint8_t add, uint8_t reg);
void i2c_set_register(uint8_t add, uint16_t reg_addr, uint16_t data);