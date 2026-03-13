// lab4main.c
// Charlie Tejano

#include "lab4LCD.h"
#include "stm32f4xx.h"
#include "RPG.h"
#include <stdint.h>

int main(void)
{
    int32_t previous_count = 0;
    int32_t current_count  = 0;
    // Fixed: cast comparisons use (int8_t) to match rotaryGetDir() return type
    int8_t  current_dir    = (int8_t)rotary_dir_none;
    int     first_move     = 0;

    // Initialize LCD port then LCD controller
    LCD_port_init();
    LCD_init();

    // Initialize rotary encoder
    rotary_Init();

    // Initial splash screen
    LCD_clearDisplay();
    LCD_placeCursor(1);
    LCD_printString("Rotary Pulse Gen");
    LCD_placeCursor(2);
    LCD_printString("Rotate Knob     ");

    previous_count = rotaryGetCount();
    current_dir    = (int8_t)rotary_dir_none;

    while (1)
    {
        // Atomic reads
        current_count = rotaryGetCount();
        current_dir   = rotaryGetDir();

        //Update display only on change
        if ((current_count != previous_count) ||
            ((current_dir != (int8_t)rotary_dir_none) && (first_move == 0)))
        {
            first_move = 1;

            LCD_clearDisplay();

            //direction label
            LCD_placeCursor(1);
            if (current_dir == (int8_t)rotary_dir_cw)
            {
                LCD_printString("Moving CW       ");
            }
            else if (current_dir == (int8_t)rotary_dir_ccw)
            {
                LCD_printString("Moving CCW      ");
            }
            else
            {
                LCD_printString("No Movement     ");
            }

            // Line 2: pulse count
            LCD_placeCursor(2);
            LCD_printString("# Turns = ");
            LCD_printInt((int)current_count);

            previous_count = current_count;
        }

        //Button press — reset everything
        if (rotaryBtnPress())
        {
            rotaryReset();
            first_move     = 0;
            previous_count = rotaryGetCount();
            current_dir    = (int8_t)rotary_dir_none;

            LCD_clearDisplay();
            LCD_placeCursor(1);
            LCD_printString("Rotary Pulse Gen");
            LCD_placeCursor(2);
            LCD_printString("Rotate Knob     ");

            //debounce
            delay_ms(300);
        }
    }
}