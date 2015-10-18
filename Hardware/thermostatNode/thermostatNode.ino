#include <RFM69.h>    //get it here: https://www.github.com/lowpowerlab/rfm69
#include <SPI.h>
#include <LowPower.h> //get library from: https://github.com/lowpowerlab/lowpower
#include <ECOCommons.h>
#include "U8glib.h"

#define NODEID        NODE_THERMOSTAT    //unique for each node on same network
#define NETWORKID     100  //the same on all nodes that talk to each other
#define FREQUENCY   RF69_868MHZ
//#define IS_RFM69HW    //uncomment only for RFM69HW! Leave out if you have RFM69W!
#define LED           9 // Moteinos have LEDs on D9
#define SERIAL_BAUD   57600
#define RELAY 4

//#define SERIAL_EN                //comment out if you don't want any serial output

#ifdef SERIAL_EN
#define DEBUG(input)   {Serial.print(input);}
#define DEBUGln(input) {Serial.println(input);}
#else
#define DEBUG(input);
#define DEBUGln(input);
#endif


RFM69 radio;
bool promiscuousMode = true; //sniff all packets on the same network
char buff[50];
char ligne1[20] = "Cible: ?";
char ligne2[20] = "Zone: ?";
char ligne3[20] = "Temp: 22.50";
char ligne4[20] = "Status: ON";
float cible = 20;
int zone = 3;
boolean currentStatus = false;
char rooms[][15] = {"?", "Raspberry", "Juliette", "Salon", "Jardin", "Thermostat", "Garage"};
float rooms_temps[7] = {0, 0, 0, 0, 0, 0, 0};

const int pin_A = 6;
const int pin_B = 7;
int n = 0;
unsigned char encoder_A;
unsigned char encoder_B;
unsigned char encoder_A_prev = 0;
unsigned long currentTime;
unsigned long buttonLoopTime;

const int buttonpin = 5;
int buttonreading;           // the current reading from the input pin
int buttonprevious = LOW;    // the previous reading from the input pin
long buttontime = 0;         // the last time the output pin was toggled
long buttondebounce = 200;   // the debounce time, increase if the output flickers

U8GLIB_SSD1306_128X64 u8g(U8G_I2C_OPT_NO_ACK);	// Display which does not send ACK

String action = "";
String action_arg = "";

void setup() {
  delay(100);
  pinMode(LED, OUTPUT);
  pinMode(RELAY, OUTPUT);
  digitalWrite(RELAY, LOW);
#ifdef SERIAL_EN
  Serial.begin(SERIAL_BAUD);
#endif
  initRadio();
  sprintf(buff, "\nTransmitting at %d Mhz...", FREQUENCY == RF69_433MHZ ? 433 : FREQUENCY == RF69_868MHZ ? 868 : 915);
  DEBUGln(buff);
  uint8_t devID;

  //OLED SETUP
  // assign default color value
  if ( u8g.getMode() == U8G_MODE_R3G3B2 ) {
    u8g.setColorIndex(255);     // white
  }
  else if ( u8g.getMode() == U8G_MODE_GRAY2BIT ) {
    u8g.setColorIndex(3);         // max intensity
  }
  else if ( u8g.getMode() == U8G_MODE_BW ) {
    u8g.setColorIndex(1);         // pixel on
  }
  else if ( u8g.getMode() == U8G_MODE_HICOLOR ) {
    u8g.setHiColorByRGB(255, 255, 255);
  }

  updateScreen();
  pinMode(pin_A, INPUT);
  pinMode(pin_B, INPUT);
  currentTime = millis();
  buttonLoopTime = currentTime;
  Blink(LED, 500);
}


void loop() {
  handleMessage();
  checkButton();
  delay(0.1);
}

void initRadio() {
  radio.initialize(FREQUENCY, NODEID, NETWORKID);
  radio.encrypt(ENCRYPTKEY);
  radio.promiscuous(promiscuousMode);
#ifdef IS_RFM69HW
  radio.setHighPower(); //uncomment only for RFM69HW!
#endif
  radio.sleep();
}

void checkButton(void) {
  currentTime = millis();
  if (currentTime >= (buttonLoopTime + 2)) {
    encoder_A = digitalRead(pin_A);    // Read encoder pins
    encoder_B = digitalRead(pin_B);
    if ((!encoder_A) && (encoder_A_prev)) {
      // A has gone from high to low
      if (encoder_B) {
        // B is high so clockwise
        // increase the brightness, dont go over 255
        cible += 0.5;
        checkTemp();
        updateScreen();

      }
      else {
        // B is low so counter-clockwise
        // decrease the brightness, dont go below 0
        cible -= 0.5;
        checkTemp();
        updateScreen();
      }
      buttonLoopTime = currentTime;
    }
  }
  encoder_A_prev = encoder_A;     // Store value of A for next time

  //READ SWITCH BUTTON
  buttonreading = digitalRead(buttonpin);
  if (buttonreading == HIGH && buttonprevious == LOW && millis() - buttontime > buttondebounce) {
    initRadio();
    rotateZone();
    buttontime = millis();
  }
  buttonprevious = buttonreading;
}

