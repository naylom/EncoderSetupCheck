# EncoderSetupCheck

This arduino sketch is designed to help test that the user has there rotary encoder correctly connected to their arduino and it is seeing
signals before they then use the related EncoderQualityCheck program

It first identifies which digital pins will support interrupts and these these.

It expects to find input signals on 3 digital pins. It first tests for 15 seconds with the identified pins set to use the internal pullups on the Arduino. 

If 3 signals are not seen then any pins that have not signalled are put into input mode and further checked for another 15 seconds . At the end it shows each pin and the result.

Given the requirement for 3 pins all supporting interrupts this will limit the model of arduino used. This was tested on a MEGA2560.
