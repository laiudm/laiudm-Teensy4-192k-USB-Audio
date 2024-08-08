# Teensyduino 192ksps USB Audio Patches, Version 0.1

## What the patch does & why

While experimenting with my build of the T41-EP Software controlled ham radio (see https://www.amazon.co.uk/Digital-Signal-Processing-Software-Defined/dp/B0D25FV48C and https://groups.io/g/SoftwareControlledHamRadio ) I’ve wanted to add usb audio support, as is available for most modern commercial transceivers. The Teensy 4.1 (https://www.pjrc.com/store/teensy41.html) development environment provides usb audio, but only at 44,100 Hz sample rate. The audio sample rate and usb sample rate must be the same, and unfortunately the T41 uses a 192,000 Hz audio sample rate that renders the usb audio inoperable.

The code presented here is my patch to the development environment to support 192,000 Hz usb audio. The 192,000 Hz option is activated in the IDE’s tools/usb-type menu optionSerial + Serial + 192k Audio". This option also activates two serial usb ports - I use one for debugging and the other port will in future be for the radio’s CAT control. It has been tested with a WIndows 10 laptop and Ubuntu 24.04 has been confirmed to work with wsjt-x, a common digital-mode application.

The now functional usb audio could in future also be used for I/Q receive and transmit modes on the T41-EP over a full 192kHz frequency range.

## Versions supported

- Arduino 1.8.19 IDE
- Arduino 2.3.2 IDE
- Teensyduino Version 1.59
- Windows 10 usb audio and development hosting. 
- Linux Ubuntu 24.04 It should work with other Linux versions but has not yet been tested.

## Files Provided

The arduino-1.8.19 directory is an upload of those Teensyduino Version 1.59 files that are altered by the 192ksps changes (see https://www.pjrc.com/teensy/td_download.html)

usb_audio.ino is a simple sketch for testing the usb audio functionality.

## Installation Steps

1. Only install in an environment with the correct versions of Arduino and Teensyduino.
1. Ensure the IDE is shut down.
1. Installation requires copying the 7 files under the arduino-1.8.19 directory in this github into their corresponding locations in your installation.
1. Find the Teensyduino files. 
   - For Windows/Arduino 1.8.19 standard install the location will be C:\Program Files (x86)\Arduino\
   - For Windows/Arduino 1.8.19 Portable installation (https://docs.arduino.cc/software/ide-v1/tutorials/PortableIDE/) the location will be <your root location>\arduino-1.8.19\
   - For Windows/Arduino 2.3.2 standard install the location will be C:\Users\<your login name>\AppData\Local\Arduino15\packages\teensy\hardware\avr\1.59.0
   - For Linux/Arduino 2.3.2 standard install the location will be $HOME/.arduino15/packages/teensy/hardware/avr/1.59.0
1. You might wish to first make a backup zip/tar file of the files you are about to overwrite so you could easily restore them if necessary.
1. Copy arduino-1.8.19/hardware/teensy/avr/boards.local.txt into the hardware/teensy/avr/ directory. The end result will look like:
	![](/images/avrsubdir.PNG)
1. Copy the 6 files at arduino-1.8.19/hardware/teensy/avr/cores/Teensy4 into your installation’s 
	- (Windows) ...\cores\teensy4 directory, 
	- (Linux) $HOME/.arduino15/packages/teensy/hardware/avr/1.59.0/cores/teensy4
	overwriting all existing copies of the files. The end result (date ordered) will look like (only the highlighted files are updated):
	![](/images/teensy4subdir.PNG)
1. For Arduino 2, the IDE caches some content so we need to delete a folder. Deleting the leveldb folder in
	- (Windows) %AppData%\Roaming\arduino-ide\Local Storage\
	- (Linux) $HOME/.config/arduino-ide/'Local Storage'
	is all that's needed. Thanks to https://github.com/K7MDL2/KEITHSDR/wiki/Adding-New-USB-Type-into-the-Arduino-2.x-Tools-Menu for the tip.

## Testing the Patch

1. Load the supplied usb_audio test sketch into the IDE
1. In the IDE select “Serial + Serial + 192k Audio” option in tools/usb-type menu
1. Download the sketch to the Teensy 4.1 board (with an Audio Shield fitted)
1. Confirm that Windows informs you that a new driver is being installed. This message will only appear the first time a sketch with the “Serial + Serial + 192k Audio” option is run.
1. [The Developer Guide](DevelopersGuide.md) lists further tests to confirm correct operation.

## Risks of Using the patch

- To date the patch has had limited testing on just a few environments.
- Any error made in installing the patch may render the development environment inoperable.
- The patch will be overwritten by an update to the Teensyduino version, most likely rendering this patch inoperable.
- It is possible that the patch may adversely affect other projects that use this development environment. This risk can be mitigated by running the IDE in portable mode:(https://docs.arduino.cc/software/ide-v1/tutorials/PortableIDE/)

## Patch Versions

V0.1 First version made generally available

## Acknowledgements

This patch is based on and leverages the following:

- https://github.com/PaulStoffregen/cores/pull/587
- https://github.com/K7MDL2/KEITHSDR/wiki/48KHz-USB-Audio

Huge thanks go out to the authors of these works without which this patch would not have been possible.

Also thanks to John Melton G0ORX for testing the patch on linux.



