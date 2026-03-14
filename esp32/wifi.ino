// Connects to a wifi network and gets the unix time, converts to EST
#include <WiFi.h>
#include <time.h>
#include <secrets.h>

const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = -18000;   // EST
const int daylightOffset_sec = 3600;  // 1 hour for daylight saving

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.printf("Connecting to Wi-Fi %s ...\n", WIFI_SSID);

  // SSID and password must be inside of secrets.h
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  // try to connect to the wifi
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }

  if(WiFi.status() != WL_CONNECTED){
    Serial.println("\nFailed to connect to Wi-Fi");
    return;
  }

  Serial.println("\nWi-Fi connected!");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  // Initialize NTP
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

  Serial.println("Getting time from NTP server...");
  time_t now = time(nullptr);
  int retry = 0;
  const int retry_count = 10;

  // ensure unix time is valid, otherwise try again
  while (now < 1000000000 && retry < retry_count) { 
    delay(500);
    Serial.print(".");
    now = time(nullptr);
    retry++;
  }

  if(now < 1000000000){
    Serial.println("\nFailed to get time from NTP server");
  } else {
    Serial.println("\nUnix timestamp received!");
    Serial.print("Unix time: ");
    Serial.println(now);
  }
}

void printTime(time_t t) {
  struct tm * timeinfo;
  timeinfo = localtime(&t); // converts to local time

  char buffer[32];
  strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", timeinfo);
  Serial.println(buffer);
}

// get time every 5s and print
void loop() {
  time_t now = time(nullptr);
  printTime(now);
  delay(5000);
}