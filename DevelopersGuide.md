# Developer Guide

These notes are for those wanting to delve into the details of the patch and to perhaps make their own changes.

The github Commit highlights the changes: https://github.com/laiudm/laiudm-Teensy4-192k-USB-Audio/commit/485d370d652313c7ec620457280603f0e069c7e8

I have developed the patch with a portable mode IDE install (https://docs.arduino.cc/software/ide-v1/tutorials/PortableIDE/). The portable install ensures changes are completely isolated to that specific install. This has been essential during the patch’s development because the changes are separate from my other ongoing arduino projects. I did not want any development bugs impacting any other project.

## Patch Details

The patch comprises the following main categories of change:

1. Many minor updates to usb descriptors to inform the host of the 192kHz sample rate, and associated buffer size changes.
1. Additions to implement the new usb type in the arduino tools menu (see below)
1. Major changes to usb_audio.h and usb_audio.cpp to implement new buffering requirements introduced by the higher sampling rate. This is where the bulk of the new work has been needed, and is described next.

## New Buffering Requirements

usb_audio.cpp effectively acts as an interface between two "clock domains" - the usb interface, and the Audio Library - and provides buffering between them:

1. The ***usb Interface***. What follows is a simplifed description; there are lots of more detailed guides online. Audio over usb is isochronous. Every 1ms the host issues a data command to hand over any audio samples. With 192,000 kHz sampling rate, this means 192 samples are handed over every 1 ms. In code, this is a call from the usb driver code to usb_audio_receive_callback(). Similarly every 1ms the host polls the device to hand over any receive audio samples. Again 192 samples are handed over. In code this corresponds to a call from the usb driver code to usb_audio_transmit_callback().
1. The ***Audio Library***. The Audio Library is also dealing with 192,000 kHz sample rate, but always deals with samples in buffers of 128 samples each. This is 192,000/128 = 1,500 buffers per second, or one buffer every 667us. In code this corresponds to calls from the Audio Library to AudioInputUSB::update() and AudioOutputUSB::update()

Usb_audio.cpp's task is to map the samples between these two delivery methods and timing regimes. A key issue created with increasing the sample rate to 192,000 kHz is that usb_audio.cpp has to deal with receiving multiple buffers from the Audio Library between polls from the usb interface. This has led to increasing queueing requirements that weren't handled in the original code. Instead I've added a Fifo class to more cleanly implement the buffering in both directions.

## Dealing with a Windows USB Driver Quirk

When a usb device is plugged into a Windows machine, Windows reads the device's descriptors to see if the device has previously been plugged into this usb port. It does this by checking the device's Vendor-id and product-id (as listed in the descriptor) against it's list of "cached" configurations. If it finds a match it simply uses that configuration and ignores all the device's remaining descriptor entries, even if they've changed in the meantime. This means that special care is needed when making programming changes to the device code. If a change has to be made to the device's usb descriptors there are 2 alternatives to overcome the "caching".

1. On any change to the descriptor, update the product-id https://github.com/laiudm/Teensy4-192k-USB-Audio/blob/4a2e75705fc46cd77accc4f144624586345900d8/arduino-1.8.19/hardware/teensy/avr/cores/Teensy4/usb_desc.h#L856. This is the approach I have adopted in developing the patch.
1. Remove the "cached" entry:
   - Using Device Manager, enable View:Show Hidden Devices. Uninstall the greyed-out Teensy drivers.
   - Restart Windows

## Adding the new usb type to the Arduino Tools Menu

https://github.com/K7MDL2/KEITHSDR/wiki/Adding-New-USB-Type-into-the-Arduino-2.x-Tools-Menu gives a good guide to the steps. The same steps appear to work for Arduino 1.8.19 too.

## Test Setup

1. Laptop with Arduino and Audacity installed. Used for firmware development and for testing the usb audio.
1. Teensy 4.1 with Audio Shield fitted
1. usb_audio.ino test sketch for downloading to the Teensy
1. Oscilloscope to check the waveforms on the Left & Right DAC outputs on the Audio Shield

## Test Steps

1. Run the usb_audio.ino test sketch, selecting the “Serial + Serial + 192k Audio” option in tools/usb-type menu
1. Confirm that Windows has picked up the correct sample rate.
   - Right click the speaker icon on the far lower right of the task bar & select "Open Sound Settings"
   - In the output selection choose the Teensy audio channel and click device properties
   - In Device Properties click "additional device properties" at the bottom.
   - In the device's Properties, select the Advanced tab and confirm that the correct sampling frequency is shown
1. Use Audacity to record a few seconds of sound from the device (further instructions below)
1. Use Audacity to play back the recorded sound to the device and confirm correct frequency sounds are played on the Audio Shield DAC outputs.
1. Repear the above tests but this time selecting "Serial + MIDI + Audio" option. The audio sample rate will be the default of 44,100Hz

## Using usb audio with SDTVer050.0 Software

It is relatively easy to patch the SDT code to confirm it works by hooking the USB up to the modeSelectIn and Out Mixers using the same ports as the Microphone and Audio Out. For example:

	AudioOutputUSB           usbAudioOut;
	AudioConnection          patchUSB1Out(modeSelectOutL, 0, usbAudioOut, 0);
	AudioConnection          patchUSB2Out(modeSelectOutR, 0, usbAudioOut, 1);

	AudioInputUSB            usbAudioIn;
	AudioConnection          patchUSB1In(usbAudioIn, 0, modeSelectInExL, 0);
	AudioConnection          patchUSB2In(usbAudioIn, 1, modeSelectInExR, 0);


Extra work will be required to integrate it properly e.g. 
- allow the operator to control between mic input and usb audio input
- set usb audio out volume level independently of the speaker volume
- CAT control to control transmit

Note that the T41 hardware provides significant analogue gain after the DAC output. This means the digial audio level is very low, and needs to be digitally amplified before being connected to the usb audio. 

As a simple hack I kept the speaker disconnected and changed the following lines in Process.cpp:

    } else if (mute == 0) {
      arm_scale_f32(float_buffer_L, 100.0 * DF * VolumeToAmplification(audioVolume), float_buffer_L, BUFFER_SIZE * N_BLOCKS);
      arm_scale_f32(float_buffer_R, 100.0 * DF * VolumeToAmplification(audioVolume), float_buffer_R, BUFFER_SIZE * N_BLOCKS);
    }
	
## Possible Linux Issues


There may be issues with the library's feedback accumulator algorithm on linux, although initial testing by John Melton G0ORX is positive. See comments by jonr in this posting: https://forum.pjrc.com/index.php?threads/usb-audio-samplerates-added.67749/


Since it works fine on Windows I have not touched the algorithm itself, just its initialisation value (https://github.com/laiudm/laiudm-Teensy4-192k-USB-Audio/blob/485d370d652313c7ec620457280603f0e069c7e8/arduino-1.8.19/hardware/teensy/avr/cores/Teensy4/usb_audio.cpp#L97).



## Audacity Hints

From Audio Setup/Playback Device and Audio Setup/Recording Device select the Teensy audio name.

From Audio Setup/Audio Settings select the sampling rate corresponding to the sample rate that the Teensy is currently configured for.

Click Audio Setup/Rescan Audio Devices whenever you've unplugged the Teensy and plugged it back in.



