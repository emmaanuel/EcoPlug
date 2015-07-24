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
#define TX_433_PIN 3
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
unsigned long sleepTime;

const int buttonpin=5;
int buttonreading;           // the current reading from the input pin
int buttonprevious = LOW;    // the previous reading from the input pin
long buttontime = 0;         // the last time the output pin was toggled
long buttondebounce = 200;   // the debounce time, increase if the output flickers

/* -------------------------------------------------------- */
/* ----                Blyss Spoofer API               ---- */
/* -------------------------------------------------------- */

/* Time constants */
const unsigned long H_TIME = 2400; // Header delay
const unsigned long T_TIME = 400;  // 1/3 frame delay
const byte nb_frames = 13; // Numbers of frames per command

/* RF signal usage macro */
#define SIG_HIGH() digitalWrite(TX_433_PIN, HIGH)
#define SIG_LOW() digitalWrite(TX_433_PIN, LOW)

U8GLIB_SSD1306_128X64 u8g(U8G_I2C_OPT_NO_ACK);	// Display which does not send ACK

/** "Rolling code" (normally avoid frame spoofing) */
byte RF_ROLLING_CODE[] = {
  0x98, 0xDA, 0x1E, 0xE6, 0x67
};


/** Transmission channels and status enumeration */
enum {
  OFF, ON,
  CH_1 = 8, CH_2 = 4, CH_3 = 2, CH_4 = 1, CH_5 = 3, CH_ALL = 0,
  CH_A = 0, CH_B = 1, CH_C = 2, CH_D = 3
};

/** Key ID to spoof */
byte RF_KEY[] = RF433KEY;

/** Frame-data buffer (key ID + status flag + rolling code + token */
byte RF_BUFFER[7];
String action = "";
String action_arg = "";
/* ------------------------------------------------------ */

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

  // BLYSS Setup
  set_key(RF_BUFFER, RF_KEY, true);
  set_global_channel(RF_BUFFER, CH_D);
  pinMode(TX_433_PIN, OUTPUT);
  SIG_LOW();

  updateScreen();
  pinMode(pin_A, INPUT);
  pinMode(pin_B, INPUT);
  currentTime = millis();
  buttonLoopTime = currentTime; 
  sleepTime = currentTime;
}
void loop() {


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
          if (action == "STORE_OPEN")
            storeOpen();
          if (action == "STORE_CLOSE")
            storeClose();
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
          updateScreen();
        }

      }
      Blink(LED, 3);
      sleepTime = millis();
    }
  }
  currentTime = millis();
  if(currentTime >= (buttonLoopTime + 2)){
    checkButton();
    buttonLoopTime = currentTime;
  }
  if(currentTime >= (sleepTime + 40000)){
    initRadio();  // to correct some hanging problem
    heaterOn(); //to detect the problem
    sleepTime = currentTime;
    Blink(LED, 300);
  }
  delay(0.1);
}

void initRadio(){
  radio.initialize(FREQUENCY, NODEID, NETWORKID);
  radio.encrypt(ENCRYPTKEY);
  radio.promiscuous(promiscuousMode);
#ifdef IS_RFM69HW
  radio.setHighPower(); //uncomment only for RFM69HW!
#endif
  radio.sleep();
}

