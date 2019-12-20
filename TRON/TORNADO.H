//
//  Tornado playback engine V1.20
//
//  (C) 2006 IT-HE Software
//

#ifndef _TORNADO_H_
#define _TORNADO_H_

//#define USE_WSS
#define USE_COVOX

#include <stdio.h>

#define TORNADO_VERSION "V1.20"

#define CHANNELS 8
#define MAXPATCHES 16

typedef struct
	{
	int port;
	char on;
	char running;
	unsigned long sysdata1;
	} CARD;

typedef struct
	{
	short note;
	long length;
	unsigned long ptr;
	} TAPE;

typedef struct
	{
	int tapes;
	TAPE *tape;
	} TAPEBANK;

typedef struct
	{
	long buffer;
	long length;
	long ptr;
	char playing;
	short fade;
	TAPE *tape;
	} CHANNEL;

typedef char *PATCHSET;

extern CHANNEL channel[];
extern PATCHSET patch[];
extern CARD bus;
extern int MidiChannel;

#endif