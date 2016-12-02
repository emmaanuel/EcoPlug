#include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino
#include <ESP8266HTTPClient.h>
#include <ESP8266httpUpdate.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>         //https://github.com/tzapu/WiFiManager
#include <WiFiUdp.h>
#include <RFM69.h>    //get it here: https://www.github.com/lowpowerlab/rfm69
#include <SPI.h>
#include <Volet.h>
#include <ECOCommons.h>
#include <RestClient.h>
#include <NtpTime.h>
#include <LibTeleinfo.h>
#include <SimpleTimer.h>
#include <SoftwareSerial.h>

#define EDF_PIN D1
#define GazPIN D3
//#define HTTP_DEBUG

#define FREQUENCY   RF69_868MHZ
#define SERIAL_BAUD   115200
#define RFM69_CS      15  // GPIO15/HCS/D8
#define RFM69_IRQ     4   // GPIO04/D2
#define RFM69_IRQN    digitalPinToInterrupt(RFM69_IRQ)
#define IS_RFM69HW    //uncomment only for RFM69HW! Leave out if you have RFM69W!

#define DEBUG(input)   {Serial.print(input);}
#define DEBUGln(input) {Serial.println(input);}

#define DAY 1
#define NIGHT 2

#define METRICS_QUEUE_SIZE 7

#define VERSION 18
#define APP_ID "BASE"
#define OTA_URL "/updates/autoupdate.php?version="

typedef struct Metric Metric;
struct Metric {
  String name;
  float value;
  unsigned long timestamp;
};

SoftwareSerial aswSerial(D1, D0, false, 256);
TInfo tinfo;
RFM69 radio;
NtpTime ntptime("0.fr.pool.ntp.org");
byte RF_KEY[] = RF433KEY;
Volet volet(D4, RF_KEY);
SimpleTimer timer;
RestClient ovhClient = RestClient(OVH_DOMAIN, 443, 1);
RestClient domoClient = RestClient(DOMO_DOMAIN, 80);
int lastDayStatus, currentDayStatus, newDayStatus;
char buff[50];
unsigned long startingTime = 0;
const long noisefilter = 500;
unsigned long currentMillis, previousGazPulseMillis = 0;
int gazPulse = 0;
byte nbmetrics = 0;
Metric metricsToSend[10];
boolean pushLock = false;
char msg[100];
char* rooms[15] = {"", "", "juliette", "salon", "jardin", "", "garage", "grenier", "rdc", "parents"};
volatile bool isGazPulse = false;

//Check for OTA update
void checkUpdate() {
  String url = "http://";
  url += DOMO_DOMAIN;
  url += OTA_URL;
  url += VERSION;
  url += "&app=";
  url += APP_ID;
  DEBUG("Check OTA update: ");
  DEBUGln(url);
  t_httpUpdate_return ret = ESPhttpUpdate.update(url);
  switch (ret) {
    case HTTP_UPDATE_FAILED:
      Serial.printf("HTTP_UPDATE_FAILD Error (%d): %s", ESPhttpUpdate.getLastError(), ESPhttpUpdate.getLastErrorString().c_str());
      break;
    case HTTP_UPDATE_NO_UPDATES:
      DEBUGln("HTTP_UPDATE_NO_UPDATES");
      break;
    case HTTP_UPDATE_OK:
      DEBUGln("HTTP_UPDATE_OK");
      break;
  }
}

//Function called by a gaz pulse interrupt
void doGazPulse() {
  isGazPulse = true;
}

// Check light to open/close the windows
void processLight(int light) {
  if (light > 5) {
    if (lastDayStatus != DAY) {
      lastDayStatus = DAY;
      volet.open();
      sendLog("STORE_OPEN_AUTO");
      DEBUGln("OPEN STORE");
    }
  } else if (light <= 20) {
    if (lastDayStatus != NIGHT) {
      lastDayStatus = NIGHT;
      volet.close();
      sendLog("STORE_CLOSE_AUTO");
      DEBUGln("CLOSE STORE");
    }
  }
}

// Send log to the domo plateform
void sendLog(char* msg) {
  char buff[255];
  String metrics = String("{\"n\":\"0\",\"msg\":\"") + msg + "\"}";
  metrics.toCharArray(buff, 255);
  DEBUGln(buff);
  String response;
  int resp = domoClient.post("/api/log", buff, &response);
  if (resp == 200) {
    DEBUGln("Log OK");
  } else {
    DEBUGln("Log KO");
    DEBUGln(response);
  }
}

