// Include required libraries for NFC communication
#include <PN5180.h>          // Base library for PN5180 NFC reader
#include <PN5180ISO15693.h>  // Specific library for ISO15693 protocol support

// Define pins based on the board type
#if defined(ARDUINO_AVR_UNO) || defined(ARDUINO_AVR_MEGA2560) || defined(ARDUINO_AVR_NANO)
// Pin definitions for Arduino boards
#define PN5180_NSS 10  // Chip select pin
#define PN5180_BUSY 9  // Busy status pin
#define PN5180_RST 7   // Reset pin
#elif defined(ARDUINO_ARCH_ESP32)
// Pin definitions for ESP32 boards
#define PN5180_NSS 16
#define PN5180_BUSY 5
#define PN5180_RST 17
#else
#error Please define your pinout here!
#endif

// Initialize NFC reader with defined pins
PN5180ISO15693 nfc(PN5180_NSS, PN5180_BUSY, PN5180_RST);

void setup() {
  // Initialize serial communication
  Serial.begin(115200);

  // Print welcome message and available commands
  Serial.println(F("PN5180 AFI Read/Write Demo"));
  Serial.println(F("Commands:"));
  Serial.println(F("R - Read AFI"));
  Serial.println(F("W - Write AFI"));
  Serial.println(F("X - Cancel operation"));

  // Initialize NFC reader
  nfc.begin();
  nfc.reset();
  nfc.setupRF();
}

// Function to print tag's UID in hex format with colons
void printUID(uint8_t *uid) {
  Serial.print(F("UID: "));
  for (int i = 0; i < 8; i++) {
    Serial.print(uid[7 - i], HEX);  // Print in reverse order as per ISO15693 standard
    if (i < 7) Serial.print(":");   // Add colon separator between bytes
  }
  Serial.println();
}

// Function to wait for a tag to be presented
// Returns true if tag is found, false if operation is cancelled
bool waitForTag(uint8_t *uid) {
  Serial.println(F("Waiting for tag... (Press X to cancel)"));
  while (true) {
    // Check for cancel command
    if (Serial.available()) {
      char input = Serial.read();
      while (Serial.available()) Serial.read();  // Clear buffer
      if (input == 'X' || input == 'x') {
        Serial.println(F("Operation cancelled"));
        return false;
      }
    }

    // Try to detect tag
    if (nfc.getInventory(uid) == ISO15693_EC_OK) {
      printUID(uid);
      return true;
    }
    delay(100);  // Small delay to prevent CPU hogging
  }
}

// Function to read AFI value from tag
void readAFIValue() {
  uint8_t uid[8], afi;

  // Wait for tag to be presented
  if (!waitForTag(uid)) {
    return;
  }

  // Try to read AFI value
  if (nfc.readAFI(uid, &afi) == ISO15693_EC_OK) {
    Serial.print(F("Current AFI: "));
    Serial.println(afi, HEX);
  } else {
    Serial.println(F("Failed to read AFI value"));
  }
}

// Function to read user input from Serial
// Returns complete string without newline characters
String readSerialInput() {
  String input = "";

  // Wait for data to be available
  while (!Serial.available()) {
    delay(10);
  }

  // Read characters until newline
  while (Serial.available()) {
    char c = Serial.read();
    if (c == '\n' || c == '\r') {
      if (input.length() > 0) {
        break;
      }
    } else {
      input += c;
    }
    delay(2);  // Small delay for reliable serial reading
  }

  while (Serial.available()) Serial.read();  // Clear remaining buffer
  return input;
}

// Function to write new AFI value to tag
void writeAFIValue() {
  uint8_t uid[8];

  Serial.println(F("Enter new AFI value (in hex, e.g., CC):"));
  String afiInput = readSerialInput();

  // Validate input is valid hexadecimal
  for (unsigned int i = 0; i < afiInput.length(); i++) {
    if (!isHexadecimalDigit(afiInput.charAt(i))) {
      Serial.println(F("Invalid hex value entered. Operation cancelled."));
      return;
    }
  }

  // Convert hex string to byte
  uint8_t afiValue = (uint8_t)strtol(afiInput.c_str(), NULL, 16);

  // Wait for tag
  if (!waitForTag(uid)) {
    return;
  }

  // Try to write new AFI value
  if (nfc.writeAFI(uid, afiValue) == ISO15693_EC_OK) {
    Serial.println(F("AFI value written successfully"));
    delay(100);  // Small delay before next operation
  } else {
    Serial.println(F("Failed to write AFI value"));
  }
}

// Function to read single command character from Serial
char readCommand() {
  String input = "";

  // Wait for input
  while (!Serial.available()) {
    delay(10);
  }

  // Read until newline, ignoring CR/LF
  while (Serial.available()) {
    char c = Serial.read();
    if (c != '\n' && c != '\r') {
      input += c;
    }
    delay(2);
  }

  // Clear any remaining characters
  while (Serial.available()) {
    Serial.read();
  }

  // Return first character or null if empty
  return input.length() > 0 ? input[0] : '\0';
}

// Main program loop
void loop() {
  Serial.println(F("\nEnter command (R/W):"));
  char command = readCommand();

  // Process command
  switch (command) {
    case 'R':
    case 'r':
      readAFIValue();
      break;
    case 'W':
    case 'w':
      writeAFIValue();
      break;
    case '\0':  // Empty input
      break;
    default:
      Serial.println(F("Invalid command. Use 'R' for read or 'W' for write."));
      break;
  }

  delay(100);  // Small delay before next command
}