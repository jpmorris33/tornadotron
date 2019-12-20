// Queue API
extern void MQ_Add(unsigned char byte);
extern int MQ_Read();

// processing system
extern int M_Poll();
extern void MP_Process();

// Device Driver API
extern int M_Init(int port, int irq);
extern void M_Term();
extern int M_GetMidi(); // Get raw MIDI byte
