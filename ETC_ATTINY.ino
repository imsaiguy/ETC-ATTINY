//============================================================================
//  ImsaiGuy 2025
//  for the Heathkit ETC-3800
//  this uses the ATTINY814
//  uses OLED display connected to pins 8 (SDA) and 9 (SCL)
//  sets address buss to 0xB240
//  sends a count to data buss
//  latches data into output port
//  blinks line AI7 (connect to an LED)
//  blinks LEDD
//============================================================================

#define LED 9     // PA2 routed to AI7, connect to LED
#define Button 8  // routed to AI6

#include <Tiny4kOLED.h>  // Driver for I2C 1306 OLED display
#include "ModernDos8.h"  // Load font
const DCfont *currentFont = FONT8X8MDOS;

#include "TCA9555.h"
TCA9555 TCA1(0x20);
TCA9555 TCA2(0x21);
TCA9555 TCA3(0x22);
//TCA1 bits
// 0 - BA0
// 1 - BA1
// 2 - BA2
// 3 - BA3
// 4 - BA4
// 5 - BA5
// 6 - BA6
// 7 - BA7
// 8 - BA8
// 9 - BA9
// 10 - BA10
// 11 - BA11
// 12 - BA12
// 13 - BA13
// 14 - BA14
// 15 - BA15

//TCA2 bits
// 0 - BD0
// 1 - BD1
// 2 - BD2
// 3 - BD3
// 4 - BD4
// 5 - BD5
// 6 - BD6
// 7 - BD7
// 8 - B-READ
// 9 - B-WRITE
// 10 - IOSEL
// 11 - NC
// 12 - NC
// 13 - NC
// 14 - NC
// 15 - NC

//TCA2 bits
// 0 - KEYCOL1
// 1 - KEYCOL2
// 2 - KEYCOL3
// 3 - NC
// 4 - NC
// 5 - LCDR-W
// 6 - LCDRS
// 7 - LEDD
// 8 - Ground
// 9 - Ground
// 10 - Ground
// 11 - Ground
// 12 - Ground
// 13 - Ground
// 14 - Ground
// 15 - Ground

int count = 0;
byte row = 0;
byte column = 0;

//============================================================================
void setup() {
  pinMode(LED, OUTPUT);
  pinMode(Button, INPUT_PULLUP);
  oled.begin();
  oled.setFont(currentFont);
  oled.clear();
  oled.on();
  oled.setCursor(0, 0);
  oled.println("ImsaiGuy 2025");
  oled.println("ETC-Attiny814");
  oled.println("..hit a key..");

  TCA1.begin();
  for (int pin = 0; pin < 16; pin++) {
    TCA1.pinMode1(pin, OUTPUT);
  }
  TCA2.begin();
  for (int pin = 0; pin < 16; pin++) {
    TCA2.pinMode1(pin, OUTPUT);
  }
  TCA3.begin();
  for (int pin = 0; pin < 16; pin++) {
    TCA3.pinMode1(pin, OUTPUT);
  }
  // for (int pin = 8; pin < 16; pin++) {
  //   TCA3.pinMode1(pin, INPUT);
  // }

  initializeLCD();  // Initialize the LCD
  writeStringToLCD("ETC-ATTiny", 1);  // Write string to line 1
  writeStringToLCD("ImsaiGuy 2025", 2);   // Write string to line 2
}

//============================================================================
void loop() {
  // counter_loop();
  // cycle_IO();
  get_key();
  OLED_update();
  
  // delay(3000);
}

//============================================================================
void counter_loop() {
  TCA3.write1(7, HIGH);
  digitalWrite(LED, HIGH);  // turn the LED on (HIGH is the voltage level)
  delay(50);                // wait for a second
  digitalWrite(LED, LOW);   // turn the LED off by making the voltage LOW
  delay(50);                //
  digitalWrite(LED, HIGH);  // turn the LED on (HIGH is the voltage level)
  delay(150);               // wait for a second
  digitalWrite(LED, LOW);   // turn the LED off by making the voltage LOW
  // delay(300);               // wait for a second
  TCA3.write1(7, LOW);
  delay(300);
  oled.setCursor(0, 10);
  oled.println(count);
  if (digitalRead(Button) == 0) {
    count = 0;  // zero count if button pressed
    oled.setCursor(0, 10);
    oled.println("        ");
  }

  TCA1.write16(0x0248);   // output port address
  TCA2.write8(0, count);  // put count on the data buss
  TCA2.write1(10, LOW);   // latch output port
  TCA2.write1(10, HIGH);

  delay(300);
  count += 1;
}

