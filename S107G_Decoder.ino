/*
SymaLib Library Example v0.1 by Jim Hung (www.jimhung.com)

 This example code is designed to demonstrate the usage of the SymaLib
 to decode control signals from a S107T2 RC helicopter controller.

 Based on 'Raw IR commander' by Ladyada:

   "This sketch/program uses the Arduno and a PNA4602 to
   decode IR received.  It then attempts to match it to a previously
   recorded IR signal

   Code is public domain, check out www.ladyada.net and adafruit.com
   for more tutorials!"
*/

#include <SymaLib.h>

#define IRpin_PIN      PIND
#define IRpin          2

#define MAXPULSE 2000
#define NUMPULSES 50

#define RESOLUTION 20
#define FUZZINESS 30

uint16_t pulses[NUMPULSES][2];  // pair is high and low pulse
uint8_t currentpulse = 0; // index for pulses we're storing

SymaLib protocolparser; // declare our SymaLib object

void setup(void) {

  Serial.begin(9600);
  Serial.println("Ready to decode IR!");
}

void loop(void) {

  int numberpulses;
  String ctrldata = "";

  // Listen for IR signals and populate the array of pulse lengths

  numberpulses = listenForIR();

  // Give the SymaLib object the pulse array to parse. If it's a valid Syma107
  // control packet, SymaLib returns boolean True.

  boolean result = protocolparser.setPulseListParse(pulses, numberpulses, RESOLUTION, FUZZINESS);

  if (result) {

    // Read the controller values and do whatever you need to with them.

      // This example prints out the integer values for each control:

    Serial.print(" Yaw: ");
    Serial.print(protocolparser.getYawDec());
    Serial.print("\tPitch: ");
    Serial.print(protocolparser.getPitchDec());
    Serial.print("\tChannel: ");
    Serial.print(protocolparser.getChanDec());
    Serial.print("\tThrottle: ");
    Serial.print(protocolparser.getThrottleDec());
    Serial.print("\tTrim: ");
    Serial.print(protocolparser.getTrimDec());
    Serial.print("\n");

      // This example prints out the binary values for the 32-bit control packet:

    Serial.println(protocolparser.getCtrlPacketBin());

  }
  delay(100);
}

int listenForIR(void) {
  currentpulse = 0;

  while (1) {
    uint16_t highpulse, lowpulse;
    highpulse = lowpulse = 0;

    while (IRpin_PIN & _BV(IRpin)) {

       highpulse++;
       delayMicroseconds(RESOLUTION);

       if (((highpulse >= MAXPULSE) && (currentpulse != 0))|| currentpulse == NUMPULSES) {
         return currentpulse;
       }
    }

    pulses[currentpulse][0] = highpulse;

    while (! (IRpin_PIN & _BV(IRpin))) {

       lowpulse++;
       delayMicroseconds(RESOLUTION);

        if (((lowpulse >= MAXPULSE)  && (currentpulse != 0))|| currentpulse == NUMPULSES) {
         return currentpulse;
       }
    }
    pulses[currentpulse][1] = lowpulse;

    currentpulse++;
  }
}
