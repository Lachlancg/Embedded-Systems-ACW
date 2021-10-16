#include "ds18b20.h"

#include <xc.h>

#include "delay.h"

#define DQ RA0
#define DQ_DIR RA0
#define DQ_HIGH() do { DQ_DIR = 1; } while (0)
#define DQ_LOW() do { DQ = 0; DQ_DIR = 0; } while (0)

static unsigned char ds18b20_read(void) {
    unsigned char i, j;
    unsigned char value = 0;
    for (i = 8; i > 0; i--) {
        value >>= 1;
        DQ_LOW();
        NOP();
        DQ_HIGH();
        NOP();
        j = DQ;
        if (j) {
            value |= 0x80;
        }
        delay(2, 3); /* 63us */
    }
    return value;
}

static void ds18b20_write(unsigned char b) {
    unsigned char i;
    for (i = 8; i > 0; i--) {
        DQ_LOW();
        NOP();
        NOP();
        /* If the bit we are writing is 1. */
        if (b & 1) {
            DQ_HIGH();
        }
        delay(2, 3); /* 63us */
        DQ_HIGH();
        NOP();
        b = b >> 1;
    }
}

static void ds18b20_reset(void) {
    char presence = 1;
    while (presence) {
        DQ_LOW();
        RB0 = 0;
        delay(1, 50); /* 503us to 650us */
        DQ_HIGH();
        RB0 = 1;
        delay(2, 3); /* 70us */
        RB1 = 0;
        RB0 = 0;
        presence = DQ;
        delay(2, 60); /* 430us */
        RB1 = 1;
        RB0 = 0;
    }
}

int ds18b20_temp(void) {
    int lo, hi, whole, fraction;

    ds18b20_reset();
    ds18b20_write(0xCC); /* Ignore ROM matching. */
    ds18b20_write(0x44); /* Temperature convert. */

    ds18b20_reset();
    ds18b20_write(0xCC);
    ds18b20_write(0xBE);

    lo = ds18b20_read();
    hi = ds18b20_read();

    whole = (lo >> 4) | ((hi << 4) & 0x3F);
    fraction = lo << 4;

    return whole;
}
