//
//  Tornado playback engine V1.10
//
//  (C) 2006 IT-HE Software
//

//#define MUTE

#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include "strlib.h"

#include "tornado.h"
#include "timer.h"
#include "flatsys.h"
#include "channel.h"
#include "bank.h"
#include "midi.h"

long ClockMultiplier=1;
char Quit=0;
int MidiChannel=0;	// default to channel 1
int Patch=1;		// default to patch 1
extern int OverRun;

static long freq=44100;
CARD bus;
CHANNEL channel[CHANNELS];
TAPEBANK *tapebank=NULL;
PATCHSET patch[MAXPATCHES];
int patches=0;

int ReadConfig(char *filename);
void Stop();
void Reset();

extern int Detect_Bus();
extern int Init_Bus(long freq, long *multiplier);
extern FILE *B_Open(char *filename);

int main(int argc, char *argv[])
{
int ret,ctr;
int k;

memset(&bus,0,sizeof(bus));
memset(&channel,0,sizeof(channel));
memset(&patch,0,sizeof(patch));

clrscr();
printf("Tornado Playback Engine %s\n",TORNADO_VERSION);
printf("Copyright (C)2006 IT-HE Software\n");
printf("================================\n");
printf("\n");

printf("Initialising memory system: ");
FRM_Init();
printf("Do what thou wilt shall be the whole of the Law\n");


printf("Reading config file: ");

if(!ReadConfig("tornado.ini"))
	{
	printf("tornado.ini NOT FOUND\n");
	Stop();
	}

printf("Initialising audio bus: ");

#ifndef MUTE
if(Init_Bus(freq,&ClockMultiplier))
	{
	bus.running=1;
	printf("OK\n");
	}
else
	{
	printf("FAILED\n\n\n");
	printf("No audio bus available!\n");
	Stop();
	}
printf("\n");
#endif

/*
ret=0;
for(k=0;k<TAPEBANKS;k++)
	for(ctr=0;ctr<tapebank[k]->tapes;ctr++)
		if(tapebank[k]->tape)
			if(tapebank[k]->tape[ctr].ptr)
				ret++;
*/

printf("%d patches setup\n",patches);
if(!patches)
	{
	printf("No valid patches defined.\n");
	Stop();
	}

// Is the default patch valid?
if(!patch[Patch])
	{
	// No, find one then
	for(ctr=0;ctr<MAXPATCHES;ctr++)
		if(patch[ctr])
			{
			Patch=ctr;
			break;
			}
	}

// Load the patch
printf("Loading default patch\n");
tapebank=TB_LoadPatch(Patch);
if(!tapebank)
	{
	// Failed to load patch!
	printf("Error loading default patch!\n");
	Stop();
	}

printf("Initialising audio kernel\n");
#ifndef MUTE
T_Init(freq,ClockMultiplier);
#endif

printf("Hit ESC to exit\n");
k=0;
ret=0;
do
	{
	// Get everything we can
	M_Poll();

	ret++;
	if(ret >= 10)
		{
		if(kbhit())
			{
			k=getch();
			if(k == ' ')
				Reset();
			}
		ret=0;
		}

	if(OverRun)
		{
		printf("			Overrun #%d %d (%x) detected\n",OverRun,OverRun);
		OverRun=0;
		}

	MP_Process();
	} while(k != 'q' && k != 27);
T_Term();
M_Term();

return 0;
}

