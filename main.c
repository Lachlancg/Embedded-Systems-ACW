#include <xc.h>

#include "buzzerDriver.h"
#include "datetime.h"
#include "delay.h"
#include "ds18b20.h"
#include "lm016l.h"
#include "clockDriver.h"
#include "buttonMatrixDriver.h"

#define EEPROM_MAGIC 0xF1

#pragma config FOSC = HS
#pragma config WDTE = OFF
#pragma config PWRTE = ON
#pragma config BOREN = OFF
#pragma config LVP = OFF


/* Current temperature, as read by thermometer. */
static int s_temp_cur = 0;

/* Target temperature, as defined by human. */
static int s_temp_target = 18;

/*Delay durations, as defined by human.*/
static int delay_drops = 3;
static int delay_rises = 3;

/* 1 if we are between 7:00 AM and 10:30 PM */
static int s_sleep_time = 0;

/* 1 if the heater is on, false otherwise */
static int s_heater_on = 0;

static char datetime_curr[] = "00/00/00" "\0" "00:00:00";
static char *time_curr = datetime_curr + 9;
static char *date_curr = datetime_curr;
static char time_target[] = "00:00:00";
static char date_target[] = "00/00/00";
static char raw_datetime[8];
static char target_datetime[8];
static char datetime_toggle[17] = {0};

/*                      sec  min   hr    date  mon   day   year */
static char table[] = {0x57, 0x59, 0x12, 0x28, 0x02, 0x02, 0x21, 0x00};

static const char deg_c[] = {0xDF, 'C', 0x00};
static const char flame[] = {0xCA, 0x00};

static const char datetime_sleep_begin[] = "00/00/00 22:30:00";
static const char datetime_sleep_end[] = "00/00/00 07:00:00";

static enum {
    SCREEN_MAIN,
    SCREEN_TEMP,
    SCREEN_DATE_TIME,
    SCREEN_DELAY_DROPS,
    SCREEN_DELAY_RISES
} s_screen = SCREEN_MAIN;

/* Clear the display and draw the info. */
static void draw(void);

static void buttonUpdate(void);

static int itoa(int value, char *buf, unsigned int radix, int sign, unsigned int pad);

static void memzero(char *buf, int len);

static char max_day(const char month, const char year);

void main(void) {
    if (eeprom_read(0) == EEPROM_MAGIC) {
        // If the EEPROM is initialised...
        s_temp_target = eeprom_read(1);
        delay_drops = eeprom_read(2);
        delay_rises = eeprom_read(3);
    } else {
        // Otherwise...
        eeprom_write(0, EEPROM_MAGIC);
        eeprom_write(1, s_temp_target);
        eeprom_write(2, delay_drops);
        eeprom_write(3, delay_rises);
    }

    // Interrupts go away
    INTCONbits.GIE = 0;
    INTCONbits.PEIE = 0;

    setTime(table);

    ADCON1 = 0x07;
    TRISA = 0x00;
    TRISD = 0x00;
    TRISB1 = 0; //b port as output.
    int count = 0;

    for (;;) {
        count++;
        buttonUpdate();

        s_temp_cur = ds18b20_temp();

        getTime(raw_datetime);
        convertDateTime(time_curr, date_curr, raw_datetime);

        // s_sleep_time is 1 if we are after sleep_begin or before sleep_end
        s_sleep_time =
              compare_date_time(datetime_curr, datetime_sleep_begin, 1) == 1
              || compare_date_time(datetime_curr, datetime_sleep_end, 1) == -1;
        
        int want_heater_on = s_sleep_time == 0 && s_temp_cur < s_temp_target;

        if (s_heater_on == 0
                && want_heater_on == 1
                && datetime_toggle[0] == 0) {
            // The heater is off, and we don't want to
            // turn it on just yet.  But we want to turn it on if we still want
            // it on in delay_drops seconds.
            add_date_time(datetime_curr, datetime_toggle, delay_drops);           
        }
        else if (s_heater_on == 0
                && datetime_toggle[0] != 0
                && compare_date_time(datetime_curr, datetime_toggle, 0) != -1) {
            // The heater is off, we wanted to turn it on and delay_drops
            // seconds have passed.
            if (want_heater_on == 1) {
                // We still want to reset the heater after waiting delay_drops
                // seconds, so turn the heater on!
                s_heater_on = 1;
                buzzer_heater_on();
            }
            // Reset the delay counter.
            datetime_toggle[0] = 0;
        }
        else if (s_heater_on == 1
                && want_heater_on == 0
                && datetime_toggle[0] == 0) {
            // The heater is on, and we want to turn it off if the temperature
            // is high after delay_rises seconds.
            add_date_time(datetime_curr, datetime_toggle, delay_rises);
        }
        else if (s_heater_on == 1
                && datetime_toggle[0] != 0
                && compare_date_time(datetime_curr, datetime_toggle, 0) != -1) {
            // The heater is on, we wanted to turn it off, and delay_rises
            // seconds have passed.
            if (want_heater_on == 0) {
                // We still want to turn the heater off...
                s_heater_on = 0;
                buzzer_heater_off();
            }
            datetime_toggle[0] = 0;
        }

        TRISA5 = 0;
        RA5 = s_heater_on == 1 ? 1 : 0;

        if (count % 2 == 0) {
            draw();
        }

        delay(100, 0);
    }
}

