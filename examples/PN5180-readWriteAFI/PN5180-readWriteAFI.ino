#include <PN5180.h>
#include <PN5180ISO15693.h>

#if defined(ARDUINO_AVR_UNO) || defined(ARDUINO_AVR_MEGA2560) || defined(ARDUINO_AVR_NANO)
#define PN5180_NSS  10
#define PN5180_BUSY 9
#define PN5180_RST  7
#elif defined(ARDUINO_ARCH_ESP32)
#define PN5180_NSS  16
#define PN5180_BUSY 5
#define PN5180_RST  17
#else
#error Please define your pinout here!
#endif

PN5180ISO15693 nfc(PN5180_NSS, PN5180_BUSY, PN5180_RST);

void setup() {
  Serial.begin(115200);
  Serial.println(F("PN5180 ISO15693 Demo"));

  nfc.begin();
  nfc.reset();
  nfc.setupRF();
}

void loop() {
  uint8_t uid[8], afi;
  if (nfc.getInventory(uid) == ISO15693_EC_OK) {
    Serial.print(F("UID: "));
    for (int i = 0; i < 8; i++) {
      Serial.print(uid[7 - i], HEX);
      if (i < 7) Serial.print(":");
    }
    Serial.println();

    nfc.readAFI(uid, &afi);
    Serial.print("Current AFI: ");
    Serial.println(afi, HEX);

    delay(1000); // Wait 1 second

    Serial.println(F("Enter new AFI value (in hex, e.g., CC):"));
    while (Serial.available() == 0) {
      // Wait for user input
    }
    String afiInput = Serial.readStringUntil('\n');
    uint8_t afiValue = (uint8_t) strtol(afiInput.c_str(), NULL, 16);

    if (nfc.writeAFI(uid, afiValue) == ISO15693_EC_OK) {
      Serial.println(F("AFI value written successfully"));
    } else {
      Serial.println(F("Failed to write AFI value"));
    }
  } else {
    Serial.println(F("No tag detected"));
  }

  delay(1000); // Wait 1 second before checking again
}