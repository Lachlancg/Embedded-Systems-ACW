#include "clockDriver.h"

#include <xc.h>

//Define clock constants
#define CE             RB5 //DS1302 Reset Pin
#define SCLK           RB0 //DS1302 Clock pin
#define IO             RB4 //DS1302 IO pin
#define Data_Tris      TRISB4 //TRIS Register

typedef unsigned char byte;

static inline void DS1302_WriteByte(unsigned char W_Byte) 
{
    int i;
    
    for(i = 0; i < 8; i++)
    {
        IO = 0;
        SCLK = 0;
        
        if(W_Byte & 0x01)
        {
            IO = 1; /* set port pin high to read data */
        }
        
        W_Byte = W_Byte >> 1;
        SCLK = 1;
       
    }
    SCLK = 0;
}


static inline unsigned char DS1302_ReadByte()
 {
   int j;                            //set the loop counter.  
   unsigned char value = 0;
   TRISB4=1;                         //continue to write 8bit 
   for(j=0;j<8;j++)                  
      {
        SCLK=0;                       //pull low clk                   
        value=value>>1;           //judge the send bit is 0 or 1.  
        if(IO) value|=0x80;               //put the received bit into the reg's highest.
       SCLK=1;                       //pull high clk                 
      }                                                              
    TRISB4=0;                        //finished 1 byte,pull low clk  
    SCLK=0;                          
    return(value);                 
  }
void ds1302_init()
  {
    TRISB = 0x02;
    SCLK=0;                            //pull low clock
    CE =0;                            //reset DS1302
    CE=1;                             //enable DS1302
    DS1302_WriteByte(0x8e);                //send control command
    DS1302_WriteByte(0);                   //enable write DS1302
    CE=0;                             //reset
  }

void setTime(char clockTable[])
  {
    ds1302_init();
    int i;                             //define the loop counter.
    CE=1;                             //enable DS1302
    DS1302_WriteByte(0xbe);                //
    for(i=0;i<8;i++)                   //continue to write 8 bytes.
      {
        DS1302_WriteByte(clockTable[i]);        //write one byte
      }
    CE=0;                             //reset
  }


unsigned char get_dec(unsigned char var)
{
    unsigned char var2;
    
    var2 =  (var >> 4) * 10;
    var2 += (var & 15);
    
    return var2;        
}
unsigned char get_bcd(unsigned char data)
{
   unsigned char nibh;
   unsigned char nibl;

   nibh=data/10;
   nibl=data-(nibh*10);

   return((nibh<<4)|nibl);
}


void convertDateTime(char time_curr[], char date_curr[], char raw_datetime[])
{
    // Hour
    byte dec_hr = get_dec(raw_datetime[2]);

    // Minute
    byte dec_min = get_dec(raw_datetime[1]);

    // Seconds
    byte dec_sec = get_dec(raw_datetime[0]);

    //char time[] = "00:00:00";
    
    if(dec_hr > 9) 
    {
        time_curr[1] = (dec_hr % 10) + '0';
        time_curr[0] = (dec_hr / 10) + '0';
    }
    else 
    {
        time_curr[1] = dec_hr + '0';
        time_curr[0] =  '0';
    }

    if (dec_min < 10){          
        time_curr[4] =  dec_min + '0';
        time_curr[3] =  '0';
    }

    else
    {
        time_curr[3] =  (dec_min / 10) + '0';

        time_curr[4] = (dec_min % 10) + '0';
    }

    if(dec_sec < 10)
    {
        time_curr[7] =  dec_sec + '0';
        time_curr[6] =  '0';
    }
    else 
    {
        time_curr[6] =  (dec_sec / 10) + '0';

        time_curr[7] = (dec_sec % 10) + '0';
    }
    
     // date
    byte dec_date = get_dec(raw_datetime[3]);

    // month
    byte dec_month = get_dec(raw_datetime[4]);

    // year
    byte dec_year = get_dec(raw_datetime[6]);

    byte dec_day = get_dec(raw_datetime[5]);

    //char date[] = "00.00.00 X";
    
    if(dec_date < 10)
    {
        date_curr[1] = dec_date + '0';
        date_curr[0] = '0';
    }
    else
    {
        date_curr[0] =  (dec_date / 10) + '0';

        date_curr[1] = (dec_date % 10) + '0';
    }


    if(dec_month < 10)
    {
        date_curr[4] = dec_month + '0';
        date_curr[3] = '0';
    }
    else
    {
        date_curr[3] =  (dec_month / 10) + '0';

        date_curr[4] = (dec_month % 10) + '0';
    }

    if(dec_year < 10)
    {
        date_curr[7] = dec_year + '0';
        date_curr[6] = '0';
    }
    else
    {
        date_curr[6] =  (dec_year / 10) + '0';

        date_curr[7] = (dec_year % 10) + '0';
    }

}
void getTime(char raw_datetime[]) 
{
    
    CE = 1;
    DS1302_WriteByte(0xBF);
/*
    unsigned char sec = DS1302_ReadByte();
    unsigned char min = DS1302_ReadByte();
    unsigned char hr = DS1302_ReadByte();
    unsigned char date = DS1302_ReadByte();
    unsigned char month = DS1302_ReadByte();
    unsigned char day = DS1302_ReadByte();
    unsigned char year = DS1302_ReadByte();
    */
    raw_datetime[0] = DS1302_ReadByte();
    raw_datetime[1] = DS1302_ReadByte();
    raw_datetime[2] = DS1302_ReadByte();
    raw_datetime[3] = DS1302_ReadByte();
    raw_datetime[4] = DS1302_ReadByte();
    raw_datetime[5] = DS1302_ReadByte();
    raw_datetime[6] = DS1302_ReadByte();
    raw_datetime[7] = 0x00;

    CE = 0;
    /*
    raw_datetime[0] = sec;
    raw_datetime[1] = min;
    raw_datetime[2] = hr;
    raw_datetime[3] = date;
    raw_datetime[4] = month;
    raw_datetime[5] = day;
    raw_datetime[6] = year;*/

   
    
}