static void buttonUpdate(void)
{
    int button = 0;
    button = checkButton();

    switch (s_screen) {
        case SCREEN_MAIN:
        {
            if(button == 10) { s_screen = SCREEN_TEMP; }
            if(button == 11) {
                s_screen = SCREEN_DATE_TIME;


                for (int i = 0; i < 8; i++)
                {
                    target_datetime[i] = raw_datetime[i];
                }
            }
            if(button == 12){ s_screen = SCREEN_DELAY_DROPS;}
            if(button == 13){ s_screen = SCREEN_DELAY_RISES;}
            break;
        }
        case SCREEN_TEMP:
        {
            if(button == 10) { s_screen = SCREEN_MAIN;}
            else if(button == 11){if(s_temp_target > 0){s_temp_target = s_temp_target -1;}}// Should this be able to go below 0
            else if(button == 12){if(s_temp_target < 100){s_temp_target = s_temp_target +1;}}
            else if(button == 13){}

            // Save the new target temp to the eeprom.
            eeprom_write(1, s_temp_target);
            break;
        }
        case SCREEN_DATE_TIME:
        {
            if(button == 10) { s_screen = SCREEN_MAIN;}
            else if(button == 13) {
                setTime(target_datetime);
                s_screen = SCREEN_MAIN;}
            /*Change Hours*/
            else if(button == 14){ unsigned char temp = get_dec(target_datetime[2]); //Convert BCD to Decimal
                                   if(temp > 0){
                                       temp = temp - 1;
                                       target_datetime[2] = get_bcd(temp);}}//Convert Decimal to BCD
            else if(button == 15){unsigned char temp = get_dec(target_datetime[2]);//Convert BCD to Decimal
                                  if(temp < 24){
                                      temp = temp + 1;
                                      target_datetime[2] = get_bcd(temp);}}//Convert Decimal to BCD
            /*Change Minutes*/
            else if(button == 16){ unsigned char temp = get_dec(target_datetime[1]); //Convert BCD to Decimal
                                   if(temp > 0){
                                       temp = temp - 1;
                                       target_datetime[1] = get_bcd(temp);}}//Convert Decimal to BCD
            else if(button == 17){unsigned char temp = get_dec(target_datetime[1]);//Convert BCD to Decimal
                                  if(temp < 60){
                                      temp = temp + 1;
                                      target_datetime[1] = get_bcd(temp);}}//Convert Decimal to BCD
            /*Change Seconds*/
            else if(button == 18){ unsigned char temp = get_dec(target_datetime[0]); //Convert BCD to Decimal
                                   if(temp > 0){
                                       temp = temp - 1;
                                       target_datetime[0] = get_bcd(temp);}}//Convert Decimal to BCD
            else if(button == 19){unsigned char temp = get_dec(target_datetime[0]);//Convert BCD to Decimal
                                  if(temp < 60){
                                      temp = temp + 1;
                                      target_datetime[0] = get_bcd(temp);}}//Convert Decimal to BCD

            /*Change Day*/

            else if(button == 20){ unsigned char temp = get_dec(target_datetime[3]); //Convert BCD to Decimal
                                   if(temp > 0){
                                       temp = temp - 1;
                                       target_datetime[3] = get_bcd(temp);}}//Convert Decimal to BCD
            else if(button == 21){unsigned char temp = get_dec(target_datetime[3]);//Convert BCD to Decimal
                                  if(temp < max_day(get_dec(target_datetime[4]), get_dec(target_datetime[6]) )){
                                      temp = temp + 1;
                                      target_datetime[3] = get_bcd(temp);}}//Convert Decimal to BCD
            /*Change Month*/
            else if(button == 22){ unsigned char temp = get_dec(target_datetime[4]); //Convert BCD to Decimal
                                   if(temp > 0){
                                       temp = temp - 1;
                                       target_datetime[4] = get_bcd(temp);
                                       if(get_dec(target_datetime[3]) > max_day(temp, get_dec(target_datetime[6])))
                                       {
                                           target_datetime[3] = get_bcd(max_day(temp, get_dec(target_datetime[6])));                                          
                                       }
                                   }}//Convert Decimal to BCD
            else if(button == 23){unsigned char temp = get_dec(target_datetime[4]);//Convert BCD to Decimal
                                  if(temp < 12){
                                      temp = temp + 1;
                                      target_datetime[4] = get_bcd(temp);
                                      if(get_dec(target_datetime[3]) > max_day(temp, get_dec(target_datetime[6])))
                                       {
                                           target_datetime[3] = get_bcd(max_day(temp, get_dec(target_datetime[6])));                                          
                                       }
                                  }}//Convert Decimal to BCD
            /*Change Year*/
            else if(button == 24){ unsigned char temp = get_dec(target_datetime[6]); //Convert BCD to Decimal
                                   if(temp > 0){
                                       temp = temp - 1;
                                       target_datetime[6] = get_bcd(temp);}}//Convert Decimal to BCD

            else if(button == 25){unsigned char temp = get_dec(target_datetime[6]);//Convert BCD to Decimal
                                  if(temp < 100){
                                      temp = temp + 1;
                                      target_datetime[6] = get_bcd(temp);}}//Convert Decimal to BCD
            break;
        }
        /*This screen is used to change the delay duration for when the heating drops below the target temp*/
        case SCREEN_DELAY_DROPS:
        {
            if(button == 10) { s_screen = SCREEN_MAIN;}
            else if(button == 11){if(delay_drops > 0){delay_drops = delay_drops -1;}}
            else if(button == 12){if(delay_drops < 90){delay_drops = delay_drops +1;}}
            eeprom_write(2, delay_drops);
            break;
        }
        /*This screen is used to change the delay duration for when the heating rises above the target temp*/
        case SCREEN_DELAY_RISES:
        {
            if(button == 10) { s_screen = SCREEN_MAIN;}
            else if(button == 11){if(delay_rises > 0){delay_rises = delay_rises -1;}}
            else if(button == 12){if(delay_rises < 60){delay_rises = delay_rises +1;}}
            eeprom_write(3, delay_rises);
            break;
        }

    }
}


