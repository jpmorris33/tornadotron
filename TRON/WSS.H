// Define some important WSS i/o ports:

#define CODEC_ADDR (wss_port+4)
#define CODEC_DATA (wss_port+5)
#define CODEC_STATUS (wss_port+6)
#define CODEC_PIO (wss_port+7)

// Definitions for CODEC_ADDR register
// Bits 0-3. Select an internal register to read/write
#define LEFT_INPUT      0x00            /* Left input control register */
#define RIGHT_INPUT     0x01            /* RIght input control register */
#define GF1_LEFT_INPUT  0x02            /* Left Aux #1 input control */
#define GF1_RIGHT_INPUT 0x03            /* Right Aux #1 input control */
#define CD_LEFT_INPUT   0x04            /* Left Aux #2 input control */
#define CD_RIGHT_INPUT  0x05            /* Right Aux #2 input control */
#define LEFT_OUTPUT     0x06            /* Left output control */
#define RIGHT_OUTPUT    0x07            /* Right output control */
#define PLAYBK_FORMAT   0x08            /* Clock and data format */
#define IFACE_CTRL      0x09            /* Interface control */
#define PIN_CTRL        0x0a            /* Pin control */
#define TEST_INIT       0x0b            /* Test and initialization */
#define MISC_INFO       0x0c            /* Miscellaneaous information */
#define LOOPBACK        0x0d            /* Digital Mix */
#define PLY_UPR_CNT     0x0e            /* Playback Upper Base Count */
#define PLY_LWR_CNT     0x0f            /* Playback Lower Base Count */

// Definitions for CODEC_ADDR register
#define CODEC_INIT      0x80            /* CODEC is initializing */
#define CODEC_MCE       0x40            /* Mode change enable */
#define CODEC_TRD       0x20            /* Transfer Request Disable */
// bits 3-0 are indirect register address (0-15)

// Definitions for CODEC_STATUS register
#define CODEC_CUL       0x80            /* Capture data upper/lower byte */
#define CODEC_CLR       0x40            /* Capture left/right sample */
#define CODDEC_CRDY     0x20            /* Capture data read */
#define CODEC_SOUR      0x10            /* Playback over/under run error */
#define CODEC_PUL       0x08            /* Playback upper/lower byte */
#define CODEC_PLR       0x04            /* Playback left/right sample */
#define CODEC_PRDY      0x02            /* Playback data register read */
#define CODEC_INT       0x01            /* interrupt status */

/* Definitions for Left output level register */
#define MUTE_OUTPUT     0x80            /* Mute this output source */
// bits 5-0 are left output attenuation select (0-63)
// bits 5-0 are right output attenuation select (0-63)

// Definitions for clock and data format register
#define BIT_16_LINEAR   0x40    /* 16 bit twos complement data */
#define BIT_8_LINEAR    0x00    /* 8 bit unsigned data */
#define TYPE_STEREO     0x10    /* stero mode */
// Bits 3-1 define frequency divisor
#define XTAL1           0x00    /* 24.576 crystal */
#define XTAL2           0x01    /* 16.9344 crystal */

// Definitions for interface control register
#define CAPTURE_PIO     0x80    /* Capture PIO enable */
#define PLAYBACK_PIO    0x40    /* Playback PIO enable */
#define AUTOCALIB       0x08    /* auto calibrate */
#define SINGLE_DMA      0x04    /* Use single DMA channel */
#define PLAYBACK_ENABLE 0x01    /* playback enable */

// Definitions for Pin control register
#define IRQ_ENABLE      0x02    /* interrupt enable */
#define XCTL1           0x40    /* external control #1 */
#define XCTL0           0x80    /* external control #0 */

#define CALIB_IN_PROGRESS 0x20  /* auto calibrate in progress */

#define WSSOUT(i, d)    outportb(CODEC_ADDR, i); \
			outportb(CODEC_DATA, d);
