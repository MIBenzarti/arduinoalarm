#include <Wire.h>         // I2C library for DS3231 and LCD
#include <LiquidCrystal_I2C.h> // I2C LCD library

// Global variables
byte seconds, minutes, hours;
byte localHour, localMinute;
char timeString[16] = "00:00";  // Buffer for time display
char debugStr[16] = "H00 M00 S00";  // Debug for raw BCD
bool inSetupMode = false;        // Flag for setup mode
byte setupTarget = 0;            // 1 for hours, 2 for minutes
unsigned long lastButtonPress = 0; // Timer for button press duration and inactivity
unsigned long lastIncrement = 0;  // Timer for increment rate
const unsigned long SETUP_TIMEOUT = 3000;       // 3 seconds timeout for setup mode (inactivity)
const unsigned long INCREMENT_DELAY = 200;      // 200ms delay between increments

// I2C LCD configuration (adjust address if needed, e.g., 0x3F)
LiquidCrystal_I2C lcd(0x27, 16, 2);  // Address 0x27, 16 columns, 2 rows

// Pin definitions
const int led0Pin = 8;    // LED for 8 AM-3 PM
const int led1Pin = 9;    // LED for alarm
const int buzzerPin = 10; // Buzzer for alarm
const int button1Pin = 12; // Button 1 for hours increment
const int button2Pin = 13; // Button 2 for minutes increment

// RTC I2C address (DS3231)
const byte RTC_ADDRESS = 0x68;

// Custom message
const char message[] = "School Alarm";

void setup() {
  // Initialize pins
  pinMode(led0Pin, OUTPUT);
  pinMode(led1Pin, OUTPUT);
  pinMode(buzzerPin, OUTPUT);
  pinMode(button1Pin, INPUT_PULLUP);
  pinMode(button2Pin, INPUT_PULLUP);

  // Initialize I2C LCD
  lcd.init();  // Initialize the LCD
  lcd.backlight();  // Turn on backlight
  lcd.clear();
  lcd.noCursor();

  // Initialize I2C
  Wire.begin();

  // Initial display
  lcd.setCursor(0, 0);
  lcd.print(message);
  lcd.setCursor(0, 1);
  lcd.print("Starting...");
  delay(2000);  // 2-second delay

  // Optional: Enable serial debugging for troubleshooting
  Serial.begin(9600);
}

void buzzerPlay() {
  digitalWrite(buzzerPin, HIGH);
  digitalWrite(led1Pin, HIGH);  // LED1 on during alarm
  delay(5000);                  // 5-second delay
  digitalWrite(buzzerPin, LOW);
  digitalWrite(led1Pin, LOW);   // LED1 off after alarm
}

void readRTC() {
  Wire.beginTransmission(RTC_ADDRESS);
  Wire.write(0x00);  // Start from seconds register
  Wire.endTransmission();

  Wire.requestFrom(RTC_ADDRESS, 3);  // Request 3 bytes (seconds, minutes, hours)
  seconds = Wire.read() & 0x7F;      // Mask to get BCD seconds (0-59)
  minutes = Wire.read() & 0x7F;      // Mask to get BCD minutes (0-59)
  hours = Wire.read() & 0x3F;        // Mask to get BCD hours (24-hour, 0-23)

  localHour = bcdToDec(hours);       // Convert BCD to decimal
  localMinute = bcdToDec(minutes);   // Convert BCD to decimal
  // Debug: Store raw BCD for verification (masked)
  sprintf(debugStr, "H%02X M%02X S%02X", hours, minutes, seconds);
}

byte bcdToDec(byte bcd) {
  return ((bcd >> 4) * 10) + (bcd & 0x0F);  // Custom BCD to decimal conversion
}

byte decToBcd(byte dec) {
  return ((dec / 10) << 4) + (dec % 10);  // Convert decimal to BCD
}

void writeRTC(byte reg, byte value) {
  Wire.beginTransmission(RTC_ADDRESS);
  Wire.write(reg);  // Register address (e.g., 0x02 for hours, 0x01 for minutes)
  Wire.write(value);  // BCD value to write
  byte error = Wire.endTransmission();
  if (error != 0) {
    Serial.print("I2C Write Error: ");
    Serial.println(error);
  }
}

void displayTime() {
  sprintf(timeString, "%02d:%02d", localHour, localMinute);  // HH:MM format
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(message);  // "School Alarm" on line 1
  lcd.setCursor(0, 1);
  lcd.print(timeString);  // Current time on line 2
  }

void displaySetup() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(message);  // "School Alarm" on line 1
  lcd.setCursor(0, 1);
  if (setupTarget == 1) {
    lcd.print("Set Hour: ");
    lcd.print(localHour);
  } else if (setupTarget == 2) {
    lcd.print("Set Min:  ");
    lcd.print(localMinute);
  }
  lcd.setCursor(9, 1);
  lcd.print(debugStr);    // Show raw BCD debug
}

void checkButtons() {
  unsigned long currentTime = millis();
  bool button1State = digitalRead(button1Pin) == LOW;
  bool button2State = digitalRead(button2Pin) == LOW;

  if (!inSetupMode) {
    // Enter setup mode on first press
    if (button1State) {
      inSetupMode = true;
      setupTarget = 1;  // Hours setup
      lastButtonPress = currentTime;  // Start inactivity timer
      displaySetup();
    } else if (button2State) {
      inSetupMode = true;
      setupTarget = 2;  // Minutes setup
      lastButtonPress = currentTime;  // Start inactivity timer
      displaySetup();
    }
  } else {
    // Continuous increment while button is held
    if (button1State && (currentTime - lastIncrement) >= INCREMENT_DELAY) {
      if (setupTarget == 1) {  // Hours
        localHour = (localHour + 1) % 24;  // Wrap around 0-23
        writeRTC(0x02, decToBcd(localHour));  // Write new hour in BCD
        Serial.print("Setting Hour: ");
        Serial.println(localHour);
      }
      lastIncrement = currentTime;  // Reset increment timer
      displaySetup();
    } else if (button2State && (currentTime - lastIncrement) >= INCREMENT_DELAY) {
      if (setupTarget == 2) {  // Minutes
        localMinute = (localMinute + 1) % 60;  // Wrap around 0-59
        writeRTC(0x01, decToBcd(localMinute));  // Write new minute in BCD
        Serial.print("Setting Minute: ");
        Serial.println(localMinute);
      }
      lastIncrement = currentTime;  // Reset increment timer
      displaySetup();
    }

    // Exit setup mode if no button pressed for 3 seconds
    if (!button1State && !button2State && (currentTime - lastButtonPress) >= SETUP_TIMEOUT) {
      inSetupMode = false;
      setupTarget = 0;
      displayTime();  // Return to normal time display
    } else if (button1State || button2State) {
      lastButtonPress = currentTime;  // Reset inactivity timer if a button is pressed
    }
  }
}

void loop() {
  readRTC();               // Read current time from DS3231/DS3232

  if (!inSetupMode) {
    displayTime();         // Display time if not in setup mode
  }

  checkButtons();          // Check for button presses to enter/exit setup mode

  // LED0: On from 8 AM to 3 PM (8:00 to 14:59)
  digitalWrite(led0Pin, (localHour >= 8 && localHour < 15));

  // Alarm: Only trigger buzzer between 8 AM and 3 PM
  if (localMinute == 0 && seconds == 0 && localHour >= 8 && localHour < 15) {
    buzzerPlay();          // 5-second alarm
  }

  delay(1000);             // Update every second
}