static void draw(void) {
    char digits[5];

    lm016l_init();
    lm016l_clear();
    lm016l_goto_line(0);

    switch (s_screen) {
        case SCREEN_MAIN:
        {
             /* Draw current time and date. */
            lm016l_puts(date_curr);
            if (s_heater_on == 1) {
                lm016l_puts("       ");
                lm016l_puts(flame);
            }

            lm016l_goto_line(1);
            lm016l_puts(time_curr);

            /* Draw current temp. */
            lm016l_goto_line(2);
            lm016l_puts("temp: ");
            memzero(digits, 5);
            itoa(s_temp_cur, digits, 10, 1, 0);
            lm016l_puts(digits);
            lm016l_puts(deg_c);

            /* Draw target temp. */
            lm016l_goto_line(3);
            lm016l_puts("target: ");
            if (s_sleep_time == 0) {
                memzero(digits, 5);
                itoa(s_temp_target, digits, 10, 1, 0);
                lm016l_puts(digits);
                lm016l_puts(deg_c);
            } else {
                lm016l_puts("sleeping");
            }

            break;
        }
        case SCREEN_TEMP:
        {
            lm016l_puts("Change target:");
            /* Draw target temp. */
            lm016l_goto_line(1);
            memzero(digits, 5);
            itoa(s_temp_target, digits, 10, 1, 0);
            lm016l_puts(digits);
            lm016l_puts(deg_c);
            break;
        }
        case SCREEN_DATE_TIME:
        {

            //tempTable = table;
            lm016l_puts("Change date/time");

            /* Draw target temp. */
            lm016l_goto_line(1);

            convertDateTime(time_target, date_target, target_datetime);
            //lm016l_puts(raw_datetime);
            lm016l_puts(time_target);
            lm016l_goto_line(2);
            lm016l_puts(date_target);

            break;
        }
        case SCREEN_DELAY_DROPS:
        {
            lm016l_puts("Drop Delay:");

            /* Draw target temp. */
            lm016l_goto_line(1);
            itoa(delay_drops, digits, 10, 1, 0);
            lm016l_puts(digits);
            break;
        }
        case SCREEN_DELAY_RISES:
        {
            lm016l_puts("Rise Delay:");

            /* Draw target temp. */
            lm016l_goto_line(1);
            itoa(delay_rises, digits, 10, 1, 0);
            lm016l_puts(digits);
            break;
        }
    }
}