void checkButton(void) {
  encoder_A = digitalRead(pin_A);    // Read encoder pins
  encoder_B = digitalRead(pin_B);
  if ((!encoder_A) && (encoder_A_prev)) {
    // A has gone from high to low
    if (encoder_B) {
      // B is high so clockwise
      // increase the brightness, dont go over 255
      cible += 0.5;
      updateScreen();
      
    }
    else {
      // B is low so counter-clockwise
      // decrease the brightness, dont go below 0
       cible -= 0.5;
       updateScreen();
    }

  }
  encoder_A_prev = encoder_A;     // Store value of A for next time

//READ SWITCH BUTTON
  buttonreading = digitalRead(buttonpin);
  if (buttonreading == HIGH && buttonprevious == LOW && millis() - buttontime > buttondebounce) {
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
  sprintf(ligne3, "Temp: %s", (rooms_temps[zone]==0)?"Inconnue":buffTemp);
  sprintf(ligne4, "Status: %s", currentStatus ? "ON" : "OFF");
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

void rotateZone() {
  zone++;
  int maxtry=7;
  if (zone>6) zone =1;
  while ((rooms_temps[zone] == 0) && (maxtry >0)) {
    zone++;
    maxtry--;
    if (zone>6) zone =1;
  }
  DEBUGln(zone);
  updateScreen();
}

void storeOpen() {
  DEBUGln("STORE OPENING");
  /* Send RF frame */
  RF_BUFFER[4] = (RF_BUFFER[4] & 0x0F) | 0x10;
  generate_rolling_code(RF_BUFFER);
  generate_token(RF_BUFFER);
  Blink(LED, 100);
  send_command(RF_BUFFER);
}

void storeClose() {
  DEBUGln("STORE CLOSING");
  /* Send RF frame */
  RF_BUFFER[4] = (RF_BUFFER[4] & 0x0F) | 0x00;
  generate_rolling_code(RF_BUFFER);
  generate_token(RF_BUFFER);
  Blink(LED, 100);
  send_command(RF_BUFFER);
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
  updateScreen();
}

void setZone(String str_cible) {
  zone = str_cible.toInt();
  DEBUG("Zone: ");
  DEBUGln(zone);
  updateScreen();
}


/* BLYSS METHODS */
/**
 * Send header over RF
 */
inline void send_header(void) {
  SIG_HIGH();
  delayMicroseconds(H_TIME);
}

/**
 * Send footer over RF
 */
inline void send_footer(void) {
  SIG_LOW();
  delay(H_TIME * 10 / 1000);
}

/**
 * Send logical "1" over RF
 */
inline void send_one(void) {
  SIG_LOW();
  delayMicroseconds(T_TIME);
  SIG_HIGH();
  delayMicroseconds(T_TIME * 2);
}

/**
 * Send logical "0" over RF
 */
inline void send_zero(void) {
  SIG_LOW();
  delayMicroseconds(T_TIME * 2);
  SIG_HIGH();
  delayMicroseconds(T_TIME);
}

/**
 * Send a bits quarter (4 bits = MSB from 8 bits value) over RF
 *
 * @param data Source data to process and sent
 */
inline void send_quarter_MSB(byte data) {
  (bitRead(data, 7)) ? send_one() : send_zero();
  (bitRead(data, 6)) ? send_one() : send_zero();
  (bitRead(data, 5)) ? send_one() : send_zero();
  (bitRead(data, 4)) ? send_one() : send_zero();
}

/**
 * Send a bits quarter (4 bits = LSB from 8 bits value) over RF
 *
 * @param data Source data to process and sent
 */
inline void send_quarter_LSB(byte data) {
  (bitRead(data, 3)) ? send_one() : send_zero();
  (bitRead(data, 2)) ? send_one() : send_zero();
  (bitRead(data, 1)) ? send_one() : send_zero();
  (bitRead(data, 0)) ? send_one() : send_zero();
}

/**
 * Generate next valid token for RF transmission
 *
 * @param data Pointer to a RF frame-data buffer
 */
void generate_token(byte *data) {
  static byte last_token = 0x7D;
  data[5] = (data[5] & 0xF0) | ((last_token & 0xF0) >> 4);
  data[6] = (last_token & 0x0F) << 4;
  last_token += 10;
}

/**
 * Generate next valid rolling code for RF transmission
 *
 * @param data Pointer to a RF frame-data buffer
 */
void generate_rolling_code(byte *data) {
  static byte i = 0;
  data[4] = (data[4] & 0xF0) | ((RF_ROLLING_CODE[i] & 0xF0) >> 4);
  data[5] = (data[5] & 0x0F) | (RF_ROLLING_CODE[i] & 0x0F) << 4;
  if (++i >= sizeof(RF_ROLLING_CODE)) i = 0;
}

/**
 * Change the status (ON / OFF) of the transmitter
 *
 * @param data Pointer to a RF frame-data buffer
 * @param status Status to use (ON or OFF)
 */
inline void set_status(byte *data, byte status) {
  if (!status) data[4] = (data[4] & 0x0F) | 0x10;
  else data[4] &= 0x0F;
}

/**
 * Send a complete frame-data buffer over RF
 *
 * @param data Pointer to a RF frame-data buffer
 */
void send_buffer(byte *data) {
  send_header();
  for (byte i = 0; i < 6; ++i) {
    send_quarter_MSB(data[i]);
    send_quarter_LSB(data[i]);
  }
  send_quarter_MSB(data[6]);
  send_footer();
}

/**
 * Send a complete frame-data buffer n times to be hooked by the target receiver
 *
 * @param data Pointer to a RF frame-data buffer
 */
inline void send_command(byte *data) {
  for (byte i = 0; i < nb_frames; ++i)
    send_buffer(data);
}

/**
 * Copy a RF key ID into a frame-data buffer
 *
 * @param data Pointer to a RF frame-data buffer
 * @param key Pointer to a RF key-data buffer
 * @param overwrite Set to true if you want to overwrite channel data and use data from key buffer
 */
inline void set_key(byte *data, byte *key, byte overwrite) {
  data[0] = 0xFE;
  if (overwrite)
    data[1] = key[0];
  else
    data[1] = (data[1] & 0xF0) | (key[0] & 0x0F);
  data[2] = key[1];
  if (overwrite)
    data[3] = key[2];
  else
    data[3] = (data[3] & 0x0F) | (key[2] & 0xF0);
}

/**
 * Set the target sub-channel of the transmitter
 *
 * @param data Pointer to a RF frame-data buffer
 * @param channel Target channel
 */
inline void set_channel(byte *data, byte channel) {
  data[3] = (data[3] & 0xF0) | (channel & 0x0F);
}

/**
 * Set the target global channel of the transmitter
 *
 * @param data Pointer to a RF frame-data buffer
 * @param channel Target channel
 */
inline void set_global_channel(byte *data, byte channel) {
  data[1] = (data[1] & 0x0F) | ((channel << 4) & 0xF0);
}
