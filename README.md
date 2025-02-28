# arduinoalarm
This is a mini fun project, a school alarm using an Arduino UNO, the time is displayed on an LCD screen (2x16), two LEDs connected to the board : a green LED is ON from 8 am to 3 pm (school time), a red LED is ON when the buzzer is playing.


* Features:
- Displays "Test Exam" on Line 1 and the current time (HH:MM) on Line 2 of an I2C LCD.

- LED indicator (LED0) turns on from 8:00 AM to 3:00 PM.

- Hourly alarm with a 5-second activation of a buzzer and LED1 (on the hour, e.g., 16:00, 17:00).

- Interactive time setup:
   - Press button1Pin (pin 12) to enter hours setup and increment hours (0-23) with each short press.
   - Press button2Pin (pin 13) to enter minutes setup and increment minutes (0-59) with each short press.
   - Exits setup mode after 3 seconds of inactivity.

- Uses the DS3231 RTC for accurate timekeeping via I2C.

- Includes debug output on the LCD showing raw BCD time values.

* Hardware Requirements:
   Arduino Uno
   DS3231 RTC Module
   I2C 16x2 LCD (e.g., with PCF8574 backpack, address 0x27 or 0x3F)
   LEDs (2x, with 220Ω resistors)
   Buzzer
   Push buttons (2x)
   4.7kΩ pull-up resistors (for I2C lines, if not built into modules)
   Breadboard and jumper wires

* Software Requirements:
  Arduino IDE
  Wire library (included with Arduino)
  LiquidCrystal_I2C library (install via Library Manager)
  
  If needed : An Arduino library for Proteus if you want to ruun the simulation, the files are included (to install it simply copy the idx and lib files to \DATA\LIBRARY folder in proteus)

* Setup Instructions:

1)Connect the hardware as follows:
   
-DS3231: SDA → SDA, SCL → SCL, VCC → 5V, GND → GND.
-I2C LCD: SDA → SCL, SCL → SCL, VCC → 5V, GND → GND.   
-LED0 (8 AM-3 PM): Pin 8 → Anode, Cathode → GND (with 220Ω resistor connected between the arduino and the anode).
-LED1 (Alarm): Pin 9 → Anode, Cathode → GND (with 220Ω resistor connected between the arduino and the anode).
-Buzzer (Alarm): Pin 10 → Positive, Negative → GND.
-Button1 (Hours): Pin 12 → One side, other side → GND.
-Button2 (Minutes): Pin 13 → One side, other side → GND.
-Ensure 4.7kΩ pull-ups on SDA/A4 and SCL/A5 if not built into modules (personally I didn't use it)

2)Install the LiquidCrystal_I2C library in the Arduino IDE.

3)Use the provided I2C scanner sketch to confirm the LCD address (0x27 or 0x3F) and update the code if needed.

4)Upload the sketch to your Arduino Uno.

5)Open the Serial Monitor (9600 baud) for debugging if issues arise.

* Usage:
The LCD will display "Time Alarm" on Line 1 and the current time on Line 2.

LED0 will light up from 8:00 AM to 3:00 PM.

The alarm (buzzer and LED1) triggers for 5 seconds every hour on the hour.

To set the time:
Press button1Pin (pin 12) to enter hours setup, keep pressing to increment hours (0-23).

Press button2Pin (pin 13) to enter minutes setup, keep pressing again to increment minutes (0-59).

Setup mode exits after 3 seconds of no button presses.

* Code Structure:
The sketch uses global variables for time data and includes functions for reading the RTC (readRTC), displaying time (displayTime), handling the alarm (buzzerPlay), and managing button inputs (checkButtons).

Custom BCD-to-decimal and decimal-to-BCD conversions ensure compatibility with DS3231 registers.

* Known Issues and Troubleshooting:
If the time is garbled, verify the bcdToDec function and I2C connections.

If setup mode doesn’t work, check button wiring and debouncing (adjust delay(200) if needed).

Use Serial Monitor for debug messages if I2C write errors occur.

