#include <RFM69.h>    //get it here: https://www.github.com/lowpowerlab/rfm69
#include <SPI.h>
#include <LowPower.h> //get library from: https://github.com/lowpowerlab/lowpower
#include <TH02.h>
#include <Wire.h>
#include <ECOCommons.h>

#define NODEID        NODE_RDC    //unique for each node on same network
#define NETWORKID     100  //the same on all nodes that talk to each other
#define GATEWAYID     NODE_BASE
#define FREQUENCY   RF69_868MHZ
//#define IS_RFM69HW    //uncomment only for RFM69HW! Leave out if you have RFM69W!
#define LED           9 // Moteinos have LEDs on D9
#define SERIAL_BAUD   115200
#define TH02VCC A2

//#define SERIAL_EN                //comment out if you don't want any serial output

#ifdef SERIAL_EN
#define DEBUG(input)   {Serial.print(input); delay(1);}
#define DEBUGln(input) {Serial.println(input); delay(1);}
#else
#define DEBUG(input);
#define DEBUGln(input);
#endif



RFM69 radio;
TH02 sensor(TH02_I2C_ADDR);
char buff[50];

void setup() {
  pinMode(TH02VCC, OUTPUT);
  digitalWrite(TH02VCC, HIGH);
#ifdef SERIAL_EN
  Serial.begin(SERIAL_BAUD);
#endif
  radio.initialize(FREQUENCY, NODEID, NETWORKID);
#ifdef IS_RFM69HW
  radio.setHighPower(); //uncomment only for RFM69HW!
#endif
  radio.encrypt(ENCRYPTKEY);
  radio.sleep();
  sprintf(buff, "\nTransmitting at %d Mhz...", FREQUENCY == RF69_433MHZ ? 433 : FREQUENCY == RF69_868MHZ ? 868 : 915);
  DEBUGln(buff);

  uint8_t devID;
  Wire.begin();
  sensor.getId(&devID);
  DEBUG("TH02 device ID = 0x");
  DEBUGln(devID);

}
void loop() {
  float temp = 0, rh = 0;
  DEBUG("Starting Temperature conversion.");
  digitalWrite(TH02VCC, HIGH);
  delay(15);
  sensor.startTempConv();
  sensor.waitEndConversion();
  DEBUGln(".done!");

  // Get temperature calculated and rounded
  sensor.getConversionValue();
  temp = sensor.getLastRawTemp() / 100.0;
  DEBUG("Temperature = ");
  DEBUGln(sensor.getLastRawTemp() / 100.0);

  DEBUG("Starting Humidity conversion.");
  // Convert humidity
  sensor.startRHConv();
  sensor.waitEndConversion();
  DEBUGln(".done!");
  sensor.getConversionValue();
  rh = sensor.getConpensatedRH(false) / 100.0;
  DEBUG("Raw Humidity = ");
  DEBUG(rh);
  DEBUGln("%");
  digitalWrite(TH02VCC, LOW);
  char str_temp[6];
  char str_rh[6];
  dtostrf(temp, 0, 2, str_temp);
  dtostrf(rh, 0, 2, str_rh);

  sprintf(buff, "T|%s|%s|", str_temp , str_rh);
  byte sendSize = strlen(buff);
  DEBUG("Sending[");
  DEBUG(sendSize);
  DEBUG("]: ");
  DEBUG(buff);
  if (radio.sendWithRetry(GATEWAYID, buff, sendSize, RFM69Retry, RFM69RetryTimeout)) {
    DEBUGln(" ok!");
  } else {
    DEBUGln(" nothing...");
  }
  radio.sleep();
  Blink(LED, 3);

  // On attend 10 min
  for (byte i = 0; i < 70; i++)
    LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
}

void Blink(byte PIN, int DELAY_MS)
{
  pinMode(PIN, OUTPUT);
  digitalWrite(PIN, HIGH);
  delay(DELAY_MS);
  digitalWrite(PIN, LOW);
}
