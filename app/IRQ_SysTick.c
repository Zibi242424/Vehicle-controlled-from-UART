#include "stm32f10x.h"

volatile unsigned int ticks;

void SysTick_Handler (void) {
	ticks++;
}
