/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */


#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/uart.h"
#include <math.h>
#include <string.h>
#include "serial.h"
#include "pid-pwm.h"
#include "ISR.h"
#include "BNO.h"
#include "VL6180X.h"
#include "device.h"
																																																									

int chars_rxed = 0;
uint impp = 0;
int goo = 0;


void on_uart_rx() {
	chars_rxed=0;
	char str[30];
    while (uart_is_readable(UART_ID)) {
		str[chars_rxed++] = uart_getc(UART_ID);
	}
	switch (str[0])
	{
	case 's':
		bool starT = true;
		queue_add_blocking(&startQueue, &starT);
		break;
	case 'r':
		resetDevice();
		break;
	case 'd':
		uint16_t facdir = (90*(str[1]-'0'));
		queue_add_blocking(&facingDirectionQueue, &facdir);
		break;
	case 'g':
		char num[] = {str[1],str[2]};
		int8_t gO = atoi(num);
		queue_add_blocking(&goQueue, &gO);
		break;
	case 'v':
		bool victiM = true;
    	queue_add_blocking(&victimQueue, &victiM);
		kit(str[1]=='s',str[2]-'0');
		victiM = false;
    	queue_add_blocking(&victimQueue, &victiM);
		/*uint16_t fadir = (90*(str[1]-'0'));
		bool turning = true;
		bool victiM = true;
		int8_t GO = 0;
    	queue_add_blocking(&victimQueue, &victiM);
		queue_add_blocking(&goQueue, &GO);
		queue_add_blocking(&facingDirectionQueue, &fadir);
		busy_wait_ms(10);
		/*while(turning){
			queue_try_remove(&turningQueue,&turning);
			printf("turningQueue turning: %d\n",turning);
		}*/
		//GO = 0;
		//queue_add_blocking(&goQueue, &GO);
		
		break;
	case 'k':
		kit(str[1]=='s',str[2]-'0');
		victiM = false;
    	queue_add_blocking(&victimQueue, &victiM);
		int GO = 1;
		queue_add_blocking(&goQueue, &GO);
		break;
	case 'p':
		int i=0;
		while(str[i]!='e')queue_add_blocking(&pathQueue,&str[i++]);
	case 'i':
		queue_try_remove(&impQueue,&impp);																																																																																																												
		queue_try_remove(&GoQueue, &goo);
		impp+=goo;
		queue_add_blocking(&impQueue, &impp);
		break;
	default:
		printf("str:%s, str[0]:%c, str[1]:%c/%d, str[2]:%d\n",str,str[0],str[1],str[1],str[2]);
		break;
	}
}

void initSer() {
	/*gpio_init(18);
    gpio_set_dir(18,true);
    gpio_put(18,true);
	queue_init(&led,1,1);*/
    // Set up our UART with the required speed.
    uart_init(UART_ID, BAUD_RATE);

    // Set the TX and RX pins by using the function select on the GPIO
    // Set datasheet for more information on function select
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);

	int UART_IRQ = UART_ID == uart0 ? UART0_IRQ : UART1_IRQ;

    // And set up and enable the interrupt handlers
    irq_set_exclusive_handler(UART_IRQ, on_uart_rx);
    irq_set_enabled(UART_IRQ, true);

    // Now enable the UART to send interrupts - RX only
    uart_set_irq_enables(UART_ID, true, false);

}


void Serial_Send_Int(int num)
{
	if(num<0)
	{
		uart_puts(UART_ID,"-");
		num=-num;
	}
	if(num==0){uart_putc_raw(UART_ID,'0');}
	else
	{
		char str[32];				// definisce una stringa sulla quale convertire il numero da trasmettere (max 10 cifre)
		char i;						// contatore ciclo
		for(i=0;i<32;i++) str[i]=0; // cancella la stringa
		i=31;
		while (num)
		{
			str[i]=num%10+'0';		// converte il numero da trasmettere in una stringa (dalla cifra meno significativa)
			num/=10;
			i--;
		}
		for (i=0;i<32;i++)			// invia la stringa un carattere alla volta
		if (str[i]) uart_putc_raw(UART_ID,str[i]);
	}
}

void SerialN()
{
	uart_putc_raw(UART_ID,13);
	uart_putc_raw(UART_ID,10);
}
void Serial_Send_Float(double data)
{
	if(data<0)
	{
		uart_puts(UART_ID,"-");
		data=-data;
	}
	long var=data;
	Serial_Send_Int(var);
	int k=0,j=0;
	double Num=data;
	
	
	while (fmod(Num,1.00)!=0)
	{
		Num*=10;
		k++;
	}
	double data1=Num;

		
	var=var*pow(10,k);

	data1=data1-var;
	
	long NuM=data1;
	while (data1>1){
		data1/=10.00;
		j++;
		
	}
	int diff=k-j;
	uart_putc_raw(UART_ID,'.');
	
	for(int i=0;i<diff;i++)uart_putc_raw(UART_ID,'0');
	Serial_Send_Int(NuM);
}

void Serial_Send_Hex(int num){
	if(num<0)
	{
		uart_puts(UART_ID,"-");
		num=-num;
	}
	uart_puts(UART_ID,"0x");
	if(num==0)uart_putc_raw(UART_ID,'0');
	else{
		char str[10];				// definisce una stringa sulla quale convertire il numero da trasmettere (max 10 cifre)
		char i;						// contatore ciclo
		long remainder;			//resto
		for(i=0;i<10;i++) str[i]=0; // cancella la stringa
		i=0;
		while (num)
		{
        remainder = num % 16;
        if (remainder < 10)str[i++] = 48 + remainder;
        else str[i++] = 55 + remainder;
        num/=16;
		}
		for (i=0; i<10; i++)			// invia la stringa un carattere alla volta
		if (str[9-i]) uart_putc_raw(UART_ID,str[9-i]);
	}
	SerialN();
}
void Serial_Send_Bin(int num){
	if(num<0)
	{
		uart_puts(UART_ID,"-");
		num=-num;
	}
	uart_puts(UART_ID,"0b");
	if(num==0)uart_putc_raw(UART_ID,'0');
	else{
		char str[16];				// definisce una stringa sulla quale convertire il numero da trasmettere (max 10 cifre)
		char i;						// contatore ciclo
		long remainder;			//resto
		for(i=0;i<16;i++) str[i]='0'; // cancella la stringa
		i=0;
		while (num)
		{
        remainder = num % 2;
        str[i++] = 48 + remainder;
        num/=2;
		}
		for (i=0; i<16; i++)			// invia la stringa un carattere alla volta
		uart_putc_raw(UART_ID,str[15-i]);
	}
	SerialN();
}

void Serial_Send_String(char *str){
    uart_puts(UART_ID, str);
}











