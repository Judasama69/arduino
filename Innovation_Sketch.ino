#include <Wire.h>
#include <RTClib.h>
#include <LiquidCrystal_I2C.h>
#include <SD.h>
#include <SoftwareSerial.h>

RTC_DS3231 rtc;
LiquidCrystal_I2C lcd(0x27, 16, 2); // Adjust the I2C address if necessary
const int chipSelect = 10;
SoftwareSerial qrScanner(0, 1); // RX, TX

void setup() {
  // Start Serial Monitor
  Serial.begin(9600);
  while (!Serial) {}

  // Initialize LCD
  lcd.init();
  lcd.backlight();
  lcd.print("Initializing...");

  // Initialize QR scanner
  qrScanner.begin(9600);

  // Initialize RTC
  if (!rtc.begin()) {
    lcd.setCursor(0, 1);
    lcd.print("RTC not found!");
    Serial.println("RTC not found!");
    while (1);
  }

  // Check RTC power
  if (rtc.lostPower()) {
    lcd.setCursor(0, 1);
    lcd.print("RTC reset...");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__))); // Set RTC to compile time
    Serial.println("RTC reset...");
  }

  // Initialize SD card
  if (!SD.begin(chipSelect)) {
    lcd.setCursor(0, 1);
    lcd.print("SD fail...");
    Serial.println("SD card initialization failed!");
    while (1);
  }

  lcd.setCursor(0, 1);
  lcd.print("Ready!");
  delay(2000);
  lcd.clear();

  // Instructions for setting time
  Serial.println("To set the date and time, enter the values in the following format:");
  Serial.println("YYYY MM DD HH MM SS");
  Serial.println("For example: 2025 01 31 18 30 00");
}

void loop() {
  // Check if there is serial input available to set the date and time
  if (Serial.available() > 0) {
    int year = Serial.parseInt();
    int month = Serial.parseInt();
    int day = Serial.parseInt();
    int hour = Serial.parseInt();
    int minute = Serial.parseInt();
    int second = Serial.parseInt();

    if (Serial.read() == '\n') { // Wait for the newline character
      rtc.adjust(DateTime(year, month, day, hour, minute, second));
      Serial.println("RTC date and time set successfully!");

      // Confirm the new date and time
      DateTime now = rtc.now();
      Serial.print("Current Date/Time: ");
      Serial.print(now.year(), DEC);
      Serial.print('/');
      Serial.print(now.month(), DEC);
      Serial.print('/');
      Serial.print(now.day(), DEC);
      Serial.print(" ");
      Serial.print(now.hour(), DEC);
      Serial.print(':');
      Serial.print(now.minute(), DEC);
      Serial.print(':');
      Serial.print(now.second(), DEC);
      Serial.println();
    }
  }

  // Get current time
  DateTime now = rtc.now();

  // Convert to 12-hour format
  int hour12 = now.hour();
  String period = "AM";
  if (hour12 >= 12) {
    period = "PM";
    if (hour12 > 12) hour12 -= 12;
  }
  if (hour12 == 0) hour12 = 12;

  // Display time on LCD in 12-hour format
  lcd.setCursor(0, 0);
  lcd.print("Time:");
  lcd.print(hour12 < 10 ? "0" : "");
  lcd.print(hour12);
  lcd.print(":");
  lcd.print(now.minute() < 10 ? "0" : "");
  lcd.print(now.minute());
  lcd.print(":");
  lcd.print(now.second() < 10 ? "0" : "");
  lcd.print(now.second());
  lcd.print(" ");
  lcd.print(period);

  lcd.setCursor(0, 1);
  lcd.print("Date:");
  lcd.print(now.day() < 10 ? "0" : "");
  lcd.print(now.day());
  lcd.print("/");
  lcd.print(now.month() < 10 ? "0" : "");
  lcd.print(now.month());
  lcd.print("/");
  lcd.print(now.year());

  // Read QR code data from the scanner
  if (qrScanner.available() > 0) {
    String qrData = qrScanner.readStringUntil('\n');
    Serial.println("QR Data: " + qrData); // Print the scanned QR code to the Serial Monitor
    saveQRCodeToSD(qrData, now);
  }

  delay(1000); // Update every second
}

void saveQRCodeToSD(String qrData, DateTime now) {
  String fileName = "qr_code_log.txt";
  Serial.println("Opening " + fileName + "...");

  File dataFile = SD.open(fileName, FILE_WRITE);

  if (dataFile) {
    int hour12 = now.hour();
    String period = "AM";
    if (hour12 >= 12) {
      period = "PM";
      if (hour12 > 12) hour12 -= 12;
    }
    if (hour12 == 0) hour12 = 12;

    dataFile.print("QR Code: ");
    dataFile.print(qrData);
    dataFile.print(" | Time: ");
    dataFile.print(hour12 < 10 ? "0" : "");
    dataFile.print(hour12);
    dataFile.print(":");
    dataFile.print(now.minute() < 10 ? "0" : "");
    dataFile.print(now.minute());
    dataFile.print(":");
    dataFile.print(now.second() < 10 ? "0" : "");
    dataFile.print(now.second());
    dataFile.print(" ");
    dataFile.print(period);
    dataFile.print(" | Date: ");
    dataFile.print(now.year());
    dataFile.print("/");
    dataFile.print(now.month() < 10 ? "0" : "");
    dataFile.print(now.month());
    dataFile.print("/");
    dataFile.print(now.day() < 10 ? "0" : "");
    dataFile.println(now.day());
    dataFile.close();

    Serial.println("QR code and timestamp saved to SD card.");
  } else {
    Serial.println("Error opening " + fileName);
  }
}