int ReadConfig(char *filename)
{
FILE *fp;
char buffer[256];
char *word,*p1;
long f,b;
int note;

fp = fopen(filename,"r");
if(!fp)
	return 0;

printf("OK\n");

do
	{
	buffer[0]=0;
	fgets(buffer,1023,fp);
	strdeck(buffer,0x0a);
	strdeck(buffer,0x0d);
	if(buffer[0] == '#' || buffer[0] == ';')
		continue;
	word=strfirst(buffer);

	if(!stricmp(word,"FREQUENCY"))
		{
		p1=strgetword(buffer,2);
		f=atol(p1);
		if(!f)
			{
			printf("  Ignoring invalid sample rate '%s'\n",p1);
			continue;
			}
		printf("  Setting frequency to %ld Hz\n",f);
		freq=f;
		}
/*
	if(!stricmp(word,"MULTIPLIER"))
		{
		p1=strgetword(buffer,2);
		f=atol(p1);
		if(f<1)
			{
			printf("  Ignoring invalid sample rate '%s'\n",p1);
			continue;
			}
		printf("  Setting IRQ multiplier to %ldx (%ld hz)\n",f,f*freq);
		ClockMultiplier=f;
		}
*/
	if(!stricmp(word,"DSP"))
		{
		p1=strgetword(buffer,2);
		sscanf(p1,"%X",&f);
		printf("  Detecting audio bus on 0x%x.. ",f);
		bus.port=f;
		if(!Detect_Bus(&bus))
			{
			printf("FAILED\n");
			continue;
			}
		else
			{
			printf("OK\n");
			bus.on=1;
			}
		}

	if(!stricmp(word,"MIDI"))
		{
		p1=strgetword(buffer,2);
		f=b=0;
		sscanf(p1,"%X",&f);
		p1=strgetword(buffer,3);
		sscanf(p1,"%d",&b);
		printf("  Using MIDI port on 0x%lx ",f);
		if(M_Init(f,b))
			{
			printf(": OK\n");
			}
		else
			{
			// Try again.
			if(!M_Init(f,b))
				{
				printf(": FAILED\n");
				Stop("MIDI port not detected\n");
				exit(1);
				}
			printf(": PASS\n");
			}
		}

	if(!stricmp(word,"PATCH"))
		{
		p1=strgetword(buffer,2);	// Bank number
		b=atol(p1);
		if(b <0 || b >= MAXPATCHES)
			{
			printf("Patch number %ld out of range: (0-%d)\n",b,MAXPATCHES-1);
			continue;
			}

		if(patch[b])
			{
			printf("Patch number %ld already defined\n",b);
			continue;
			}
		p1=strgetword(buffer,3);	// Path
		if(!TB_InitPatch((int)b,p1))
			{
			printf("Failed to set up patch %ld\n",b);
			printf("No .a1s files found in '%s'\n",p1);
			}
		else
			patches++;

		printf("Created patch %ld OK\n",b);
		}


	if(!stricmp(word,"MIDICHANNEL"))
		{
		p1=strgetword(buffer,2);	// channel number
		note=atoi(p1);
		if(note<1 || note>16)
			{
			printf("Invalid MIDI channel %d, defaulting to %d\n",note,MidiChannel+1);
			continue;
			}
		MidiChannel=note-1;
		printf("Using MIDI channel %d\n",MidiChannel+1);
		}

	} while(!feof(fp));

fclose(fp);
return 1;
}

void Stop()
{
printf("\n");
printf("Exiting..\n");

T_Term();	// Timer
M_Term();	// Midi processor
exit(1);
}


void StartNote(int key)
{
TAPE *tptr;

if(CH_GetChannelForNote(key))
	return;

tptr=TB_GetTapeForNote(key,tapebank);
TB_PlayTape(tptr);
}

void StopNote(int key)
{
CHANNEL *ch;

ch=CH_GetChannelForNote(key);
if(ch)
	{
	// Stop
//	ch->playing=0;
	CH_StopSound(ch);
	}
}

void Reset()
{
int ctr;
// Kill all channels
for(ctr=0;ctr<CHANNELS;ctr++)
	channel[ctr].playing=0;
}


void ProgChange(int prog)
{
if(prog>=MAXPATCHES || prog < 0 || !patch[prog])
	return; // Not present

if(prog == Patch)
	return; // "Done it!"

// Shut down the system for a bit
//T_Term();

FRM_FreeAll(); // Free all the samples
TB_FreeBank(tapebank);
tapebank=TB_LoadPatch(prog);
if(!tapebank)
	{
	// Ouch!
	tapebank=TB_LoadPatch(Patch);
	if(!tapebank)
		{
		// Oh ____
		printf("Unrecoverable error loading bank '%d'\n",prog);
		Stop();
		}
	}

// Ok, we're done
Patch=prog;

/*
#ifndef MUTE
T_Init(freq,ClockMultiplier);
#endif
*/
}