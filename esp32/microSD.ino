// Code primarily from https://github.com/witnessmenow/ESP32-Cheap-Yellow-Display/blob/main/Examples/Basics/3-SDCardTest/3-SDCardTest.ino
// This code simply prints an example of 15 JSON Lines to a file called 'log.txt' on the microSD card inserted to the ESP32 CYD

#include "FS.h"
#include "SD.h"
#include "SPI.h"

const char* filename = "/log.txt";

/*
  Usage: writeFile(SD, "/hello.txt", "Hello ");
*/
bool writeFile(fs::FS &fs, const char * path, const char * message) {
  Serial.printf("Writing file: %s\n", path);

  File file = fs.open(path, FILE_WRITE);
  if (!file) {
    Serial.println("Failed to open file for writing");
    return false;
  }
  if (file.print(message)) {
    Serial.println("File written");
  } else {
    Serial.println("Write failed");
  }
  file.close();
  return true;
}

/*
  Usage: appendFile(SD, "/hello.txt", "World!\n");
*/
void appendFile(fs::FS &fs, const char * path, const char * message) {
  Serial.printf("Appending to file: %s\n", path);

  File file = fs.open(path, FILE_APPEND);
  if (!file) {
    Serial.println("Failed to open file for appending");  
    if (!writeFile(SD, filename, "")) {
      return;
    }
    file = fs.open(path, FILE_APPEND);
  }
  if (file.print(message)) {
    Serial.println("Message appended");
  } else {
    Serial.println("Append failed");
  }
  file.close();
}

/*
  Usage: deleteFile(SD, "/foo.txt");
*/
void deleteFile(fs::FS &fs, const char * path) {
  Serial.printf("Deleting file: %s\n", path);
  if (fs.remove(path)) {
    Serial.println("File deleted");
  } else {
    Serial.println("Delete failed");
  }
}

void readFile(fs::FS &fs, const char * path) {
  Serial.printf("Reading file: %s\n", path);

  File file = fs.open(path);
  if (!file) {
    Serial.println("Failed to open file for reading");
    return;
  }

  Serial.print("Read from file: ");
  while (file.available()) {
    Serial.write(file.read());
  }
  file.close();
}

void setup() {
  Serial.begin(115200);

  SPIClass spi = SPIClass(VSPI);

  if (!SD.begin(SS, spi, 80000000)) {
    Serial.println("Card Mount Failed");
    return;
  }
  uint8_t cardType = SD.cardType();

  if (cardType == CARD_NONE) {
    Serial.println("No SD card attached");
    return;
  }

  static short int i = 0;

  char* text = "{\"mode\":\"ambient\",\"building\":\"COE\",\"room_number\":306,\"unix_timestamp\":1678886400,\"temperature\":22.5}\n";
  for (i; i < 15; i++) {
    appendFile(SD, filename, text);
  }
  //deleteFile(SD, filename);

}

void loop() {

}