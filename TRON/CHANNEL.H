//
//  Channel management
//

#include "tornado.h"

extern void CH_PlaySound(CHANNEL *ch, long ptr, long len);
extern void CH_StopSound(CHANNEL *ch);
extern CHANNEL *CH_GetChannel();
extern CHANNEL *CH_GetChannelForNote();
extern void CH_MixAndOutputAudio();
