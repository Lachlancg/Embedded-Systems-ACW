#include "buzzerDriver.h"

#include <xc.h>

#include "delay.h"

void buzzer_heater_on(void) {
    TRISB7 = 0;
    for (char i = 0; i < 3; i++) {
        RB7 = 1;
        delay(127, 127);
        RB7 = 0;
        if (i != 2) {
            delay(127, 127);
        }
    }
}

void buzzer_heater_off(void) {
    TRISB7 = 0;
    for (char i = 0; i < 2; i++) {
        RB7 = 1;
        delay(255, 255);
        RB7 = 0;
        if (i != 1) {
            delay(255, 127);
        }
    }
}
