#define UART_ID uart0
#define BAUD_RATE 1000000

#define UART_TX_PIN 0
#define UART_RX_PIN 1

void initSer(void);
void Serial_Send_Int(int num);
void Serial_Send_Hex(int num);
void Serial_Send_Bin(int num);
void Serial_Send_String(char *);
void SerialN(void);
void Serial_Send_Float(double val); 
void flush();
#define Serial_Send(_1) _Generic(_1,int: Serial_Send_Int,bool: Serial_Send_Int, char*: Serial_Send_String ,float: Serial_Send_Float,char: Serial_Send_String ,double: Serial_Send_Float,unsigned char: Serial_Send_Int,unsigned long: Serial_Send_Int,unsigned int: Serial_Send_Int)(_1)