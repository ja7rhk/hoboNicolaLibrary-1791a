// Arduino Leonardo
// Wiring Connections
// - Connect external module TX to Leonardo Pin 0 (RX)
// - Connect external module RX to Leonardo Pin 1 (TX)
// - Connect shared Ground (GND)

void setup() {
  // Start the USB serial monitor (wait for connection if needed)
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for native USB port to connect
  }
  Serial.println("oasys30_kb_test start\n");

  // Start the hardware serial port on pins 0 and 1
  Serial1.begin(9600, SERIAL_8E1);
}

/*
void loop() {
  // Pass data from external device to computer screen
  if (Serial1.available() > 0) {
    uint8_t c = Serial1.read();
    Serial.println(c, HEX);
  }
}
*/

uint8_t p = 0;
void loop() {
     // Pass data from external device to computer screen
    if (Serial1.available() > 0) 
    {
        uint8_t c = Serial1.read();
        Serial.print(c, HEX);
        Serial.print(' ');

        if (c == 0x7F && p == 0x90)
            Serial.print('\n');
        p = c;
    }
}

/*
bool pressed = false;
void loop() {
  // Pass data from external device to computer screen
  if (Serial1.available() > 0) {
    uint8_t c = Serial1.read();
    if (c > 0 && c < 0x80) {
        Serial.println(c, HEX);
    } else if (c == 0x80) {
        if (!pressed) 
            Serial.println("key pressed");
        else
            Serial.println("key repeated");
        pressed = true;
    } else if (c == 0x90) {
        Serial.println("key released");
        pressed = false;
    }
  }

  // Pass data from computer screen to external device
  if (Serial.available() > 0) {
    char outgoingChar = Serial.read();
    Serial1.print(outgoingChar);
  }
}
*/
