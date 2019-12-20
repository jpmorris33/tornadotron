//
//  Tape Bank management
//

#define BUFLEN 1024

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <dos.h>
#include <dir.h>

#include "tornado.h"
#include "channel.h"
#include "flatsys.h"
#include "bank.h"

static int LoadWav(TAPE *t, int note, char *filename);
static int LoadAkai(TAPE *t, char *filename);
void Beep(int freq);


int TB_LoadTape(TAPE *t, int note, char *filename)
{
if(note == NOTE_AKAI)
	return LoadAkai(t,filename);

return LoadWav(t,note,filename);
}


int LoadWav(TAPE *t, int note, char *filename)
{
FILE *fp;
unsigned long ptr;
char buffer[BUFLEN];
int len;

fp=fopen(filename,"rb");
if(!fp)
	return 0;

t->note=note;
fseek(fp,0L,SEEK_END);
t->length=ftell(fp);
//fseek(fp,0L,SEEK_SET);
fseek(fp,32L,SEEK_SET);
printf("WAV LOADER HACK: skipped 32 bytes\n");
t->ptr = FRM_Alloc(t->length);

ptr=t->ptr;

do
	{
	len=fread(buffer,1,BUFLEN,fp);
	if(len<1)
		break;

	FRM_CopyChunk((char far *)ptr,(char far *)MK_FP(_DS,buffer),len);  // Copy the chunk into FRMspace
	ptr += len;
	} while(len > 0);
fclose(fp);

return 1;
}

int LoadAkai(TAPE *t, char *filename)
{
FILE *fp;
unsigned long ptr;
char buffer[BUFLEN];
char header[150];
int len;

fp=fopen(filename,"rb");
if(!fp)
	return 0;

fread(header,1,150,fp);

if(header[0] != 3)
	{
	fclose(fp);
	return 0;	// Wrong
	}

memcpy(&t->length,&header[26],4); // Get length (samples)
t->length <<= 1; // Multiply by 2 to get bytes

t->note=header[2];	// Get MIDI note number

//fseek(fp,150L,SEEK_END);
t->ptr = FRM_Alloc(t->length);

ptr=t->ptr;

do
	{
	len=fread(buffer,1,BUFLEN,fp);
	if(len<1)
		break;

	FRM_CopyChunk((char far *)ptr,(char far *)MK_FP(_DS,buffer),len);  // Copy the chunk into FRMspace
	ptr += len;
	} while(len > 0);
fclose(fp);

return 1;
}


CHANNEL *TB_PlayTape(TAPE *t)
{
CHANNEL *ch;

if(!t)
	return NULL;

ch=CH_GetChannel();
if(!ch)
	return NULL;

ch->playing=0;
ch->ptr=0;
ch->fade=0;
ch->buffer=t->ptr;
ch->length=t->length;
ch->playing=1;
ch->tape = t; // Keep track of the tape so we can release the note

return ch;
}

TAPE *TB_GetTapeForNote(int note, TAPEBANK *bank)
{
int ctr=0;

//printf("Find note '%d'\n",note);

if(!bank || !bank->tape)
	{
//	printf("Bad bank!\n");
	return NULL;
	}

for(ctr=0;ctr<bank->tapes;ctr++)
	if(bank->tape[ctr].ptr)
		{
//		printf("tape %d is note %d\n",ctr,bank->tape[ctr].note);
		if(bank->tape[ctr].note == note)
			return &bank->tape[ctr];
		}

//printf("Can't find note '%d'\n",note);

return NULL;
}

TAPEBANK *TB_InitBank(int tapes)
{
TAPEBANK *ptr=NULL;

ptr=(TAPEBANK *)calloc(1,sizeof(TAPEBANK));
if(!ptr)
	return NULL;

ptr->tapes=tapes;
ptr->tape=(TAPE *)calloc(tapes,sizeof(TAPE));
if(!ptr->tape)
	return NULL;

return ptr;
}

void TB_FreeBank(TAPEBANK *ptr)
{
int ctr;
// First, cut all notes and remove all references to the tapes
for(ctr=0;ctr<CHANNELS;ctr++)
	{
	channel[ctr].playing=0;
	channel[ctr].tape=NULL;
	}
free(ptr->tape);
free(ptr);
}

//
// Load in a patch
//

TAPEBANK *TB_LoadPatch(int pat)
{
struct ffblk ffblk;
int i;
int qty=0;
TAPEBANK *ptr;
char path[256];

if(!patch[pat])
	return NULL; // No such patch

strcpy(path,patch[pat]);
strcat(path,"*.a1s");

// First, how many files are there?
i=findfirst(path,&ffblk,0);
if(i<0)
	return NULL;	// shouldn't happen
qty++;
while(!findnext(&ffblk)) qty++;

// Now we know how many there are, create the bank
ptr=TB_InitBank(qty);
if(!ptr)
	{
	printf("Out of memory allocating bank\n");
	return NULL;
	}

Beep(1000);

// Now populate the bank
qty=0;
i=findfirst(path,&ffblk,0);
if(i<0)
	{
	printf("Urk\n");	// REALLY shouldn't happen
	TB_FreeBank(ptr);
	Beep(250);
	return NULL;
	}
while(!i)
	{
	strcpy(path,patch[pat]);
	strcat(path,ffblk.ff_name);
	printf("Load tape %s : ",path);
	i=LoadAkai(&ptr->tape[qty++],path);
	if(i)
		printf("OK\n");
	else
		printf("Failed\n");

	i=findnext(&ffblk);
	}

Beep(500);

return ptr;
}


//
// Initialise a patch by checking the given directory
//

int TB_InitPatch(int pat, char *dir)
{
int i;
char path[256];
struct ffblk ffblk;

strcpy(path,dir);
strcat(path,"*.a1s");

i=findfirst(path,&ffblk,0);
if(i<0)
	return 0;

patch[pat] = (char *)calloc(1,strlen(dir)+1);
if(!patch[pat])
	return 0;
strcpy(patch[pat],dir);
return 1;
}

void Beep(int freq)
{
sound(freq);
delay(100);
nosound();
}