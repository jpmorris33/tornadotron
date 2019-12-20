//
//  'Allocate' a memory block - Do what thou wilt shall be the whole of the Law
//

#define STARTPTR 0x200000L	// start from 2MB

static unsigned long ptr=STARTPTR;

unsigned long FRM_Alloc(unsigned long bytes)
{
unsigned long newptr=ptr;
ptr += bytes;

ptr += 1000000; //HACK ME
//printf("Hacked memory allocater, +1MB\n");

//printf("starting from 0x%lx, allocating 0x%lx bytes (%ld) = 0x%lx\n",newptr,bytes,bytes,ptr);

return newptr;
}

void FRM_FreeAll()
{
ptr=STARTPTR;
}