//--- KEEP THIS IN MIND ---
  //The MAX6954 is written to using the following sequence:
  // Take CLK low.
  // Take CS low. This enables the internal 16-bit shift register.
  // Clock 16 bits of data into DIN, D15 first to D0 last, observing the setup and hold times. Bit D15 is low, indicating a write command.
  // Take CS high (while CLK is still high after clocking in the last data bit).
  // Take CLK low.


#include <MAX6954.h>
#include <Arduino.h>
#include <SPI.h>
#define CS1       // CHIP SELECT
#define DATAIN 11  // DATAIN
#define CK 13      // CK
#define STARTING_PORT 2
#define NUMBER_OF_MAXS 2 + STARTING_PORT

uint8_t incomingByte = 0;

// For displaying the digits on a 7-segment
const byte digit7Patterns[10] = {
  0b01111110,  // 0: segments a, b, c, d, e, f are on
  0b00110000,  // 1: segments b, c are on
  0b01101101,  // 2: segments a, b, d, e, g are on
  0b01111001,  // 3: segments a, b, c, d, g are on
  0b00110011,  // 4: segments b, c, f, g are on
  0b11011011,  // 5: segments a, c, d, f, g are on
  0b11111010,  // 6: segments a, c, d, e, f, g are on
  0b00001110,  // 7: segments a, b, c are on
  0b11111110,  // 8: all segments are on
  0b11011110   // 9: segments a, b, c, d, f, g are on
};

// For sending the tuple of address-value
void send_data(byte address, byte data, uint8_t CS) {
  send_address(address, CS);
  send_value(data, CS);
}

// For sending the address/register
void send_address(byte address, uint8_t CS) {
  char abit;
  char index;
  digitalWrite(CS, LOW);
  for (index = 7; index >= 0; index--) {
    abit = ((address >> index) & 0x01);
    if (abit == 1) {
      digitalWrite(DATAIN, HIGH);
      pulse_clock();
    } else {
      digitalWrite(DATAIN, LOW);
      pulse_clock();
    }
  }
}

// For sending the value
void send_value(byte data, uint8_t CS) {
  char abit;
  char index;
  for (index = 7; index >= 0; index--)  //7
  {
    abit = ((data >> index) & 0x01);
    if (abit == 1) {
      digitalWrite(DATAIN, HIGH);
    } else {
      digitalWrite(DATAIN, LOW);
    }
    if (index == 0) {
      digitalWrite(CK, HIGH);
    } else {
      pulse_clock();
    }
  }
  digitalWrite(CS, HIGH);
  digitalWrite(CK, LOW);
}

// For syncronizing the clock when a bit is sent to the driver
void pulse_clock() {
  digitalWrite(CK, HIGH);
  digitalWrite(CK, LOW);
}

// For clearing the display
void clear() {
  for (int x = STARTING_PORT; x < NUMBER_OF_MAXS; x++) {
    for (int y = 0; y < 8; y++) {
      // Clearing the 16-segements LED
      send_data(0x60+y, ' ', x);
    }
  }
}

// The configuring commands for the MAX6954
void configure_16segments(uint8_t CS){
    //DECODE MODE - Using the Built-In ASCII 104-Character Font for 16-Segment
  send_data(0x01, 0xFF, CS);
  // Global brightness - 14/15 - Full brightness
  send_data(0x02, 0x14, CS);
  // Scan Limit - Because we use only 1 Digit
  send_data(0x03, 0x07, CS);
  // Configuration - Normal operation and global brightness
  send_data(0x04, 0x01, CS);  //0x00
  // Digit Type - We are using 16-segment LED display
  send_data(0x0C, 0x00, CS);
}

