//
//  LPT DAC I/O driver
//

#include <stdlib.h>
#include <dos.h>
#include "tornado.h"


extern int OverRun;
static volatile unsigned char byte=0;
static volatile int sword=0;
static int BusAddr=0x378;

int Detect_Bus()
{
unsigned int far *ptr=0x408;
BusAddr=*ptr;
if(BusAddr > 0)
	{
	printf("Detected LPT port on: 0x%x\n",BusAddr);
	return 1;
	}
return 0;
}

int Init_Bus(long freq, long *multiplier)
{
if(BusAddr < 1)
	return 0;
*multiplier=1;	// This driver must be 1:1 with the sample rate
return 1;
}

//
//  Send the sample data out
//

int Bus_Output(int wss_port, unsigned char *p)
{
sword = p[0]+(p[1]<<8);
sword += 32768;
outportb(BusAddr,(unsigned char)(sword>>8));
return 0; // Say we're done
}

int Bus_Ready(int wss_port)
{
/*
byte=inportb(BusAddr+1);
if(!(byte & 0x80))
	return 0;
*/
return 1;
}