// Send metrics to the OVH and domo plateform
void pushMetrics() {
  DEBUGln("GO PUSH METRICS");
  if (nbmetrics > 0) {
    char header[100];
    sprintf(header, "Authorization: Basic %s", OVHKey);
    ovhClient.setHeader(header);
    String metrics = String("[{\"metric\":\"") + metricsToSend[0].name + "\",\"value\":" + metricsToSend[0].value + ",\"timestamp\":" + metricsToSend[0].timestamp + "}";
    for (int i = 1; i < nbmetrics; i++) {
      metrics.concat(",{\"metric\":\"" + metricsToSend[i].name + "\",\"value\":" + metricsToSend[i].value + ",\"timestamp\":" + metricsToSend[i].timestamp + "}");
    }
    metrics.concat("]");
    if (metrics.length() > 1000) {
      nbmetrics = 0;
      sendLog("Metrics too long");
      return;
    }
    char buff[1000];
    metrics.toCharArray(buff, 1000);
    DEBUGln(metrics);
    String response;
    int resp = ovhClient.post("/api/put", buff, &response);
    if (resp == 204) {
      DEBUGln("OVH OK");
      nbmetrics = 0;
    } else {
      DEBUGln("OVH KO");
      DEBUGln(response);
    }
    resp = domoClient.post("/api/metrics", buff, &response);
    if (resp == 200) {
      DEBUGln("Domo OK");
      nbmetrics = 0;
    } else {
      DEBUGln("Domo KO");
      DEBUGln(response);
    }
  }
  DEBUGln("END PUSH METRICS");
}

//Check if a metric is already in queue
boolean isMetricAlreadyInQueue(String name) {
  for (int i = 0; i < nbmetrics; i++) {
    if (metricsToSend[i].name == name) {
      return true;
    }
  }
  DEBUG(name);
  DEBUGln(" metric already in queue");
  return false;
}

// Add a metric to queue
void addMetric(String name, String value) {
  if ((nbmetrics < METRICS_QUEUE_SIZE) && (!isMetricAlreadyInQueue(name))) {
    DEBUG(name);
    DEBUG(":");
    DEBUGln(value);
    DEBUGln(nbmetrics);
    unsigned long t = startingTime +  (millis() / 1000);
    Metric m;
    m.name = String(name);
    m.value = value.toFloat();
    m.timestamp = t;
    metricsToSend[nbmetrics] = m;
    nbmetrics++;
  }
}

// Process Motion event
void processMotion(int senderID, int rssi) {
  char buff[255];
  String metrics = String("{\"n\":\"") + senderID + "\",\"r\":\"" + rssi + "\"}";
  metrics.toCharArray(buff, 255);
  DEBUGln(buff);
  String response;
  int resp = domoClient.post("/api/motion", buff, &response);
  if (resp == 200) {
    DEBUGln("Motion OK");
  } else {
    DEBUGln("Motion KO");
    DEBUGln(response);
  }
}

//process Gaz consumption event
void processGazconsumption() {
  DEBUGln("GO GAZ");
  unsigned long t = startingTime +  (millis() / 1000);
  char value[4];
  sprintf(value, "%i", gazPulse);
  addMetric("home.gaz2", value);
  gazPulse = 0;
  DEBUGln("END GAZ");
}

// push Nest ambiant temp
void processNest() {
  
}

//process EDF consumption
void processEDFconsumption() {
  DEBUGln("GO EDF");
  tinfo.init();
  unsigned long EDFStartTime = millis();
  char papp[10] = "";
  char compteur[15] = "";
  aswSerial.enableRx(true);
  DEBUGln(tinfo.labelCount());
  while ((tinfo.labelCount() == 0) && ((millis() - EDFStartTime) < 2000)) {
    if (aswSerial.available() > 0) {
      char CaractereRecu = aswSerial.read() & 0x7F;
      DEBUG(CaractereRecu);
      tinfo.process(CaractereRecu);
    }
    // Has the treatment could last 2 seconds, we still checking for incoming radio message
    checkRadio();
  }
  DEBUGln("Fin de detection de trame");
  if (tinfo.labelCount() > 0) {
    DEBUGln("Lecture des trames");
    while (((tinfo.valueGet("PAPP", papp) == NULL || tinfo.valueGet("BASE", compteur) == NULL))) {
      if (aswSerial.available() > 0) {
        char CaractereRecu = aswSerial.read() & 0x7F;
        tinfo.process(CaractereRecu);
      }
      // Has the treatment could last 2 seconds, we still checking for incoming radio message
      checkRadio();
    }
  }
  if (tinfo.valueGet("PAPP", papp) != NULL)
    addMetric("home.edf.power", String(papp));
  if (tinfo.valueGet("BASE", compteur) != NULL)
    addMetric("home.edf.hc", String(compteur));
  aswSerial.enableRx(false);
  DEBUGln("END EDF");
}

