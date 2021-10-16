#include "lm016l.h"

#include <xc.h>

#include "delay.h"

/*
 * LCD Pinout.
 */
#define DI RA1
#define RW RA2
#define E RA3

/*
 * Controller commands
 */
#define COMMAND_CLEAR 0x01

/*
 * LCD DDRAM addresses | COMMAND_WRITE_DDRAM.
 */
#define ADDR_L1 0x80
#define ADDR_L2 0xC0
#define ADDR_L3 0x90
#define ADDR_L4 0xD0

static void lm016l_write(char di, char c) {
    /* Tell the controller if this is data or command. */
    DI = di;
    
    /* Tell the controller we are writing. */
    RW = 0;
    
    /* Actually perform the write. */
    PORTD = c;
    
    /* Inform the LCD that we've written something. */
    E = 0;
    
    /* Sleep for some amount of time to give LCD chance to read. */
    delay(4, 20);
    
    /* Bring E high again. */
    E = 1;
    
    /* Wait longer for commands for some reason. */
    if (di == 0) {
        delay(4, 20);
    }
}

void lm016l_init(void) {
    lm016l_write(0, 0x0E);
    lm016l_write(0, 0x10);    
    lm016l_write(0, 0x38);
}

void lm016l_clear(void) {
    lm016l_write(0, COMMAND_CLEAR);
}

void lm016l_goto_line(char l) {
    switch (l) {
        case 0:
            lm016l_write(0, ADDR_L1);
            return;
        case 1:
            lm016l_write(0, ADDR_L2);
            return;
        case 2:
            lm016l_write(0, ADDR_L3);
            return;
        case 3:
            lm016l_write(0, ADDR_L4);
            return;
        default:
            return;
    }
}

void lm016l_puts(const char *s) {
    while (*s) {
        lm016l_write(1, *s++);
    }
}
