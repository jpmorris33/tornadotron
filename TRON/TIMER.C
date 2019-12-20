//
//	Timer driver
//

#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <dos.h>
#include <string.h>
#include <alloc.h>

#include "channel.h"
#include "timer.h"
#include "midi.h"

void T_Term();
extern void T_Update();
extern int M_Poll();


static volatile long counter,clock_ticks;
static volatile void interrupt (*oldi8)();
static int running=0;
static int multiplier=0;
volatile int OverRun=0;

void interrupt newi8()
{
static char Lock=0;

if(Lock)
	{
	OverRun=1;
	outp(0x20,0x20);
	return;
	}

Lock=1;

CH_MixAndOutputAudio();

clock_ticks += counter;
if (clock_ticks >= 0x10000L )
	{
	clock_ticks -= 0x10000L;
	oldi8();
	}
else
	outp(0x20,0x20);

Lock=0;
}

void T_Init(long hz, long mult)
{
if(running)
	return;

hz *= mult;
multiplier = (int)mult;

printf("Starting timer at %ld Hz (%dx oversampling)\n",hz,multiplier);

clock_ticks = 0;
counter = 0x1234DD/hz;

oldi8=getvect(8);
setvect(8,newi8);

outp(0x43,0x34);			//  Port[0x43] := 0x34;
outp(0x40,counter % 256);		//  Port[0x40] := counter mod 256;
outp(0x40,counter / 256);		//  Port[0x40] := counter div 256;

running = 1;
}

//
// Shut down the accelerated timer routines
//

void T_Term()
{
if(!running)
	return;

setvect(8,oldi8);
outp(0x43,0x34);			//  Port[0x43] := 0x34;
outp(0x40,0);				//  Port[0x40] := counter mod 256;
outp(0x40,0);				//  Port[0x40] := counter div 256;
}