// decode RFM69 message
void decodeRFM69(int senderid, int targetid, char *data, int len, int rssi) {
  DEBUGln(data);
  if (strlen(data) > 0) {
    String msg(data);
    char dlm = '|';
    int cnt = 0;
    String tab[10];
    int idx = msg.indexOf(dlm);
    while ( idx >= 0 ) {
      if (cnt < 10) {
        tab[cnt++] = msg.substring(0, idx);
        if (msg.length() > idx) {
          msg = msg.substring(idx + 1);
        } else {
          break;
        }
      } else {
        break;
      }
      idx = msg.indexOf(dlm);
    }
    if (idx == -1) {
      tab[cnt] = msg;
    }
    unsigned long t = startingTime +  (millis() / 1000);
    if (tab[0] == "T") {
      if (tab[1] != "") {
        String room = String("home.temp.");
        room.concat(rooms[senderid]);
        addMetric(room, tab[1]);
      }
      if (tab[2] != "") {
        String room = String("home.rh.");
        room.concat(rooms[senderid]);
        addMetric(room, tab[2]);
      }
      if (tab[3] != "") {
        String room = String("home.light.");
        room.concat(rooms[senderid]);
        addMetric(room, tab[3]);
        processLight(tab[3].toInt());
      }
      if (tab[4] != "") {
        String room = String("home.volt.");
        room.concat(rooms[senderid]);
        addMetric(room, tab[4]);
      }
      
    } else if (tab[0] == "E") {
      if (tab[1] == "MOTION") {
        processMotion(senderid, rssi);
      }
    }
  }
}

// function called whend EDF label is detected
void DataCallback(ValueList * me, uint8_t  flags)
{
  // compteur de secondes basique
  DEBUG(millis() / 1000);
  DEBUG(F("\t"));

  if (flags & TINFO_FLAGS_ADDED)
    DEBUG(F("NEW -> "));

  if (flags & TINFO_FLAGS_UPDATED)
    DEBUG(F("MAJ -> "));

  // Display values
  DEBUG(me->name);
  DEBUG("=");
  DEBUGln(me->value);
}

//Just print amount of free Memory
void printMemory() {
  Serial.println( ESP.getFreeHeap(), DEC);
}

// check if a radio message has arrived
void checkRadio() {
  if (radio.receiveDone()) {
    sprintf(msg, "%s", (char *) radio.DATA);
    decodeRFM69(radio.SENDERID, radio.TARGETID, msg, radio.DATALEN, radio.RSSI);
    if (radio.ACKRequested())
    {
      radio.sendACK();
      DEBUGln(" - ACK sent.");
    }
  }
}

// check if a gaz pulse has arrived
void checkGaz() {
  if (isGazPulse) {
    currentMillis = millis();
    if (currentMillis - previousGazPulseMillis >= noisefilter) {
      previousGazPulseMillis = currentMillis;
      gazPulse = gazPulse + 1;
      DEBUGln("GAZ RISING");
    }
    isGazPulse = false;
  }
}




void setup() {
  aswSerial.begin(1200);
  Serial.begin(115200);
  pinMode(D3, OUTPUT);

  // Connect the wifi Network
  WiFiManager wifiManager;
  wifiManager.autoConnect("AutoConnectAP");
  DEBUGln("connected...yeey :)");

  // Check if the is an update to do
  checkUpdate();

  // Get current and starting time from NTP server
  unsigned long ntpTime = ntptime.getNtpTime();
  DEBUGln(ntpTime);
  startingTime = ntpTime - (unsigned long) (millis() / 1000);

  // init Radio module
  radio = RFM69(RFM69_CS, RFM69_IRQ, true, RFM69_IRQN);
  radio.initialize(FREQUENCY, 1, NETWORKID);
  DEBUGln("RADIO OK");
  radio.encrypt(ENCRYPTKEY);
  radio.promiscuous(false);
  sprintf(buff, "\nTransmitting at %d Mhz...", FREQUENCY == RF69_433MHZ ? 433 : FREQUENCY == RF69_868MHZ ? 868 : 915);
  DEBUGln(buff);

  // Schedule tasks
  timer.setInterval(60 * 1000, processGazconsumption);
  timer.setInterval(45 * 1000, processEDFconsumption);
  timer.setInterval(20 * 1000, pushMetrics);
  timer.setInterval(20 * 1000, printMemory);
  timer.setInterval(300 * 1000, checkUpdate); // toutes les 5min
  timer.setInterval(120 * 1000, processNest); // toutes les 2min

  //Declare Gaz interrupt
  attachInterrupt(D3, doGazPulse, RISING);

  // Init teleinfo
  tinfo.init();
  tinfo.attachData(DataCallback);
  aswSerial.enableRx(false);

  sendLog("Starting");
}

void loop() {
  timer.run();
  checkRadio();
  checkGaz();
}
