/*
   Syma S107G RC helicopter Arduino controller
   2-channel version (both 24-bit and 32-bit)

   Emulates the remote control of the S107G with commands passed through
   the serial port, using an array of infrarred LEDs attached to pin 3.

   The circuit:
   * Two/three 940nm IR LEDs and a resistor (see http://led.linear1.org/led.wiz)

   Based on:
   * http://www.jimhung.co.uk/?p=901
   * http://www.jimhung.co.uk/?p=1241
   * http://www.jimhung.co.uk/wp-content/uploads/2013/01/Syma107_ProtocolSpec_v1.txt
   * http://www.kerrywong.com/2012/08/27/reverse-engineering-the-syma-s107g-ir-protocol/
   For the 3-channel model refer to:
   * http://abarry.org/s107g-helicopter-control-via-arduino/
*/

#define LED 3
#define HEADER_HIGH_US  2002
#define HEADER_LOW_US   1998
#define FOOTER_HIGH_US  312
#define FOOTER_LOW_US   2001  // >2000us according to specification
#define CONTROL_HIGH_US 312
#define CONTROL_1_US    700
#define CONTROL_0_US    300

byte inputBuffer[5];

void setup() {
  pinMode(LED, OUTPUT);
  digitalWrite(LED, LOW);

  Serial.begin(9600);

  inputBuffer[0] = 0;   // channel (0 for A OR 128 for B)
  inputBuffer[1] = 63;  // yaw (0 to 127)
  inputBuffer[2] = 63;  // pitch (0 to 127)
  inputBuffer[3] = 0;   // throttle (0 to 127)
  inputBuffer[4] = 63;  // trim (0 to 127)
}

void sendPulse(long us) {
  /* Sends 38Khz pulse when using a 16Mhz IC */
  for (int i = 0; i < (us / 26) - 1; i++) {
    digitalWrite(LED, HIGH);
    delayMicroseconds(10);
    digitalWrite(LED, LOW);
    delayMicroseconds(10);
  }
}

void sendHeader() {
  sendPulse(HEADER_HIGH_US);
  delayMicroseconds(HEADER_LOW_US);
}

void sendFooter() {
  sendPulse(FOOTER_HIGH_US);
  delayMicroseconds(FOOTER_LOW_US);
}

void sendControlPacket(byte channel, byte yaw, byte pitch, byte throttle, byte trim_) {
  const byte mask[] = {1, 2, 4, 8, 16, 32, 64, 128};
  byte dataPointer = 0;
  byte maskPointer = 8;
  byte data[4];

  data[0] = byte(yaw + (int(trim_) - 63) / 3); // trim_ adjusts yaw +/-20
  data[1] = pitch;
  data[2] = throttle + channel;
  data[3] = trim_;  // The S107G model ignores the trim byte

  sendHeader();

  // Send 32-bit command (replace 4 with a 3 for 24-bit version)
  while (dataPointer < 4) {
    sendPulse(CONTROL_HIGH_US);
    if(data[dataPointer] & mask[--maskPointer]) {
      delayMicroseconds(CONTROL_1_US); // 1
    } else {
      delayMicroseconds(CONTROL_0_US); // 0
    }
    if (!maskPointer) {
      maskPointer = 8;
      dataPointer++;
    }
  }

  sendFooter();

  // Print command data in serial monitor
  Serial.print(" Channel = ");
  Serial.print(channel, DEC);
  Serial.print("\tYaw = ");
  Serial.print(data[0], DEC);
  Serial.print("\tPitch = ");
  Serial.print(data[1], DEC);
  Serial.print("\tThrottle = ");
  Serial.print(throttle, DEC);
  Serial.print("\tTrim = ");
  Serial.println(data[3], DEC);
}

void loop() {
  static unsigned long millisLast = millis();
  unsigned long interval;

  if (inputBuffer[3] > 0) {
    // Send command to helicopter
    sendControlPacket(
      inputBuffer[0], inputBuffer[1], inputBuffer[2],
      inputBuffer[3], inputBuffer[4]
    );
  }

  // Wait before sending next command
  if (!inputBuffer[0]) {
    interval = 120;  // channel A
  } else {
    interval = 180;  // channel B
  }
  while (millis() < millisLast + interval);
  millisLast = millis();
}

void serialEvent() {
  while (Serial.available()) {
    String str = Serial.readString();
    if (str.startsWith("c:")){
      inputBuffer[0] = str.substring(2).toInt();
    } else if (str.startsWith("y:")) {
      inputBuffer[1] = str.substring(2).toInt();
    } else if (str.startsWith("p:")) {
      inputBuffer[2] = str.substring(2).toInt();
    } else if (str.startsWith("t:")) {
      inputBuffer[3] = str.substring(2).toInt();
    } else if (str.startsWith("r:")) {
      inputBuffer[4] = str.substring(2).toInt();
    }
  }
}
