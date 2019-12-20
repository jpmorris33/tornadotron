//
//  Flat memory system
//

extern void FRM_Init(); // Init flat-realmode memory system
extern void FRM_CopyChunk(char far *dest, char far *src, unsigned short len);
extern unsigned short FRM_GetSample(unsigned long ptr);
extern unsigned long FRM_Alloc(unsigned long amount);
extern void FRM_FreeAll();
