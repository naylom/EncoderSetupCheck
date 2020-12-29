# EncoderSetupCheck

This arduino sketch is designed to help test that the user has there rotary encoder correctly connected to their arduino and it is seeing
signals before they then use the related EncoderQualityCheck program

It first identifies which digital pins will support interrupts and monitors these.

It expects to find input signals on 3 digital pins. It first tests for 15 seconds with the identified pins set to use the internal pullups on the Arduino. 

If 3 signals are not seen then any pins that have not signalled are put into input mode and further checked for another 15 seconds . At the end it shows each pin and the result.

Given the requirement for 3 pins all supporting interrupts this will limit the model of arduino used. This was tested on a MEGA2560.

Typical output
--------------

Encoder Setup Check Ver. 1

Serial output running at 115200

Compatibility warning: This program will only check the first 22 digital pins, your machine has 70

6 pin(s) can be used for interrupts as follows:

Pin #	Int Vector

2	0

3	1

18	5

19	4

20	3

21	2

Starting to look for signals on pins
 

Checking INPUT_PULLUP mode

Please turn encoder

...

 Pin 2 attached to interrupt 0 signalled in mode INPUT_PULLUP #Signals 864 #DiscardedSignals 69

 Pin 3 attached to interrupt 1 signalled in mode INPUT_PULLUP #Signals 862 #DiscardedSignals 71

 Pin 18 attached to interrupt 5 signalled in mode INPUT_PULLUP #Signals 1 #DiscardedSignals 0

 Pin 19 attached to interrupt 4 did not signal

 Pin 20 attached to interrupt 3 did not signal

 Pin 21 attached to interrupt 2 did not signal

Minimum number of pins found, terminating program

