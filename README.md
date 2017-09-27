# s107g-arduino

Code to control a S107G RC helicopter with an Arduino Uno

## Sources

The code in this repository is based on the work of several people, including [Jim'ung](http://www.jimhung.co.uk/) and [Ladyada](https://www.adafruit.com/). Credits to them for reverse engineering the controller protocol and showing how to send a 38KHz infrarred pulse using Arduino, respectively.

Some of the links on which I based my work are:
* https://github.com/vivin/syma
* http://www.jimhung.co.uk/?p=901
* http://www.jimhung.co.uk/?p=1241
* http://www.kerrywong.com/2012/08/27/reverse-engineering-the-syma-s107g-ir-protocol/
* https://sites.google.com/site/spirixcode/code/kodek.txt
* https://learn.adafruit.com/ir-sensor/making-an-intervalometer

## S107G protocol

The protocol to control the RC helicopter can be found [here](http://www.jimhung.co.uk/wp-content/uploads/2013/01/Syma107_ProtocolSpec_v1.txt). Note that this code works for the 2-channel version, if you have a 3-channel remote controller follow [this link](http://abarry.org/s107g-helicopter-control-via-arduino/).
