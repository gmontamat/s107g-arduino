/*
   Syma S107G RC helicopter Arduino controller
   2-channel version (both 24-bit and 32-bit)

   Emulates the remote control of the S107G with commands passed through
   the serial port, using an array of infrarred LEDs attached to pin 3.
   For more information refer to https://github.com/gmontamat/s107g-arduino

   The circuit:
   * Two/three 940nm IR LEDs and a resistor (see http://led.linear1.org/led.wiz)
*/

#define LED 3
#define HEADER_HIGH_US  2000
#define HEADER_LOW_US   2000
#define FOOTER_HIGH_US  300
#define FOOTER_LOW_US   2001  // >2000us according to specification
#define BIT_HIGH_US     300
#define BIT_LOW_1_US    700
#define BIT_LOW_0_US    300

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
  /* Sends 38KHz (50% duty cycle) pulse to LED pin for 'us' microseconds */
  while (us > 26) {
    digitalWrite(LED, HIGH);  // 3us approximately
    delayMicroseconds(10);
    digitalWrite(LED, LOW);   // 3us approximately
    delayMicroseconds(10);
    us -= 26;                 // 26us in total
  }
  if (us > 19) {
    // If more than 75% of a period remains, send another pulse
    digitalWrite(LED, HIGH);
    delayMicroseconds(10);
    digitalWrite(LED, LOW);
    delayMicroseconds(10);
  } else if (us > 6) {
    // If between 25% and 75% of a period remains, send half a pulse
    digitalWrite(LED, HIGH);
    delayMicroseconds(10);
    digitalWrite(LED, LOW);
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

  // Begin control packet transmission
  cli();
  sendHeader();

  // Send 32-bit command (replace 4 with a 3 for 24-bit version)
  while (dataPointer < 4) {
    sendPulse(BIT_HIGH_US);
    if(data[dataPointer] & mask[--maskPointer]) {
      delayMicroseconds(BIT_LOW_1_US); // 1
    } else {
      delayMicroseconds(BIT_LOW_0_US); // 0
    }
    if (!maskPointer) {
      maskPointer = 8;
      dataPointer++;
    }
  }

  sendFooter();
  sei();
  // Control packet sent

  // Print command data in serial monitor
  Serial.print(" Channel: ");
  Serial.print(channel, DEC);
  Serial.print("\t Yaw: ");
  Serial.print(data[0], DEC);
  Serial.print("\t Pitch: ");
  Serial.print(data[1], DEC);
  Serial.print("\t Throttle: ");
  Serial.print(throttle, DEC);
  Serial.print("\t Trim: ");
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
    interval = 120;  // channel A (120ms between headers)
  } else {
    interval = 180;  // channel B (180ms between headers)
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

