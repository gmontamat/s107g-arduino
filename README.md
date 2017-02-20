# s107g-arduino

Code to control a S107G RC helicopter with an Arduino Uno

## Sources

The code in this repository is based on the work of several people. Credit to them for reverse engineering the controller protocol.

* https://github.com/vivin/syma
* http://www.jimhung.co.uk/?p=901
* http://www.jimhung.co.uk/?p=1241
* http://www.kerrywong.com/2012/08/27/reverse-engineering-the-syma-s107g-ir-protocol/
* https://sites.google.com/site/spirixcode/code/kodek.txt?attredirects=0
* https://learn.adafruit.com/ir-sensor/making-an-intervalometer

## Information

The protocol to control the RC helicopter can be found [here](http://www.jimhung.co.uk/wp-content/uploads/2013/01/Syma107_ProtocolSpec_v1.txt). Note that this code works for the 2-channel version, if you have a 3-channel remote controller follow [this link](ttp://abarry.org/s107g-helicopter-control-via-arduino/).