//============================================================================
// just for debug
void cycle_IO() {        // these are the selects
  TCA1.write16(0x0300);  // IO port address IO0
  TCA2.write1(10, LOW);  // IOSEL low
  TCA1.write16(0x0340);  // IO port address IO1
  TCA1.write16(0x0380);  // IO port address IO2
  TCA1.write16(0x03c0);  // IO port address IO3
}

//============================================================================
// scans the keyboard matrix
// if no key pressed row = 0
// if key pressed row and column are set to key pressed
// row and column are global variables
void get_key() {
  TCA1.write16(0xB200);                 // keyboard port address
  for (byte pin = 0; pin < 8; pin++) {  // set data port for input
    TCA2.pinMode1(pin, INPUT);
  }
  TCA2.write1(10, LOW);  // IOSEL low
  TCA3.write1(0, LOW);   // set KEYCOL1
  TCA3.write1(1, HIGH);
  TCA3.write1(2, HIGH);
  byte column1 = TCA2.read8(0);
  TCA3.write1(0, HIGH);
  TCA3.write1(1, LOW);  // set KEYCOL2
  TCA3.write1(2, HIGH);
  byte column2 = TCA2.read8(0);
  TCA3.write1(0, HIGH);
  TCA3.write1(1, HIGH);
  TCA3.write1(2, LOW);           // set KEYCOL3
  byte column3 = TCA2.read8(0);  // read data bus
  row = 0;
  if (column1 != 0xff) {
    column = 1;
    row = __builtin_ctz(~column1);  // counts the number of trailing zeros
  }
  if (column2 != 0xff) {
    column = 2;
    row = __builtin_ctz(~column2);
  }
  if (column3 != 0xff) {
    column = 3;
    row = __builtin_ctz(~column3);
  }
}

//============================================================================
// refresh the OLED display with latest data if a key was pressed
void OLED_update() {
  if (row != 0) {
    oled.clear();
    oled.setCursor(0, 0);
    oled.println("Last Key Pressed");
    oled.print("Row = ");
    oled.println(row);
    oled.print("Column = ");
    oled.println(column);
    // output_port(row);  // just for debug
  }
}

//============================================================================
// output a byte to the ETC-3800 ouput port
void output_port(byte databyte) {
  TCA1.write16(0xB240);
  TCA2.write1(10, LOW);                 // IOSEL low
  for (byte pin = 0; pin < 8; pin++) {  // set data port for input
    TCA2.pinMode1(pin, OUTPUT);
  }
  TCA2.write8(0, databyte);
}



//============================================================================
// write to the ETW-3800 LCD display
//
// Initialize the LCD in 8-bit mode
void initializeLCD() {
  TCA1.write16(0x02C0);  // LCD address
  // these 3 lines are inverted logic
  TCA2.write1(10, HIGH);  // IOSEL high, Enable low
  TCA3.write1(5, HIGH);   // RW low
  TCA3.write1(6, HIGH);   // RS low
  delay(80);
  writeCommand(0x38);  // 8-bit mode
  delay(1);
  writeCommand(0x38);
  delay(1);
  writeCommand(0x0E);  // display on, blink off, cursor off, display off
  delay(1);
  writeCommand(0x01);  // clear display
  delay(10);
  writeCommand(0x06);  // display not shifted, cursor increment
  delay(10);
  // writeCommand(0x0C);  // display on
  // delay(10);
}

//============================================================================
// Write a string to a specific line (1 or 2)
void writeStringToLCD(const char *str, int line) {
  if (line == 1) {
    writeCommand(0x80);  // Line 1 starts at 0x80
  } else if (line == 2) {
    writeCommand(0xC0);  // Line 2 starts at 0xC0
  }
  // Write each character
  while (*str) {
    writeData(*str++);
  }
}

//============================================================================
// Write a command to the LCD
void writeCommand(uint8_t cmd) {
  TCA3.write1(6, HIGH);  //  RS = 0 for command
  write8Bits(cmd);       // write command
  pulseEnable();
}

//============================================================================
// Write data to the LCD
void writeData(uint8_t data) {
  TCA3.write1(6, LOW);  //  RS = 1 for data
  write8Bits(data);     // write data
  pulseEnable();
}

//============================================================================
// Send 8 bits to the data pins
void write8Bits(uint8_t value) {
  for (byte pin = 0; pin < 8; pin++) {  // set data port for output
    TCA2.pinMode1(pin, OUTPUT);
  }
  TCA2.write8(0, value);
}

//============================================================================
// Generate a pulse on the Enable pin
void pulseEnable() {
  TCA1.write16(0x02C0);
    TCA2.write1(10, LOW);  // IOSEL low  Enable high
    TCA2.write1(10, HIGH);  // IOSEL high  Enable low
  }




//============================================================================
// End of file
//============================================================================