# Tornado Mellotron Engine

This is a sample playback engine I wrote in 2006.  It was designed to run directly on top of OpenDOS, accepting MIDI via port 0x330 in the usual manner for DOS, and outputting 16-bit audio via a Crystal CS4236 CODEC on port 0x530.  There was also a COVOX driver for 8-bit parallel port output, but it sounded a lot worse.  The output driver can't be selected at runtime, it had to be compiled in.

The software was build to load in a number of mellotron samples in AKAI format, converted from the Mike Pinder sample library.  I have modified the code slightly to fall back onto WAV format.  It is expected that they be in 22KHz 16-bit mono format, and it may not handle it terribly well if they aren't.

The target machine had 48MB of RAM installed.  Owing to the fact that it was written for my own personal use it did various uncouth things like simply _stealing_ the upper memory instead of asking for it via HIMEM.SYS.  It may crash horribly if HIMEM.SYS is running, and will not run at all under EMM386 since it switches into 32-protected mode to enable Flat-Realmode.  This had an unexpected advantage in that it was possible to debug the software using Borland C++ 2.0 which ran in real mode.

Audio output works by setting the interrupt frequency to a multiple of 22 KHz and banging the audio samples directly to the soundcard instead of using DMA.  This meant there was essentially no latency (notwithstanding the internal FIFO buffer in the soundcard).  It also means that there has to be enough CPU power to sustain this or it will crash hard.  I believe the target machine was some flavour of AMD K6 running at 450MHz.

The TME was used to provide Mellotron samples for the following DOUG the Eagle albums:
*Songs for the Wild-At-Heart (2006)
*The Mythical Creatures Exhibition (2007)
*Pancake Ferret (2008)
*Three Little Pigs - A Tale of Vengeance (2009)
*...And Daryil Answered (2010)
*More Songs About Demons (2011)
*Baklawa Doom (2012)
*Incubi and Succubi (2013)

...after which it was replaced by a Manikin Memotron Rack.
Learning that there are freely-available mellotron samples on the 'net I've decided to release the source code in the hopes that it may amuse or interest people.  If you want to actually set this thing up, have fun - but it's probably a LOT easier to just use a Raspberry Pi and fluidsynth if you need mellotron samples in a box.
