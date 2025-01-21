// Här finns bra exempel på hur vi kan arbeta med datum och tid: https://docs.arduino.cc/tutorials/uno-r4-minima/rtc/

#include <WiFiS3.h>

/* 
Öppna Arduino IDE.
Gå till Sketch > Include Library > Manage Libraries....
Sök efter "TimeLib" av Michael Margolis.
Klicka på "Install".
*/
#include <TimeLib.h>

#include "ArduinoGraphics.h"
#include "Arduino_LED_Matrix.h"

#define SECRET_SSID "WIFI NAMN HÄR"
#define SECRET_PASS "LÖSENORD HÄR"
#define MAX_ATTEMPTS 5

ArduinoLEDMatrix matrix;

const char* ssid = SECRET_SSID;
const char* pass = SECRET_PASS;
IPAddress ntpServer(162, 159, 200, 123);
const int NTP_PACKET_SIZE = 48;

int status = WL_IDLE_STATUS;

static unsigned long epoch = 0;
unsigned int localPort = 2390;

byte packetBuffer[NTP_PACKET_SIZE];

WiFiUDP Udp;

static bool wifiConnected = false;
static int attempts = 0;

// Tidszonsförskjutning för Sverige (UTC+1)
const int TZ_SWEDEN_STD = 1;
// Tidszonsförskjutning för Sverige under sommartid (UTC+2)
const int TZ_SWEDEN_DST = 2;

// Funktion för att hämta tiden från NTP och ställa in TimeLib
void getNtpTime() {
  IPAddress ntpServerIP;
  WiFi.hostByName("pool.ntp.org", ntpServerIP);

  sendNTPpacket(ntpServerIP); // Skickar NTP-paket
  delay(1000);
  if (Udp.parsePacket()) {
    Udp.read(packetBuffer, NTP_PACKET_SIZE);
    unsigned long secsSince1900;
    // Konverterar de fyra byten som representerar tiden
    secsSince1900 = (packetBuffer[40] << 24) | (packetBuffer[41] << 16) | (packetBuffer[42] << 8) | packetBuffer[43];
    setTime(secsSince1900 - 2208988800UL); // Ställer in TimeLib med Unix-tid
  }
}

// Funktion för att kontrollera om det är sommartid i Sverige med TimeLib
bool isDaylightSavingTime() {
  // TimeLib har inbyggt stöd för sommartid via timezone-bibliotek,
  // men för enkelhetens skull (utan extra bibliotek) använder vi en förenklad kontroll.
  // För mer exakt hantering, använd ett timezone-bibliotek.

  if (month() > 3 && month() < 10) {
    return true;
  } else if (month() == 3) {
      if (day() > 24) return true; // Grov uppskattning
  } else if (month() == 10) {
      if (day() < 28) return true; // Grov uppskattning
  }
  return false;
}

void setup() {
  wifiConnected = false;
  Serial.begin(115200); // Högre baudrate för snabbare utskrift
  matrix.begin();
}

void loop() {
  char str[256];
  Serial.println(matrix.getWriteError());
  matrix.clearWriteError();

  if (!wifiConnected) {
    setupWifi();
  } else {
      getNtpTime(); // Hämta tiden regelbundet

    int timeZoneOffset = isDaylightSavingTime() ? TZ_SWEDEN_DST : TZ_SWEDEN_STD;
    // Beräkna svensk tid
    time_t swedishTime = now() + timeZoneOffset * SECS_PER_HOUR;

    sprintf(str, "Svensk tid: %02d:%02d:%02d", hour(swedishTime), minute(swedishTime), second(swedishTime));

    writeMatrix(str);

    Serial.print("UTC-tid: ");
    Serial.print(hour());
    Serial.print(":");
    Serial.print(minute());
    Serial.print(":");
    Serial.println(second());

    Serial.print("Svensk tid: ");
    Serial.print(hour(swedishTime));
    Serial.print(":");
    Serial.print(minute(swedishTime));
    Serial.print(":");
    Serial.println(second(swedishTime));

    delay(10000); // Uppdatera tiden var 10:e sekund
  }
}

void writeMatrix(const char* text) {
  Serial.println(matrix.getWriteError());
  matrix.clearWriteError();

  matrix.clear();
  matrix.beginDraw();

  matrix.stroke(0xFFFFFFFF);
  matrix.textScrollSpeed(50);

  matrix.textFont(Font_5x7);
  matrix.beginText(0, 1, 0xFFFFFF);
  matrix.println(text);
  matrix.endText(SCROLL_LEFT);

  matrix.endDraw();
  delay(1000);
}

void setupWifi() {
  while (!Serial);

  //if (WiFi.status() == WL_NO_MODULE) {
  //  Serial.println("Kommunikation med WiFi-modulen misslyckades!");
  //  while (true);
  //}

  while (status != WL_CONNECTED && attempts <= MAX_ATTEMPTS) {
    Serial.print("Försöker ansluta till SSID: ");
    Serial.println(ssid);

    char str[256];
    sprintf(str, "      Ansluter (%d/%d)      ", ++attempts, MAX_ATTEMPTS);
    writeMatrix(str);

    status = WiFi.begin(ssid, pass);
    delay(5000);
  }

  Udp.begin(localPort);
  wifiConnected = true;
}

unsigned long sendNTPpacket(IPAddress& address) {
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  packetBuffer[12] = 49;
  packetBuffer[13] = 0x4E;
  packetBuffer[14] = 49;
  packetBuffer[15] = 52;

  Udp.beginPacket(address, 123);
  Udp.write(packetBuffer, NTP_PACKET_SIZE);
  Udp.endPacket();
}
