//*****************************************************************************
//
//! \file main.c
//! \brief main application
//! \version 1.0.0.0
//! \date October 2017
//! \author Zbigniew Mansel
//! \copy
//!
//! Copyright (c) 2014 CooCox.  All rights reserved.
//*****************************************************************************
#include <stm32f10x_gpio.h>
#include <stm32f10x_rcc.h>
#include "stm32f10x_flash.h"
#include "stm32f10x.h"
#include "misc.h"

extern uint32_t ticks;

void delay_ms(int time);
void send_char(char c);
void send_string(const char* s);
void send_num(uint32_t x);
void move();




int main(void)
{
	GPIO_InitTypeDef gpio;
	ADC_InitTypeDef adc;
	USART_InitTypeDef uart;
	TIM_TimeBaseInitTypeDef tim;
	NVIC_InitTypeDef nvic;
	TIM_OCInitTypeDef channel;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOD, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);

	RCC_ADCCLKConfig(RCC_PCLK2_Div6);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);

	GPIO_StructInit(&gpio);
	gpio.GPIO_Pin = GPIO_Pin_2;
	gpio.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOA, &gpio);

	gpio.GPIO_Pin = GPIO_Pin_3;
	gpio.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA, &gpio);

	USART_StructInit(&uart);
	uart.USART_BaudRate = 115200;
	USART_Init(USART2, &uart);
	USART_Cmd(USART2, ENABLE);

	TIM_TimeBaseStructInit(&tim);
	tim.TIM_CounterMode = TIM_CounterMode_Up;
	tim.TIM_Prescaler = 6400 - 1;
	tim.TIM_Period = 1000 - 1;
	TIM_TimeBaseInit(TIM4, &tim);

	TIM_OCStructInit(&channel);
	channel.TIM_OCMode = TIM_OCMode_PWM1;
	channel.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OC1Init(TIM4, &channel);

	TIM_ITConfig(TIM4, TIM_IT_Update, ENABLE);
	TIM_Cmd(TIM4, ENABLE);

	GPIO_StructInit(&gpio);
	gpio.GPIO_Pin = GPIO_Pin_6;
	gpio.GPIO_Speed = GPIO_Speed_50MHz;
	gpio.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOB, &gpio);

	USART_Cmd(USART2, ENABLE);

	SysTick_Config(SystemCoreClock / 1000);

	TIM_SetCompare1(TIM4, 700);

	//RIGHT WHEEL: [C5+C6] ON -> Forward, [C8+C9] ON -> Backward
	GPIO_StructInit(&gpio);
	gpio.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_8 | GPIO_Pin_9;
	gpio.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOC, &gpio);

	//LEFT WHEEL: [C10+C11] ON -> Forward, [C12+C13] ON -> Backward
	GPIO_StructInit(&gpio);
	gpio.GPIO_Pin = GPIO_Pin_10 | GPIO_Pin_11 | GPIO_Pin_12 | GPIO_Pin_13;
	gpio.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOC, &gpio);


    while(1)
    {
    	move();

    }
}



void move(){
	int counter = 0;
	while (USART_GetFlagStatus(USART2, USART_FLAG_RXNE)){
		GPIOC -> ODR = 0x0000;
		char c = USART_ReceiveData(USART2);
		delay_ms(40);
		if (counter == 0){
			switch((int)c){
			case 56://forward
				GPIOC -> ODR = 0x0000;
				GPIO_SetBits(GPIOC, GPIO_Pin_5);
				GPIO_SetBits(GPIOC, GPIO_Pin_6);
				GPIO_SetBits(GPIOC, GPIO_Pin_10);
				GPIO_SetBits(GPIOC, GPIO_Pin_11);
				send_string("MOVING FORWARD \r\n");
				break;
			case 50://backward
				GPIOC -> ODR = 0x0000;
				GPIO_SetBits(GPIOC, GPIO_Pin_8);
				GPIO_SetBits(GPIOC, GPIO_Pin_9);
				GPIO_SetBits(GPIOC, GPIO_Pin_12);
				GPIO_SetBits(GPIOC, GPIO_Pin_13);
				send_string("MOVING BACKWARD \r\n");
				break;
			case 52://left
				GPIOC -> ODR = 0x0000;
				GPIO_SetBits(GPIOC, GPIO_Pin_5);
				GPIO_SetBits(GPIOC, GPIO_Pin_6);
				send_string("TURNING LEFT \r\n");
				break;
			case 54://right
				GPIOC -> ODR = 0x0000;
				GPIO_SetBits(GPIOC, GPIO_Pin_10);
				GPIO_SetBits(GPIOC, GPIO_Pin_11);
				send_string("TURNING RIGHT \r\n");
				break;
			default:
				send_string("STOP \r\n");
				GPIOC -> ODR = 0x0000;
			}counter = 1;
		}
	}
}

void delay_ms(int time){
	ticks = 0;
	while (ticks < time);
}

void send_char(char c)
{
	while (USART_GetFlagStatus(USART2, USART_FLAG_TXE) == RESET);
	USART_SendData(USART2, c);
}

void send_string(const char* s)
{
	while (*s)
		send_char(*s++);
}

void send_num(uint32_t x){

	char value[10];
	int i = 0;

	do{
		value[i++] = (char)(x%10) + '0';
		x /= 10;
	}while(x);

	while(i){
		send_char(value[--i]);
	}
}
//! @}
//! @}
