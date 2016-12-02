#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>

#include <ESP8266HTTPClient.h>
#include <ESP8266httpUpdate.h>
#include <WiFiUdp.h>
#include <aJSON.h>

#define USE_SERIAL Serial
#define WIFI_SSID "HomeNetwork2"
#define WIFI_PWD "0emmanueletveronique0"

const int version = 6;
const int interruptPIN = 4;
const int ledPIN = 5;
const long noisefilter = 70;
const long interval = 60000;
unsigned int localPort = 2390;      // local port to listen for UDP packets
char* ovhHost = "opentsdb.iot.runabove.io";
const char* TOKEN_ID = "ebqaacayeebwd";
const char* TOKEN_KEY = "2zEuCUD8n_nc3cqdaOnkBR_u";

IPAddress timeServerIP;
const char* ntpServerName = "0.fr.pool.ntp.org";
const int NTP_PACKET_SIZE = 48; // NTP time stamp is in the first 48 bytes of the message
byte packetBuffer[ NTP_PACKET_SIZE]; //buffer to hold incoming and outgoing packets

WiFiUDP udp; // A UDP instance to let us send and receive packets over UDP

int pulse = 0;
unsigned long previousMillis = 0;
unsigned long startingTime = 0;
HTTPClient http;

// send an NTP request to the time server at the given address
unsigned long sendNTPpacket(IPAddress& address)
{
  //Serial.println("sending NTP packet...");
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12]  = 49;
  packetBuffer[13]  = 0x4E;
  packetBuffer[14]  = 49;
  packetBuffer[15]  = 52;

  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:
  udp.beginPacket(address, 123); //NTP requests are to port 123
  udp.write(packetBuffer, NTP_PACKET_SIZE);
  udp.endPacket();
}

void checkUpdate() {
  String url = "http://domo.emmaanuel.com/updates/autoupdate.php?version=";
  url +=  version;
  USE_SERIAL.println(url);
  t_httpUpdate_return ret = ESPhttpUpdate.update(url);
  //t_httpUpdate_return  ret = ESPhttpUpdate.update("https://server/file.bin");

  switch (ret) {
    case HTTP_UPDATE_FAILED:
      USE_SERIAL.printf("HTTP_UPDATE_FAILD Error (%d): %s", ESPhttpUpdate.getLastError(), ESPhttpUpdate.getLastErrorString().c_str());
      break;

    case HTTP_UPDATE_NO_UPDATES:
      USE_SERIAL.println("HTTP_UPDATE_NO_UPDATES");
      break;

    case HTTP_UPDATE_OK:
      USE_SERIAL.println("HTTP_UPDATE_OK");
      break;
  }
}

unsigned long getNTPTime() {
  udp.begin(localPort);
  WiFi.hostByName(ntpServerName, timeServerIP);
  sendNTPpacket(timeServerIP); // send an NTP packet to a time server
  // wait to see if a reply is available
  delay(1000);
  int cb = 0;
  while (!cb) {
    cb = udp.parsePacket();
    if (!cb) {
      Serial.println("no packet yet");
    }
    else {
      udp.read(packetBuffer, NTP_PACKET_SIZE); // read the packet into the buffer
      unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
      unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);
      unsigned long secsSince1900 = highWord << 16 | lowWord;
      const unsigned long seventyYears = 2208988800UL;
      unsigned long epoch = secsSince1900 - seventyYears;
      return epoch;
    }

  }

}

void sendMetric() {
  WiFiClientSecure client;
  if (!client.connect(ovhHost, 443)) {
    USE_SERIAL.println("connection failed");
    return;
  }

  unsigned long t = startingTime +  (millis() / 1000);
  String payload = String("[{\"metric\":\"home.gaz2\",\"timestamp\":")
                   + t
                   + ",\"value\":"
                   + pulse
                   + "}]";
  if (pulse > 0){
    digitalWrite(ledPIN, HIGH);
    delay(200);
    digitalWrite(ledPIN, LOW);
  }
  pulse = 0;
  String request = String("POST /api/put HTTP/1.1\r\n\Host: opentsdb.iot.runabove.io\r\nUser-Agent: ESP8266HTTPClient\r\nConnection: close\r\nAuthorization: Basic ZWJxYWFjYXllZWJ3ZDoyekV1Q1VEOG5fbmMzY3FkYU9ua0JSX3U=\r\nContent-Length: ")
                   + payload.length()
                   + "\r\n\r\n"
                   + payload;
  USE_SERIAL.println(request);
  client.print(request);
  bool status = false;
  while (client.connected()) {
    if ( client.available() )
    {
      String str = client.readStringUntil('\r');
      Serial.print(str);
      if (str.indexOf("HTTP/1.1 204 No Content") == 0)
        status = true;
    }
  }

  digitalWrite(ledPIN, HIGH);
  if (status) {
    delay(200);
  } else {
    delay(1000);
  }
  digitalWrite(ledPIN, LOW);
}

void doPulse() {
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= noisefilter) {
    previousMillis = currentMillis;
    pulse = pulse + 1;
    USE_SERIAL.print("RISING");
  }
  digitalWrite(ledPIN, HIGH);
  USE_SERIAL.println(currentMillis - previousMillis);
}

void setup() {

  USE_SERIAL.begin(115200);
  // USE_SERIAL.setDebugOutput(true);
  pinMode(interruptPIN, OUTPUT);
  pinMode(ledPIN, OUTPUT);
  digitalWrite(ledPIN, LOW);
  USE_SERIAL.println();
  USE_SERIAL.println();
  USE_SERIAL.println();

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PWD);
  while (WiFi.status() != WL_CONNECTED) {
    USE_SERIAL.println("Waiting for connection");
    digitalWrite(ledPIN, HIGH);
    delay(250);
    digitalWrite(ledPIN, LOW);
    delay(250);
  }
  USE_SERIAL.println("CONNECTED");
  for (uint8_t t = 10; t > 0; t--) {
    digitalWrite(ledPIN, HIGH);
    delay(30);
    digitalWrite(ledPIN, LOW);
    delay(30);
  }
  USE_SERIAL.print("Version: ");
  USE_SERIAL.println( version );
  digitalWrite(ledPIN, HIGH);
  checkUpdate();
  digitalWrite(ledPIN, LOW);
  unsigned long ntpTime = getNTPTime();
  startingTime = ntpTime - (unsigned long) (millis() / 1000);
  USE_SERIAL.println(startingTime);

  attachInterrupt(digitalPinToInterrupt(interruptPIN), doPulse, RISING);
}

void loop() {
  delay(interval);
  USE_SERIAL.print("RUNNING ");
  USE_SERIAL.println(pulse);
  sendMetric();
}






