// lcd.c
// Charlie Tejano

#include "lab4LCD.h"
#include "stm32f4xx.h"
#include <stdio.h>

void delay_us(uint32_t us) {
    for (volatile uint32_t i = 0; i < us * 16u; i++);  // 16MHz clock
}

void delay_ms(uint32_t ms) {
    for (volatile uint32_t i = 0; i < ms * 16000u; i++);
}

void LCD_port_init(void) {
    // STEP 1: Enable GPIOD in RCC AHB1ENR register
    RCC->AHB1ENR |= (1u << 3);

    // STEP 2: Set MODER of GPIOD Pins 7, 6, 5, 3, 2, 1 & 0 as outputs
    GPIOD->MODER &= ~((uint32_t)0x0000FCFF);
    GPIOD->MODER |=  ((uint32_t)0x00005455);

    // STEP 3: Set OTYPER of GPIOD Pins 7, 6, 5, 3, 2, 1 & 0 as push-pull
    GPIOD->OTYPER &= ~((uint32_t)0x00FF);
}

/*******************************
 * LCD_sendInstr_raw()
 * Internal helper used DURING init only.
 * Does NOT call check_BF() — uses delay instead.
 * Avoids hang caused by check_BF() before LCD is ready.
 *******************************/
static void LCD_sendInstr_raw(unsigned char Instruction) {
    clear_PIN(RS);
    clear_PIN(RW);

    // Send upper 4-bits
    set_PIN(EN);
    GPIOD->ODR = (GPIOD->ODR & ~(uint32_t)0x0F) | ((uint32_t)(Instruction >> 4));
    clear_PIN(EN);

    // Send lower 4-bits
    set_PIN(EN);
    GPIOD->ODR = (GPIOD->ODR & ~(uint32_t)0x0F) | ((uint32_t)(Instruction & 0x0F));
    clear_PIN(EN);

    delay_us(50);
}

void LCD_init(void) {
    // STEP 1: Power-on delay (>40ms)
    delay_ms(50);

    // STEP 2: Set RS and RW LOW
    clear_PIN(RS);
    clear_PIN(RW);

    // STEP 3: Send 0x03 three times to sync 8-bit mode
    for (int i = 0; i < 3; i++) {
        set_PIN(EN);
        GPIOD->ODR = (GPIOD->ODR & ~(uint32_t)0x0F) | 0x03u;
        clear_PIN(EN);
        delay_ms(5);
    }

    // STEP 4: Switch to 4-bit mode — send 0x02
    set_PIN(EN);
    GPIOD->ODR = (GPIOD->ODR & ~(uint32_t)0x0F) | 0x02u;
    clear_PIN(EN);
    delay_ms(2);

    // Confirm 4-bit mode
    set_PIN(EN);
    GPIOD->ODR = (GPIOD->ODR & ~(uint32_t)0x0F) | 0x02u;
    clear_PIN(EN);
    delay_us(150);

    // STEP 5: Function Set — 4-bit, 2 lines, 5x8 font (0x28)
    LCD_sendInstr_raw(0x28);

    // STEP 6: Display OFF
    LCD_sendInstr_raw(0x08);

    // STEP 7: Clear Display
    LCD_sendInstr_raw(0x01);
    delay_ms(2);

    // STEP 8: Entry Mode Set — increment, no shift
    LCD_sendInstr_raw(0x06);

    // STEP 9: Display ON, cursor ON, blink ON
    LCD_sendInstr_raw(0x0F);
}

void LCD_placeCursor(uint32_t lineno) {
    if (lineno == 1u) {
        LCD_sendInstr(0x80u | 0x00u);
    } else if (lineno == 2u) {
        LCD_sendInstr(0x80u | 0x40u);
    }
}

void LCD_sendData(unsigned char data) {
    check_BF();

    set_PIN(RS);
    clear_PIN(RW);

    // Send upper 4-bits
    set_PIN(EN);
    GPIOD->ODR = (GPIOD->ODR & ~(uint32_t)0x0F) | ((uint32_t)((data >> 4) & 0x0F));
    clear_PIN(EN);

    // Send lower 4-bits
    set_PIN(EN);
    GPIOD->ODR = (GPIOD->ODR & ~(uint32_t)0x0F) | ((uint32_t)(data & 0x0F));
    clear_PIN(EN);
}

void LCD_sendInstr(unsigned char Instruction) {
    check_BF();

    clear_PIN(RS);
    clear_PIN(RW);

    // Send upper 4-bits
    set_PIN(EN);
    GPIOD->ODR = (GPIOD->ODR & ~(uint32_t)0x0F) | ((uint32_t)(Instruction >> 4));
    clear_PIN(EN);

    // Send lower 4-bits
    set_PIN(EN);
    GPIOD->ODR = (GPIOD->ODR & ~(uint32_t)0x0F) | ((uint32_t)(Instruction & 0x0F));
    clear_PIN(EN);
}

void LCD_clearDisplay(void) {
    LCD_sendInstr(0x01);
    delay_ms(2);  // Clear display needs up to 1.64ms
}

void clear_PIN(int PINNO) {
    GPIOD->BSRR = (1u << ((uint32_t)PINNO + 16u));
}

void set_PIN(int PINNO) {
    GPIOD->BSRR = (1u << (uint32_t)PINNO);
}

void check_BF(void) {
    // STEP 1: RS = 0 (instruction), RW = 1 (read)
    clear_PIN(RS);

    // STEP 2: Set DB7 pin (GPIOD Pin DB7) as input
    GPIOD->MODER &= ~(3u << ((uint32_t)DB7 * 2u));

    // STEP 3: RW = 1 to read
    set_PIN(RW);

    int is_busy = 1;
    while (is_busy) {
        // Clock high nibble — DB7 is the Busy Flag
        set_PIN(EN);
        is_busy = (int)(GPIOD->IDR & (1u << (uint32_t)DB7));
        clear_PIN(EN);

        // Dummy clock low nibble (required in 4-bit mode)
        set_PIN(EN);
        clear_PIN(EN);
    }

    // Restore RW = 0 (write mode)
    clear_PIN(RW);

    // Restore DB7 pin back to output
    GPIOD->MODER &= ~(3u << ((uint32_t)DB7 * 2u));
    GPIOD->MODER |=  (1u << ((uint32_t)DB7 * 2u));
}

void LCD_printChar(char c) {
    LCD_sendData((unsigned char)c);
}

void LCD_printString(const char text[]) {
    int i = 0;

    while (text[i] != '\0') {
        if (i == 16) {
            LCD_placeCursor(2);
        }
        if (i == 32) {
            break;
        }
        LCD_printChar(text[i]);
        i++;
    }
}

void LCD_printInt(int number) {
    char buffer[16];
    snprintf(buffer, sizeof(buffer), "%d", number);
    LCD_printString(buffer);
}

void LCD_printFloat(float number, int decimal_places) {
    char buffer[16];
    snprintf(buffer, sizeof(buffer), "%.*f", decimal_places, number);
    LCD_printString(buffer);
}





