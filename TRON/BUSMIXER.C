//
//  Bus channel mixer
//

#define USE_FADER


#include <stdio.h>
#include <stdlib.h>
#include "tornado.h"
#include "flatsys.h"

extern int Bus_Output(int wss_port, unsigned char *p);
extern int Bus_Ready(int wss_port);

short CH_GetWord(CHANNEL *ch);

extern int OverRun;

unsigned char *block=NULL;
static volatile int lock=0;

void CH_MixAndOutputAudio()
{
static long mixword;
static short outword;
static int ctr,num;
static char *outptr=(char *)&outword;
static int CDIV=CHANNELS/2;	//  This gives us less headroom but is louder

if(lock)
	{
	OverRun=1;
	return;
	}
lock=1;

if(!Bus_Ready(bus.port))
	{
	// Doesn't want any
	lock=0;
	return;
	}

mixword=0;

// Now mix the channels together
num=0;
for(ctr=0;ctr<CHANNELS;ctr++)
	{
	outword=CH_GetWord(&channel[ctr]);
	mixword+=outword/CDIV; // This may clip
	}

// Saturate it to a 16-bit WORD
if(mixword>32767)
	mixword=32767;
if(mixword<-32768)
	mixword=-32768;

// Shift it down (AGC/compression)
//while(mixword > 32767) mixword/=2;
/*
if(num)
	mixword /= num;
*/


outword=(short)mixword;

// Output the bytes

//Bus_Output(bus.port,outptr);
//Bus_Output(bus.port,outptr);
//Bus_Output(bus.port,outptr);
//Bus_Output(bus.port,outptr);


/*
 *  we're now syncronous with the soundcard, running
    at say twice the sample rate, and only advancing the counts for the
    channels when the soundcard says it wants feeding
 */
while(Bus_Output(bus.port,outptr)); // Keep feeding it until its full
lock=0;
}


//
//  Get the data, automatically stop if we run out
//

short CH_GetWord(CHANNEL *ch)
{
short wptr;
static int clock=0;
if(!ch->playing)
	return 0;

clock--;

#ifdef USE_FADER
// If we've just cut a note, fade it out
if(ch->fade != 0)
	{
	if(clock<0)
		{
		ch->fade=ch->fade/2;
		clock=4;
		}

	// If it's approaching zero, kill it
	if(ch->fade<6 && ch->fade>-6)
		{
		// Stop
		ch->fade=0;
		ch->playing=0;
		return 0;
		}
	// return fader value
	return ch->fade;
	}
#endif

wptr = FRM_GetSample(ch->buffer+ch->ptr);

ch->ptr+=2; // skip the sample

if(ch->ptr>=ch->length)
	{
	// run out of tape, take the last sample and fade it
#ifdef USE_FADER
	ch->fade=wptr;
#else
	ch->playing=0;
#endif
	ch->ptr=0;
	}
return wptr;
}

void CH_PlaySound(CHANNEL *ch, long ptr, long len)
{
if(!ch || !ptr || !len)
	return;

ch->playing=0;
ch->ptr=0;
ch->buffer=ptr;
ch->length=len;
ch->playing=1;
ch->fade=0;
}

void CH_StopSound(CHANNEL *ch)
{
#ifdef USE_FADER
if(ch->playing)
	if(!ch->fade)
		ch->fade=CH_GetWord(ch);
	else
		ch->playing=0;
#else
	ch->playing=0;
#endif
}


CHANNEL *CH_GetChannel()
{
int ctr;
CHANNEL *ch;

// Look for empty channels first
for(ctr=0,ch=&channel[0];ctr<CHANNELS;ctr++,ch++)
	if(!ch->playing)
		{
//		printf("Using blank channel %d\n",ctr);
		return ch;
		}

#ifdef USE_FADER
// Now look for faders
for(ctr=0,ch=&channel[0];ctr<CHANNELS;ctr++,ch++)
	if(ch->fade)
		{
//		printf("Using fader channel %d\n",ctr);
		ch->fade=ch->playing=0;
		return ch;
		}
#endif

printf("ran out of channels at %d\n",ctr);
return NULL;
}

CHANNEL *CH_GetChannelForNote(int note)
{
int ctr;
for(ctr=0;ctr<CHANNELS;ctr++)
	if(channel[ctr].playing)
		if(channel[ctr].tape)
			if(channel[ctr].tape->note == note)
				return &channel[ctr];
return NULL;
}