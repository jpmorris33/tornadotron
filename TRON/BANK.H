extern int TB_LoadTape(TAPE *t, int note, char *filename);
extern CHANNEL *TB_PlayTape(TAPE *t);
extern TAPE *TB_GetTapeForNote(int note, TAPEBANK *bank);
extern TAPEBANK *TB_InitBank(int tapes);
extern void TB_FreeBank(TAPEBANK *ptr);
extern TAPEBANK *TB_LoadPatch(int pat);
extern int TB_InitPatch(int pat, char *dir);

#define NOTE_AKAI -1
