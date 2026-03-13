// RPG.c
// Charlie Tejano

#include "stm32f4xx.h"
#include "RPG.h"

// Volatile globals — shared between ISR and main
volatile int32_t rotary_count = 0;
volatile int8_t  rotary_dir   = rotary_dir_none;

// Keeping here to avoid redefinition if RPG.h already declares them
#ifndef ROTARY_OUTA_PIN
#define ROTARY_OUTA_PIN  1
#endif
#ifndef ROTARY_OUTB_PIN
#define ROTARY_OUTB_PIN  0
#endif
#ifndef ROTARY_SW_PIN
#define ROTARY_SW_PIN    15
#endif

void rotary_Init(void)
{
    // Enable GPIOB and SYSCFG clocks
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;
    RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;

    // Dummy reads for clock stabilization
    (void)RCC->AHB1ENR;
    (void)RCC->APB2ENR;

    // Set PB0, PB1, PB15 as inputs (MODER = 00)
    GPIOB->MODER &= ~(3u << (ROTARY_OUTB_PIN * 2));
    GPIOB->MODER &= ~(3u << (ROTARY_OUTA_PIN * 2));
    GPIOB->MODER &= ~(3u << (ROTARY_SW_PIN   * 2));

    // PB0 (OUTB) — pull-up
    GPIOB->PUPDR &= ~(3u << (ROTARY_OUTB_PIN * 2));
    GPIOB->PUPDR |=  (1u << (ROTARY_OUTB_PIN * 2));

    // PB1 (OUTA) — pull-up
    GPIOB->PUPDR &= ~(3u << (ROTARY_OUTA_PIN * 2));
    GPIOB->PUPDR |=  (1u << (ROTARY_OUTA_PIN * 2));

    // PB15 (SW) — pull-up (active-low button)
    GPIOB->PUPDR &= ~(3u << (ROTARY_SW_PIN * 2));
    GPIOB->PUPDR |=  (1u << (ROTARY_SW_PIN * 2));

    // Map EXTI1 to PB1 via SYSCFG EXTICR1 bits [7:4]
    SYSCFG->EXTICR[0] &= ~(0xFu << 4);
    SYSCFG->EXTICR[0] |=  (0x1u << 4);

    // Unmask EXTI line 1
    EXTI->IMR |= (1u << 1);

    // Trigger on both edges for full quadrature accuracy
    EXTI->RTSR |= (1u << 1);
    EXTI->FTSR |= (1u << 1);

    // Clear any stale pending interrupt
    EXTI->PR = (1u << 1);

    // Configure NVIC
    NVIC_ClearPendingIRQ(EXTI1_IRQn);
    NVIC_SetPriority(EXTI1_IRQn, 2);
    NVIC_EnableIRQ(EXTI1_IRQn);

    __enable_irq();
}

void EXTI1_IRQHandler(void)
{
    if (EXTI->PR & (1u << 1))
    {
        // Sample OUTB to determine direction
        if ((GPIOB->IDR & (1u << ROTARY_OUTB_PIN)) == 0u)
        {
            rotary_count++;
            rotary_dir = rotary_dir_cw;
        }
        else
        {
            rotary_count--;
            rotary_dir = rotary_dir_ccw;
        }

        // Clear pending EXTI1 flag
        EXTI->PR = (1u << 1);
    }
}

int32_t rotaryGetCount(void)
{
    int32_t count;
    __disable_irq();
    count = rotary_count;
    __enable_irq();
    return count;
}

int8_t rotaryGetDir(void)
{
    int8_t dir;
    __disable_irq();
    dir = rotary_dir;
    __enable_irq();
    return dir;
}

uint8_t rotaryBtnPress(void)
{
    // Active-low: returns 1 if button pressed
    return ((GPIOB->IDR & (1u << ROTARY_SW_PIN)) == 0u) ? 1u : 0u;
}

void rotaryReset(void)
{
    __disable_irq();
    rotary_count = 0;
    rotary_dir   = rotary_dir_none;
    __enable_irq();
}

float rotaryGetTurns(uint32_t detents_per_turn)
{
    if (detents_per_turn == 0u)
    {
        return 0.0f;
    }
    int32_t count;
    __disable_irq();
    count = rotary_count;
    __enable_irq();
    return ((float)count) / ((float)detents_per_turn);
}







