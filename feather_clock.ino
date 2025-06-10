#include <dummy.h>
#include "arduino_secrets.h"

#include <Wire.h> // Enable this line if using Arduino Uno, Mega, etc.
#include <WiFi.h>
#include <Adafruit_GFX.h>
#include "Adafruit_LEDBackpack.h"
#include "time.h"

#define DUBUG_FLAG 1

// Clock settings
const char *ntpServer = "time.nist.gov";
const long gmtOffset_sec = -18000;
const int daylightOffset_sec = 3600;

struct tm *timeinfo = (struct tm *)malloc(sizeof(struct tm));

Adafruit_7segment segmentDisplay = Adafruit_7segment();

void displayTime(int hour, int min)
{
  char s[10];
  if (hour < 12)
  {
    sprintf(s, "%02d:%02d AM", hour ? hour : 12, min);
  }
  else
  {
    sprintf(s, "%02d:%02d PM", hour, min);
  }
  Serial.println(s);

  // Calculate the Displayed Twelve Hour format
  int displayedHour = hour < 12 ? hour : hour - 12;
  displayedHour = displayedHour ? displayedHour : 12;

  if (displayedHour / 10)
  {
    segmentDisplay.writeDigitNum(0, 1, false);
  }
  else
  {
    segmentDisplay.writeDigitRaw(0, 0x00); // Show nothing
  }

  segmentDisplay.writeDigitNum(1, displayedHour % 10, false);
  segmentDisplay.drawColon(true);
  segmentDisplay.writeDigitNum(3, min / 10, false);
  segmentDisplay.writeDigitNum(4, min % 10, hour >= 12);

  segmentDisplay.writeDisplay();
}

void getAndDisplayLocalTime()
{
  if (!getLocalTime(timeinfo))
  {
    Serial.println("Failed to obtain time");
    return;
  }
  Serial.println(timeinfo);
  int hour = timeinfo->tm_hour;
  int min = timeinfo->tm_min;
  displayTime(hour, min);
}

void getTimeFromNTP()
{
  // Connect to WiFi
  Serial.printf("Connecting to %s\n", SECRET_WIFI_SSID);
  WiFi.begin(SECRET_WIFI_SSID, SECRET_WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.println(".");
  }
  Serial.printf("Connected to %s\n", SECRET_WIFI_SSID);

  //Get time from NTP
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  getAndDisplayLocalTime();

  //disconnect WiFi as it's no longer needed
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
}

void startUpSequence()
{
  for (int j = 0; j < 60; j++)
  {
    delay(100);
    displayTime(0, j);
  }

  for (int i = 0; i < 24; i++)
  {
    delay(100);
    displayTime(i, 0);
  }
}

void setup()
{
  Serial.begin(115200);
  delay(100);
  Serial.println("DBS Clock Engaged");
  segmentDisplay.begin(0x70);

  startUpSequence();
  getTimeFromNTP();
  getAndDisplayLocalTime();
}

void loop()
{
  delay(5000);
  getAndDisplayLocalTime();
}
