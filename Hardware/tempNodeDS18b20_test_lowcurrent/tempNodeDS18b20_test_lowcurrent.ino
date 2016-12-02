#include <RFM69.h>    //get it here: https://www.github.com/lowpowerlab/rfm69
#include <SPI.h>
#include <LowPower.h> //get library from: https://github.com/lowpowerlab/lowpower
#include <OneWire.h>
#include <DallasTemperature.h>
#include <ECOCommons.h>

#define NODEID        NODE_JARDIN    //unique for each node on same network
#define GATEWAYID     NODE_BASE
#define FREQUENCY   RF69_868MHZ
//#define IS_RFM69HW    //uncomment only for RFM69HW! Leave out if you have RFM69W!
#define LED           9 // Moteinos have LEDs on D9
#define SERIAL_BAUD   115200

//#define SERIAL_EN                //comment out if you don't want any serial output

#ifdef SERIAL_EN
#define DEBUG(input)   {Serial.print(input); delay(1);}
#define DEBUGln(input) {Serial.println(input); delay(1);}
#else
#define DEBUG(input);
#define DEBUGln(input);
#endif

RFM69 radio;
char buff[50];



void setup() {
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
  // Start up the library
  
}
void loop() {

  float temp = 0, rh = 0;
  //uint8_t status;

  DEBUG("go sleep...");

  radio.sleep();
  Blink(LED, 3000);

  // On attend 10 min
  for (byte i = 0; i < 2; i++)
    LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
}

void Blink(byte PIN, int DELAY_MS)
{
  pinMode(PIN, OUTPUT);
  digitalWrite(PIN, HIGH);
  delay(DELAY_MS);
  digitalWrite(PIN, LOW);
}
