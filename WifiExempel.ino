#include <WiFiS3.h>

#define SECRET_SSID "WIFI NAMN HÄR"
#define SECRET_PASS "LÖSENORD HÄR"
#define MAX_ATTEMPTS 5

const char ssid[] = SECRET_SSID;
const char pass[] = SECRET_PASS;

int status = WL_IDLE_STATUS;  // Status för Wi-Fi-anslutningen

bool wifi_connected = false;
int attempts = 0;

void setup() {
  Serial.begin(9600);  // Initiera seriekommunikation

  // Försök att ansluta till Wi-Fi
  setup_wifi();
}

void loop() {

}

void setup_wifi() {
  while (status != WL_CONNECTED && attempts <= MAX_ATTEMPTS) {
    Serial.print("Ansluter till ");
    Serial.println(ssid);

    // Anslut till Wi-Fi-nätverk (lösenordskyddat)
    status = WiFi.begin(ssid, pass);

    delay(10000);  // Vänta 5 sekunder
    attempts++;
  }

  wifi_connected = status == WL_CONNECTED;

  if (wifi_connected) {

    Serial.println("setup_wifi(): WiFi anslutet");
    char str[256];
    byte mac[6];
    WiFi.macAddress(mac);
    sprintf(str, "MAC address: %02X:%02X:%02X:%02X:%02X:%02X", mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
    Serial.println(str);
    WiFi.end();
    
  } else {
    Serial.print("Kunde inte ansluta till Wi-Fi");
  }
}
