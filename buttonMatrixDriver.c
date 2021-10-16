#include <xc.h>
#include "delay.h"

int checkButton(void)
{
    TRISC = 0X0F; // Half of C port as input.

    PORTC = 0x10;
    if(PORTC & 0x01) { delay(50,0); if(PORTC & 0x01){while(PORTC & 0x01){} return 13; }}//K13
    if(PORTC & 0x02) { delay(50,0); if(PORTC & 0x02){while(PORTC & 0x02){} return 12; }}//K12
    if(PORTC & 0x04) { delay(50,0); if(PORTC & 0x04){while(PORTC & 0x04){} return 11; }}//K11
    if(PORTC & 0x08) { delay(50,0); if(PORTC & 0x08){ while(PORTC & 0x08){} return 10; }}//K10

    PORTC = 0x20;
    if(PORTC & 0x01) { delay(50,0); if(PORTC & 0x01){while(PORTC & 0x01){} return 17; }}//K17
    if(PORTC & 0x02) { delay(50,0); if(PORTC & 0x02){while(PORTC & 0x02){} return 16; }}//K16
    if(PORTC & 0x04) { delay(50,0); if(PORTC & 0x04){while(PORTC & 0x04){} return 15; }}//K15
    if(PORTC & 0x08) { delay(50,0); if(PORTC & 0x08){while(PORTC & 0x08){} return 14; }}//K14

    PORTC = 0x40;
    if(PORTC & 0x01) { delay(50,0); if(PORTC & 0x01){while(PORTC & 0x01){} return 21; }}//K21
    if(PORTC & 0x02) { delay(50,0); if(PORTC & 0x02){while(PORTC & 0x02){} return 20; }}//K20
    if(PORTC & 0x04) { delay(50,0); if(PORTC & 0x04){while(PORTC & 0x04){} return 19; }}//K19
    if(PORTC & 0x08) { delay(50,0); if(PORTC & 0x08){while(PORTC & 0x08){} return 18; }}//K18

    PORTC = 0x80;
    if(PORTC & 0x01) { delay(50,0); if(PORTC & 0x01){while(PORTC & 0x01){} return 25; }}//K25
    if(PORTC & 0x02) { delay(50,0); if(PORTC & 0x02){while(PORTC & 0x02){} return 24; }}//K24
    if(PORTC & 0x04) { delay(50,0); if(PORTC & 0x04){while(PORTC & 0x04){} return 23; }}//K23
    if(PORTC & 0x08) { delay(50,0); if(PORTC & 0x08){while(PORTC & 0x08){} return 22; }}//K22

    return  0;

}