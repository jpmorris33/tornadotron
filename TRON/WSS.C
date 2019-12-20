//
//  WSS I/O driver
//


#include <stdlib.h>
#include <dos.h>
#include "tornado.h"
#include "wss.h"

#ifdef USE_WSS

typedef unsigned char UBYTE;
int WSS_Detect(int wss_port);
int WSS_GetRate(long frequency);
int WSS_SetDataFormat(int wss_port, int div);
void WSS_UnMute(int wss_port);

extern int OverRun;

static volatile unsigned char byte=0;

static void wss_wait(int _sound_port)
{
   unsigned int i = 0xFFFF;

   /* Wait for INIT bit to clear */
   while ((inportb(_sound_port + 4) & CODEC_INIT) || (i-- > 0))
      ;

if(i<1)
	printf("WSS timeout\n");
}

struct codec_rate_struct
{
   long freq;
   int divider;
};


/* List of supported rates */
static struct codec_rate_struct codec_rates[] =
{
   { 5512,  0x01 },
   { 6620,  0x0F },
   { 8000,  0x00 },
   { 9600,  0x0E },
   { 11025, 0x03 },
   { 16000, 0x02 },
   { 18900, 0x05 },
   { 22050, 0x07 },
   { 27420, 0x04 },
   { 32000, 0x06 },
   { 33075, 0x0D },
   { 37800, 0x09 },
   { 44100, 0x0B },
   { 48000, 0x0C },
   { 54857, 0x08 },
   { 0, 0 },

   /* These last two values are ILLEGAL, { 64000, 0x0A }
      but they may work on some devices. Check 'em out! */
};


//
//  See if there is a WSS codec
//

int WSS_Detect(int wss_port)
{
unsigned char b,c;
b=inportb (CODEC_ADDR);
if(b & 0x80)
	{
	printf("WSSdetect got (%x)\n",b);
	return 0;
	}

outportb(CODEC_ADDR,MISC_INFO); // Select MISC register
b=inportb(CODEC_DATA);		// Get contents
outportb(CODEC_DATA,0);		// Try to erase
c=inportb(CODEC_DATA);		// Get new contents
if(b != c)
	return 0;

return 1;
}

int WSS_GetRate(long frequency)
{
int i,use;
long diff,bestdiff;

bestdiff = 10000000;
use=0;

for (i=0; codec_rates[i].freq; i++)
	{
	diff = abs(frequency - codec_rates[i].freq);
	if (diff < bestdiff)
		{
		bestdiff = diff;
		use = i;
		}
	}

printf("WSS init at %ld hz using divider %x\n",codec_rates[use].freq,codec_rates[use].divider);
return codec_rates[use].divider;
}

int WSS_SetDataFormat(int wss_port, int div)
{
unsigned short i;

   /* Enable MCE */
   outportb(CODEC_ADDR, CODEC_MCE | IFACE_CTRL);
   wss_wait(wss_port);

   /* Enable full ACAL */
   outportb(CODEC_DATA, 0x18);
   wss_wait(wss_port);

   /* Disable MCE */
   outportb(CODEC_ADDR, TEST_INIT); // errstat

   /* Wait for ACAL to finish */
   i = 0xFFFF;

   while ((inportb(CODEC_DATA) & CALIB_IN_PROGRESS) && (i-- > 0))
      ;

   if (i < 1)
      return 0;

   /* Enter MCE */
   outportb(CODEC_ADDR, CODEC_MCE | PLAYBK_FORMAT);
   wss_wait(wss_port);

   /* Set playback format */
//   i = div | 0x50;  // 10 = stereo 40 = 16bit
   i = div | 0x40;    // MONO, please
//   i = div | 0x10;  // 10 = stereo 40 = 16bit
   outportb(CODEC_DATA, i);
   wss_wait(wss_port);

   outportb(CODEC_ADDR,CODEC_MCE |IFACE_CTRL);
   wss_wait(wss_port);
   outportb(CODEC_DATA, PLAYBACK_PIO|PLAYBACK_ENABLE);
   wss_wait(wss_port);
//   printf("got WSS status = %x\n",b);

// JM: This was uncommented before.. guess we left the interrupt bit floating
   outportb(CODEC_ADDR, 0);
   outportb(CODEC_STATUS, 0);
   outportb(CODEC_ADDR, PIN_CTRL);
   wss_wait(wss_port);
   outportb(CODEC_DATA, 0);        // DISABLE interrupts in Pin Control reg.
//   outportb(CODEC_DATA, 0x2);        // Enable interrupts in Pin Control reg.

return 1;
}

void WSS_UnMute(int wss_port)
{
// Unmute all these
WSSOUT(LEFT_INPUT, 0);
WSSOUT(RIGHT_INPUT, 0);
WSSOUT(GF1_LEFT_INPUT, 0);
WSSOUT(GF1_RIGHT_INPUT, 0);
WSSOUT(CD_LEFT_INPUT, 0);
WSSOUT(CD_RIGHT_INPUT, 0);
WSSOUT(LEFT_OUTPUT, 0);
WSSOUT(RIGHT_OUTPUT, 0);
}

///////////////////////////////////////////////////////////
//  This is the actual driver interface used by Tornado  //
///////////////////////////////////////////////////////////


int Detect_Bus()
{
return WSS_Detect(bus.port);
}

int Init_Bus(long freq, long *multiplier)
{
bus.sysdata1 = WSS_GetRate(freq);
if(!WSS_SetDataFormat(bus.port,bus.sysdata1))
	{
	bus.on=0;  // Argh
	return 0;
	}
WSS_UnMute(bus.port);
*multiplier = 6;	// Six times oversample is enough
return 1;
}

//
//  Send the sample data out
//

int Bus_Output(int wss_port, unsigned char *p)
{
byte=inportb(CODEC_STATUS);

if(!(byte & CODEC_PRDY))
	{
	// Do Not Want
//	OverRun=-3;
	return 0;
	}


if(byte & CODEC_PLR)
	{
	// Left
	if(byte & CODEC_PUL)
		outportb(CODEC_PIO,p[1]);	// Want upper byte
	else
		outportb(CODEC_PIO,p[0]);	// Want lower byte
	}
else

	{
	// Right
	if(byte & CODEC_PUL)
		outportb(CODEC_PIO,p[3]);
	else
		outportb(CODEC_PIO,p[2]);
	}

return 1;
}

int Bus_Ready(int wss_port)
{
byte=inportb(CODEC_STATUS);
if(byte & CODEC_PRDY)
	return 1;
return 0;
}

#endif