void setup() {

  //Set PINS
  pinMode(DATAIN, OUTPUT);
  pinMode(CK, OUTPUT);
  for (int i = STARTING_PORT; i < NUMBER_OF_MAXS; i++) {
    int CS = i;
  // Setting all the CS pins as OUTPUT
    pinMode(CS, OUTPUT);
}
  // pinMode(CS, OUTPUT);
  
  // Assuring the pins are correctly set up before any action
  digitalWrite(CK, LOW);
  digitalWrite(DATAIN, LOW);
  for (int i = STARTING_PORT; i < NUMBER_OF_MAXS; i++) {
    int CS = i;
  // Setting all the CS pins on HIGH
    digitalWrite(CS, HIGH);
}
  //digitalWrite(CS, HIGH);

  //7 segments Configuration
    // // Decode mode - 7-segment font enabled
    // send_data(0x01, 0xFF);
    // //7seg
    // // Decode mode - NO DECODE MODE
    // send_data(0x01, 0x00);
    // // Global brightness - 14/15 - full brightness
    // send_data(0x02, 0x14);
    // // Scan Limit - 1 digit
    // send_data(0x03, 0x00);
    // // Configuration - normal operation and global brightness
    // send_data(0x04, 0x01);
    // // Digit Type - all 7-segments
    // send_data(0x0C, 0x00);

  //Call the configuring 16-segments FUNCTION
  for (int i = STARTING_PORT; i < NUMBER_OF_MAXS; i++) {
    int CS = i;
    configure_16segments(CS);
  }
  
  //Testing the LED segments/connections
  for (int i = STARTING_PORT; i < NUMBER_OF_MAXS; i++) {
    int CS = i;
    send_data(0x07, 0x01, CS);
    delay(100);
    send_data(0x07, 0x00, CS);
    delay(100);
  }

  // Initialize Serial Communication at a baud rate of 9600
  Serial.begin(9600);
   
}

void displayScrollingText(const char* text) {
    int len = strlen(text);
    for (int i = 0; i < len + (NUMBER_OF_MAXS - STARTING_PORT)*8; i++) { //TO GO THROUGH ALL THE MESSAGE
        clear();
        for (int j = 0; j < (NUMBER_OF_MAXS - STARTING_PORT)*8; j++) { // DISPLAYING ON ALL THE DIGITS
            int charIndex = i + j;
            if (charIndex >= 0 && charIndex < len) {
                send_data(0x60 + j%8, text[charIndex], j/8+STARTING_PORT);
            }
        }
        delay(50); // Adjust the delay to control the scrolling speed
    }
}

void loop() {
    // Check if data is available to read
    if (Serial.available() > 0) {
        // Read the incoming byte
        String message = Serial.readStringUntil('\n'); // Read until newline character
        message.trim(); // Remove any leading/trailing whitespace
        
        if (message.length() > 0) {
            // Display the received message
            displayScrollingText(message.c_str());
            delay(1000); // Wait before scrolling the message again
        }
    }



  // Direct controlling the Yellow LED
    // digitalWrite(YELLOW_LED, 1);
    // delay(500);
    // digitalWrite(YELLOW_LED, 0);
    // delay(500);
    // digitalWrite(YELLOW_LED, 1);
    // delay(500);

  // For the 7 segments
    // Display digits NO DECODE MODE - for version
    //   for (int i = 0; i < 9 ; i++) {
    //     send_data(0x20, digit7Patterns[i]);
    //     delay(1000);
    //     clear();
    // }
  // Tried to display different characters in different writting types
    //  send_data(0x60, '0');
    //  delay(1000);
    //  clear();
    //  send_data(0x60,0x31);
    //  delay(1000);
    //  clear();
    //  send_data(0x60,']');
    //   delay(1000);
    //   clear();
    //   send_data(0x60,'~');
    //   delay(1000);
    //   clear();
  
  // Display on each digit one number
    // for (int i = 0; i < 8; i++) {
    //   send_data(0x60+i, 0x30+i);
    //   delay(500);
    // }
    // delay(1000);
    // clear();

  // Display on each digit one character
    //String nume = "ANDREI12";
    // for (int i = 0; i < 8; i++) {
    //   send_data(0x60+i, nume[i]);
    //   delay(500);
    // }
    // delay(1000);
    // clear();

  // Digits
    // for (int i = 0; i < 10; i++) {
    //   send_data(0x60, 0x30 + i);
    //   delay(1000);
    //   clear();
    // }

  // Capital letters
    // for (int i = 0; i < 26; i++) {
    //   send_data(0x60, 0x41 + i);
    //   delay(1000);
    //   clear();
    // }

  // LITERE MICI
    // for (int i = 0; i < 26 ; i++) {
    //   send_data(0x60,0x61+i);
    //   delay(1000);
    //   clear();
    // }
}