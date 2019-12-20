/*
 *	MPU401-based MIDI I/O
 */

#define SB_TIMEOUT 0x200

#include <stdio.h>
#include <dos.h>
#include "midi.h"

static int MidiPort = 0;
extern int OverRun,ORC;

FILE *fp;

// Stop Midi handler
void M_Term()
{
if(!MidiPort)
	return;
MidiPort=0;
}

int M_Init(int port, int irq)
{
int ctr;

if(MidiPort)
	return 0;

// MPU401 reset
if(inportb(port+1) & 0x40)
	{
//	printf("MPU401: busy or not present\n");
	return 0;
	}

outportb(port+1,0xff); // Reset

for(ctr=0;ctr<100;ctr++)
	{
	if(inportb(port+1) & 0x80)
		{
		// Busy
		delay(1);
		continue;
		}

	if(inportb(port) == 0xfe)
		{
		ctr=0;	// Success!
		break;
		}
	delay(1);
	}

if(ctr>2)
	{
//	printf("MPU401: busy or not present (2)\n");
	return 0;
	}

// Enter UART mode
outportb(port+1,0x3f);

for(ctr=0;ctr<100;ctr++)
	{
	if(inportb(port+1) & 0x80)
		{
		// Busy
		delay(1);
		continue;
		}

	if(inportb(port) == 0xfe)
		{
		ctr=0;	// Success!
		break;
		}
	delay(1);
	}

if(ctr>2)
	{
//	printf("MPU401: busy or not present (3)\n");
	return 0;
	}

MidiPort = port;
return 1;
}


//
//	Get raw MIDI byte if available
//

int M_GetMidi()
{
int i;

if(!MidiPort)
	return -2;

if (!(inportb(1 + MidiPort) & 0x80))
	{
	i=(int)inportb(MidiPort);
	return i;
	}

return -1;
}

