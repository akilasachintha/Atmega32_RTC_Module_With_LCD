#include <avr/io.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "I2C/I2C_Master_H_file.h"
#include "LCD/lcd.h"

#define Device_Write_address	0xD0				/* Define RTC DS1307 slave address for write operation */
#define Device_Read_address		0xD1				/* Make LSB bit high of slave address for read operation */
#define TimeFormat12			0x40				/* Define 12 hour format */
#define AMPM					0x20
#define hour_12_AM				0x40
#define hour_12_PM				0x60
#define hour_24					0x00

int second,minute,hour,day,date,month,year;

bool IsItPM(char hour_)
{
	if(hour_ & (AMPM))
		return 1;
	else
		return 0;
}

void RTC_Read_Clock(char read_clock_address)
{
	I2C_Start(Device_Write_address);				/* Start I2C communication with RTC */
	I2C_Write(read_clock_address);					/* Write address to read */
	I2C_Repeated_Start(Device_Read_address);		/* Repeated start with device read address */

	second = I2C_Read_Ack();						/* Read second */
	minute = I2C_Read_Ack();						/* Read minute */
	hour = I2C_Read_Nack();							/* Read hour with Nack */
	I2C_Stop();										/* Stop i2C communication */
}

void RTC_Read_Calendar(char read_calendar_address)
{
	I2C_Start(Device_Write_address);
	I2C_Write(read_calendar_address);
	I2C_Repeated_Start(Device_Read_address);

	day = I2C_Read_Ack();							/* Read day */ 
	date = I2C_Read_Ack();							/* Read date */
	month = I2C_Read_Ack();							/* Read month */
	year = I2C_Read_Nack();							/* Read the year with Nack */
	I2C_Stop();										/* Stop i2C communication */
}

void RTC_Clock_Write(char _hour, char _minute, char _second, char AM_PM)
{
	_hour |= AM_PM;
	I2C_Start(Device_Write_address);/* Start I2C communication with RTC */
	I2C_Write(0);			/* Write 0 address for second */
	I2C_Write(_second);		/* Write second on 00 location */
	I2C_Write(_minute);		/* Write minute on 01(auto increment) location */
	I2C_Write(_hour);		/* Write hour on 02 location */
	I2C_Stop();			/* Stop I2C communication */
}

/* function for calendar */
void RTC_Calendar_Write(char _day, char _date, char _month, char _year)
{
	I2C_Start(Device_Write_address);/* Start I2C communication with RTC */
	I2C_Write(3);			/* Write 3 address for day */
	I2C_Write(_day);		/* Write day on 03 location */
	I2C_Write(_date);		/* Write date on 04 location */
	I2C_Write(_month);		/* Write month on 05 location */
	I2C_Write(_year);		/* Write year on 06 location */
	I2C_Stop();			/* Stop I2C communication */
}

int main(void){
	char buffer[20];
	char* days[7]= {"Sun","Mon","Tue","Wed","Thu","Fri","Sat"};
		
	lcd_init(LCD_DISP_ON);
	I2C_Init();
	
	
	RTC_Clock_Write(0x11, 0x59, 0x00, hour_12_AM);/* Write Hour Minute Second Format */
	RTC_Calendar_Write(0x01, 0x01, 0x01, 0x21);	/* Write day date month and year */
	
	while(1){
		RTC_Read_Clock(0);							/* Read the clock with second address i.e location is 0 */
		if (hour & TimeFormat12)
		{
			sprintf(buffer, "%02x:%02x:%02x  ", (hour & 0b00011111), minute, second);
			if(IsItPM(hour))
				strcat(buffer, "PM");
			else
				strcat(buffer, "AM");
			lcd_gotoxy(0, 0);
			lcd_puts(buffer);
		}
		else
		{
			sprintf(buffer, "%02x:%02x:%02x  ", (hour & 0b00011111), minute, second);
			lcd_gotoxy(0, 0);
			lcd_puts(buffer);
		}
		
		RTC_Read_Calendar(3);						/* Read the calender with day address i.e location is 3 */
		sprintf(buffer, "%02x/%02x/%02x %3s ", date, month, year,days[day-1]);
		lcd_gotoxy(0, 1);
		lcd_puts(buffer);
	}
	
}