static char max_day(const char month, const char year)
{
    //unsigned char month = get_dec(target_datetime[4]);
    //unsigned char year = get_dec(target_datetime[6]);
    switch(month)
    {
        case 1:
            return 31;
        case 2:
        {
            if(((year%4==0) && ((year%400==0) || (year%100!=0))))
            {
                return 29;
            }
            else
            {
                return 28;
            }
        }
        case 3:
            return 31;
        case 4:
            return 30;
        case 5:
            return 31;
        case 6:
            return 30;
        case 7:
            return 31;
        case 8:
            return 31;
        case 9:
            return 30;
        case 10:
            return 31;
        case 11:
            return 30;
        case 12:
            return 31;
    }
    return 0;
}

/* Taken from https://github.com/00-matt/flibc */
static int itoa(int value, char *buf, unsigned int radix, int sign,
        unsigned int pad) {
    int neg = 0;
    char *pbuf = buf;
    unsigned int i;
    unsigned int len;

    if (radix > 16) {
        *buf = '\0';
        return 0;
    }

    if (sign && value < 0) {
        neg = 1;
        value = -value;
    }

    do {
        int digit = value % radix;
        *(pbuf++) = (digit < 10 ? '0' + digit : 'A' + digit - 10);
        value /= radix;
    } while (value > 0);

    for (i = pbuf - buf; i < pad; i++) {
        *(pbuf++) = 0;
    }

    if (neg) {
        *(pbuf++) = '-';
    }

    *pbuf = '\0';

    len = pbuf - buf;

    for (i = 0; i < len / 2; i++) {
        char tmp = buf[i];
        buf[i] = buf[len - i - 1];
        buf[len - i - 1] = tmp;
    }

    return len;
}

static void memzero(char *buf, int len) {
    for (int i = 0; i < len; i++) {
        buf[i] = 0;
    }
}
