// RPG.h

//Charlie Tejano
#ifndef RPG_H
#define RPG_H

#include "stm32f4xx.h"
#include <stdint.h>
    
// Pin definitions

/*
* Interrupt on rising edge of OUTA - EXTI1
* Check state of OUTB to determine direction
* Push button on PB15
*/

// OUTA->PB1

// OUTB->PB0

// SWITCH->PB15

#define rotary_dir_none 0
#define rotary_dir_cw 1
#define rotary_dir_ccw -1

void rotary_Init(void);
int32_t rotaryGetCount(void);
int8_t rotaryGetDir(void);
uint8_t rotaryBtnPress(void);
void rotaryReset(void);
float rotaryGetTurns(uint32_t detents_per_turn);

#endif

























