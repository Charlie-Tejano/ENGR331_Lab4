//RPG.c

//Charlie Tejano
#include "stm32f4xx.h"
#include <stdio.h>
#include "Rotary.h"

//Lock in rotary count and direction
volatile int32_t rotary_count = 0;
volatile int8_t rotary_dir = rotary_dir_none;

//rotary functions
#define ROTARY_OUTA_PIN 1
#define ROTARY_OUTB_PIN 0
#define ROTARY_SW_PIN 15

//Initialize the rotary
void rotary_Init(void)
{
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN; //Enable GPIOB
    RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN; //Enable SYSCFG

    //dummy read after initializing to allow clock to stabilize
    (void)RCC->AHB1ENR;
    (void)RCC->APB2ENR;

    // three-lines of setting up the following pins -PB0, PB1, and PB15
    GPIOB->MODER &= ~(3u << (ROTARY_OUTB_PIN*2));
    GPIOB->MODER &= ~(3u << (ROTARY_OUTA_PIN*2));
    GPIOB->MODER &= ~(3u << (ROTARY_SW_PIN*2));

    //Pull-up and pull-down

    //PB15 pull-up
    GPIOB->PUPDR &= ~(3u << (ROTARY_OUTB_PIN*2));
    GPIOB->PUPDR &= ~(3u << (ROTARY_OUTA_PIN*2));

    //PB0, PB1 pull-down
    GPIOB->PUPDR &= ~(3u << (ROTARY_SW_PIN*2));
    GPIOB->PUPDR |= (1u << (ROTARY_SW_PIN*2));

    //EXTI1, using EXTICR1
    SYSCFG->EXTICR[0] &= ~(0xFU<<4);
    SYSCFG->EXTICR[0] |= (0x1U<<4);

    //Unmask EXTI1
    EXTI->IMR |= (1u<<1);

    //Rising edge on PB1 (OUTA)
    EXTI->RTSR |= (1u<<1);
    EXTI->FTSR &= ~(1u<<1);

    //Clear pending bit for EXTI1
    EXTI->PR = (1u<<1);

    //set up NVIC for EXTI1
    NVIC_ClearPendingIRQ(EXTI1_IRQn);
    NVIC_SetPriority(EXTI1_IRQn, 2);
    NVIC_EnableIRQ(EXTI1_IRQn);

    __enableIRQ();
}

//Interrupt handler for EXTI1
void EXTI1_IRQHandler(void)
{
    if (EXTI1->PR & (1u<<1)) {
        if ((GPIOB->IDR & (1u<<ROTARY_OUTB_PIN)) == 0u) { //Check if OUTB is low
            rotary_count++;
            rotary_dir = rotary_dir_cw;
        } else {
            rotary_count--;
            rotary_dir = rotary_dir_ccw;
        }
        EXTI->PR = (1u<<1);
    }
}

//Lock in rotary count
volatile uint32_t rotaryGetCount(void)
{
    return rotary_count;
}

//Get rotary direction
int8_t rotaryGetDir(void)
{
    return rotary_dir;
}
//Rotary button pressed
uint8_t rotaryBtnPress(void)
{
    if ((GPIOB->IDR & (1u<<ROTARY_SW_PIN)) == 0u) {
        return 1u;
    }
    return 0u;
}

//Reset rotary count as well as direction
void rotaryReset(void)
{
    rotary_count = 0;
    rotary_dir = rotary_dir_none;
} 

//Get # of turns
float rotaryGetTurns(uint32_t detents_per_turn){ 
    if (detents_per_turn == 0) {
        return 0.0f;
    }
    return ((float)rotary_count) / ((float)detents_per_turn);
}







