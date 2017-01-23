/*
   S107G RC helicopter Arduino controller (2-channel, 32-bit version)

   Replaces the remote controller of the S107G with commands passed through
   the serial port by using an array of infrarred LEDs attached to pin 3.

   The circuit:
   * Two or three 940nm IR LEDs and a resistor (see http://led.linear1.org/led.wiz)

   Based on:
   * http://www.jimhung.co.uk/?p=901
   * http://www.jimhung.co.uk/?p=1241
   * http://www.jimhung.co.uk/wp-content/uploads/2013/01/Syma107_ProtocolSpec_v1.txt
   * http://www.kerrywong.com/2012/08/27/reverse-engineering-the-syma-s107g-ir-protocol/
*/

#define LED 3

byte inputBuffer[5];

void setup() {
  pinMode(LED, OUTPUT);
  digitalWrite(LED, LOW);

  Serial.begin(9600);

  inputBuffer[0] = byte(0);   // channel (0 OR 128)
  inputBuffer[1] = byte(63);  // yaw (0 to 127)
  inputBuffer[2] = byte(63);  // pitch (0 to 127)
  inputBuffer[3] = byte(0);   // throttle (0 to 127)
  inputBuffer[4] = byte(63);  // trim (0 to 127)
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
  sendPulse(2002);
  delayMicroseconds(1998);
}

void sendFooter() {
  sendPulse(312);
}

void sendControlPacket(byte channel, byte yaw, byte pitch, byte throttle, byte trim_) {
  byte dataPointer, maskPointer;
  byte data[4];
  const byte mask[] = {1, 2, 4, 8, 16, 32, 64, 128};

  data[0] = yaw;
  data[1] = pitch;
  data[2] = throttle;
  data[3] = trim_;
  dataPointer = 4;
  maskPointer = 8;

  data[2] = data[2] + channel;

  sendHeader();

  // Send 32-bit command
  while (dataPointer > 0) {
    sendPulse(312);
    if(data[4 - dataPointer] & mask[--maskPointer]) {
      delayMicroseconds(700); // 1
    } else {
      delayMicroseconds(300); // 0
    }
    if (!maskPointer) {
      maskPointer = 8;
      dataPointer--;
    }
  }

  sendFooter();
}

void loop() {
  static unsigned long millisLast = millis();
  int cycleTime;

  if (inputBuffer[3] > 0) {
    sendControlPacket(
      inputBuffer[0], inputBuffer[1], inputBuffer[2],
      inputBuffer[3], inputBuffer[4]
    );
  }

  // Print control data to serial monitor
  Serial.print("Channel = ");
  Serial.print(inputBuffer[0], DEC);
  Serial.print("\tYaw = ");
  Serial.print(inputBuffer[1], DEC);
  Serial.print("\tPitch = ");
  Serial.print(inputBuffer[2], DEC);
  Serial.print("\tThrottle = ");
  Serial.print(inputBuffer[3], DEC);
  Serial.print("\tTrim = ");
  Serial.println(inputBuffer[4], DEC);

  if (inputBuffer[0] == byte(0)) {
    cycleTime = 120;
  } else {
    cycleTime = 180;
  }
  while (millis() < millisLast + cycleTime);
  millisLast = millis();
}

void serialEvent() {
  while (Serial.available()) {
    String str = Serial.readString();
    if (str.startsWith("c:")){
      inputBuffer[0] = byte(str.substring(2).toInt());
    } else if (str.startsWith("y:")) {
      inputBuffer[1] = byte(str.substring(2).toInt());
    } else if (str.startsWith("p:")) {
      inputBuffer[2] = byte(str.substring(2).toInt());
    } else if (str.startsWith("t:")) {
      inputBuffer[3] = byte(str.substring(2).toInt());
    } else if (str.startsWith("r:")) {
      inputBuffer[4] = byte(str.substring(2).toInt());
    }
  }
}
