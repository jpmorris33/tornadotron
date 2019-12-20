//
//	MIDI processing: queue management, note processing
//

//#define PRINTK printf
//#define PRINTI printf

#define PRINTX printf

#ifndef PRINTI
#define PRINTI
#endif

#ifndef PRINTK
#define PRINTK
#endif

#ifndef PRINTX
#define PRINTX
#endif


#define SUPPORT_RSB

#include <stdio.h>
#include <string.h>
#include "midi.h"
#include "tornado.h"

#define QUEUESIZE 256
#define TIMEOUT 200

static unsigned char mqueue[QUEUESIZE];
static int wptr=0,rptr=0,count=0;
extern int OverRun;

int MP_ReadData();
void Controller(int CC, int P);

extern StartNote(int note);
extern StopNote(int note);
extern Reset();
extern ProgChange(int prog);

void MQ_Add(unsigned char byte)
{
if(count == QUEUESIZE)
	{
	OverRun=1616;
	return;
	}

mqueue[wptr]=byte;
wptr++;
count++;
if(wptr == QUEUESIZE)
	wptr=0;
}

int MQ_Read()
{
int i;

if(count == 0)
	{
	// Nothing to read
	return -1;
	}

i=mqueue[rptr];
rptr++;
count--;
if(rptr == QUEUESIZE)
	rptr=0;

return i;
}

int MQ_Count()
{
return count;
}

///////////////

void MP_Process()
{
int byte,p1,p2,channel;
static int lastbyte=0;
static int SysEx=0;


byte = MQ_Count();
if(byte<2)
	return; // no.


//printf("%d in queue\n",byte);

byte=MQ_Read();
if(byte<0)
	return;

if((byte & 0x80) == 0)
	{
	if(SysEx)
		return; // Do Nothing
#ifdef SUPPORT_RSB
//	PRINTX("Got RSB\n");
	// data, use the last valid opcode ('running status byte')
	p1=byte;		// What we got is actually P1
	byte=lastbyte;
#else
	PRINTX("RSB - SONAR doesn't do these.. may be a bug\n");

	byte=0x90+MidiChannel; // It's probably a Note On
	p1=byte;

#endif
	}
else
	{
	if(SysEx)
		{
		SysEx=0;
		PRINTK("SysEx OFF\n");
		}

	if(byte < 0xf8)
		{
		// Store this as the last valid opcode
		lastbyte=byte;
		// Now get P1
		p1=MP_ReadData();
		}
	else
		{
		// RT-System opcodes have no parameter and don't affect RSB
		}
	}

if(p1<0)
	{
	PRINTK("Failed to get p1\n");
//	return;
	}

if(p1 & 0x80)
	{
	PRINTK("P1 is bogus (0x%02x)\n",p1);
	}

channel=byte&0xf;

switch((byte & 0x70)>>4)
	{
	case 0:
		// Note off:  p1 = key, p2 = velocity (?)
		p2=MP_ReadData();
		if(channel == MidiChannel)
			{
			StopNote(p1);
			PRINTK("Note off for ch %d (%d,%d)\n",channel,p1,p2);
			}
		else
			{
			PRINTI("IGNORED Note off for ch %d (%d,%d)\n",channel,p1,p2);
			}

		break;

	case 1:
		// Note On:  p1 = key, p2 = velocity.
		// some systems send a 0-velocity note-on
		// instead of note off
		p2=MP_ReadData();

		if(channel == MidiChannel)
			{
			if(p2 < 1)
				StopNote(p1);
			else
				StartNote(p1);
			PRINTK("Note on for ch %d (%d,%d)\n",channel,p1,p2);
			}
		else
			{
			PRINTI("IGNORED Note on for ch %d (%d,%d)\n",channel,p1,p2);
			}


		break;

	case 2:	// Key pressure (don't care)
		p2=MP_ReadData();	// Absorb P2
		break;

	case 3:	// CC
		p2=MP_ReadData();	// Get P2

		if(channel == MidiChannel)
			{
			Controller(p1,p2);
			PRINTK("CC on ch %d\n",byte&0x0f);
			PRINTK("C = 0x%02x, V = 0x%02x\n",p1,p2);
			}
		else
			{
			PRINTI("IGNORED CC for ch %d (%d,%d)\n",channel,p1,p2);
			}

		break;

/*
	case 4:
		if(channel == MidiChannel)
			{
			ProgChange(p1);
			PRINTK("Prog change on ch %d\n",byte&0x0f);
			PRINTK("Prog = 0x%02x\n",p1);
			}
		else
			{
			PRINTI("IGNORED Prog change for ch %d (%d)\n",channel,p1);
			}
		break;
*/

	case 5: // Channel pressure (ignore)
		p2=MP_ReadData();	// Absorb P2
		break;

	case 6: // Pitch bend
		p2=MP_ReadData();	// Absorb P2
		p2<<=7;
		p2+=(p1&0x7f);
		PRINTK("Pitch bend on ch %d\n",byte&0x0f);
		PRINTK("Pitch %d\n",p2&0x3fff);
		break;

	case 7: // SYSTEM
		{
		switch(channel)
			{
			case 0:
				// Enable SYSEX mode
				SysEx=1;
				PRINTK("SYSEX ON\n");
				return;
			case 2:
				// song pos
				p1=MQ_Read();
				p2=MQ_Read();
				PRINTK("Song Pos %d\n",p1+(p2<<7));
				break;
			case 3:
				// Set song
				p1=MQ_Read();
				PRINTK("Set Song %d\n",p1);
				break;

			case 0xf:
				// Reset
				Reset();
				break;

			default:
				// 1,4-E are undefined or unsupported
				// and have no parameters
				break;
			}
		}

	default:
		PRINTK("Unhandled control %d on ch %d\n",(byte&0x70)>>4,byte&0x0f);
		PRINTK("P1 = 0x%02x, P2 = 0x%02x\n",p1,p2);
		break;
	}




}

//
//  Safely read a byte from the datastream
//

int MP_ReadData()
{
int p,c;
c=0;
p=-1;
while(p < 0)
	{
	p=MQ_Read();
	c++;
	if(c>TIMEOUT)
		return -1;

	if(p == 0xfe)
		{
//		printf(".");
		p=-1; // Junk
		}
	if(p == -1)
		M_Poll();
	};

return p;
}


int M_Poll()
{
static int i;

// Look for stuff on the MIDI port
do
	{
	i=M_GetMidi();
	} while(i == 0xfe); // Timer sync: we really hate those

if(i >= 0)
	{
	// Got something, add it to the queue
	MQ_Add(i);
	return 1;
	}

return 0;
}

// Handle Control Changes

void Controller(int CC, int P)
{
switch(CC)
	{
	case 7:
		// Volume
		PRINTK("CC: Set volume %d [not implemented]\n",P);
		break;

	case 80:
		// General Purpose #5 - set patch
		ProgChange(P);
		break;

	case 120:	// All sound off
	case 121:	// Reset all controllers
	case 123:	// All notes off
	case 124:	// Omni off
	case 125:	// Omni on
	case 126:	// Mono
	case 127:	// Poly
		// All of these will cut the sound
		PRINTK("CC: sound off (CC %d, P= %d)\n",CC,P);
		Reset();
		break;
	default:
		PRINTK("CC: unsupported (CC %d, P= %d)\n",CC,P);
		break;
	}
}