void draw(void) {
  // graphic commands to redraw the complete screen should be placed here
  u8g.setFont(u8g_font_unifont);
  u8g.drawStr( 0, 15, ligne1);
  u8g.drawStr( 0, 30, ligne2);
  u8g.drawStr( 0, 45, ligne3);
  u8g.drawStr( 0, 60, ligne4);
}

void updateScreen() {
  char buffcible[6];
  char buffTemp[6];
  dtostrf(cible, 5, 2, buffcible);
  dtostrf(rooms_temps[zone], 5, 2, buffTemp);
  sprintf(ligne1, "Cible: %s", buffcible);
  sprintf(ligne2, "Zone: %s", rooms[zone]);
  sprintf(ligne3, "Temp: %s", (rooms_temps[zone] == 0) ? "Inconnue" : buffTemp);
  sprintf(ligne4, "Status: %s", currentStatus ? "ON" : "OFF");
  //sprintf(ligne4, "ct: %lu %d %d", nbMessage, radio._mode, radio.DATALEN);

  u8g.firstPage();
  do {
    draw();
  } while ( u8g.nextPage() );
}

void Blink(byte PIN, int DELAY_MS)
{
  digitalWrite(PIN, HIGH);
  delay(DELAY_MS);
  digitalWrite(PIN, LOW);
}

void checkTemp()
{
  if ((rooms_temps[zone] < cible) && (rooms_temps[zone] != 0)) {
    heaterOn();
  } else {
    heaterOff();
  }
}

void handleMessage()
{
  if (radio.receiveDone())
  {
    for (byte i = 0; i < radio.DATALEN; i++)
      buff[i] = (char)radio.DATA[i];
    buff[radio.DATALEN] = '\0';
    if (radio.DATALEN > 0) {
      DEBUGln(buff);
      DEBUG("From: ");
      DEBUG(radio.SENDERID);
      DEBUG(" -> ");
      DEBUG(radio.TARGETID);
      DEBUG(" : ");
      DEBUGln(rooms[radio.SENDERID]);
      String msg = String(buff);
      if ((radio.TARGETID == NODEID) && radio.ACKRequested())
      {
        radio.sendACK();
      }
      if (msg.indexOf('|') != -1) {
        String msgtype = msg.substring(0, 1);
        if (msgtype == "A") {
          String fullaction = msg.substring(msg.indexOf('|') + 1);
          DEBUGln(fullaction);
          action = fullaction.substring(0, fullaction.indexOf('|'));
          action_arg = fullaction.substring(fullaction.indexOf('|') + 1);
          action_arg = fullaction.substring(fullaction.indexOf('|') + 1);
          DEBUGln(action);
          if (action == "HEATER_ON")
            heaterOn();
          if (action == "HEATER_OFF")
            heaterOff();
          if (action == "SET_CIBLE")
            setCible(action_arg);
          if (action == "SET_ZONE")
            setZone(action_arg);
        }
        if (msgtype == "T")  {
          String t = msg.substring(msg.indexOf('|') + 1);
          t = t.substring(0, t.indexOf('|'));
          DEBUG(radio.SENDERID);
          DEBUG(":");
          DEBUGln(t);
          rooms_temps[radio.SENDERID] = t.toFloat();
        }

      }
    }
    updateScreen();
    checkTemp();
    Blink(LED, 10);
  }
}

void rotateZone() {
  zone++;
  int maxtry = 7;
  if (zone > 6) zone = 1;
  while ((rooms_temps[zone] == 0) && (maxtry > 0)) {
    zone++;
    maxtry--;
    if (zone > 6) zone = 1;
  }
  DEBUGln(zone);
  checkTemp();
  updateScreen();
}



void heaterOn() {
  DEBUGln("HEATER ON");
  currentStatus = true;
  updateScreen();
  digitalWrite(RELAY, HIGH);
}

void heaterOff() {
  DEBUGln("HEATER OFF");
  currentStatus = false;
  updateScreen();
  digitalWrite(RELAY, LOW);
}

void setCible(String str_cible) {
  cible = str_cible.toFloat();
  DEBUG("CIBLE: ");
  DEBUGln(cible);
  checkTemp();
  updateScreen();
}

void setZone(String str_cible) {
  zone = str_cible.toInt();
  DEBUG("Zone: ");
  DEBUGln(zone);
  checkTemp();
  updateScreen();
}
