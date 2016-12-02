#include <RFM69.h>    //get it here: https://www.github.com/lowpowerlab/rfm69
#include <SPI.h>
#include <LowPower.h> //get library from: https://github.com/lowpowerlab/lowpower
#include <TH02.h>
#include <Wire.h>
#include <ECOCommons.h>

#define NODEID        NODE_RDC    //unique for each node on same network
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

  int sensorValue = analogRead(A0);
  // print out the value you read:
  float volt = (float) sensorValue / 1024 * readVcc() *2 / 1000;
  
  char str_temp[6];
  char str_rh[6];
  char str_volt[6];
  dtostrf(temp, 0, 2, str_temp);
  dtostrf(rh, 0, 2, str_rh);
  dtostrf(volt, 0, 2, str_volt);
  

  sprintf(buff, "T|%s|%s||%s", str_temp , str_rh, str_volt);
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

long readVcc() {
  // Read 1.1V reference against AVcc
  // set the reference to Vcc and the measurement to the internal 1.1V reference
  #if defined(__AVR_ATmega32U4__) || defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
    ADMUX = _BV(REFS0) | _BV(MUX4) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  #elif defined (__AVR_ATtiny24__) || defined(__AVR_ATtiny44__) || defined(__AVR_ATtiny84__)
    ADMUX = _BV(MUX5) | _BV(MUX0);
  #elif defined (__AVR_ATtiny25__) || defined(__AVR_ATtiny45__) || defined(__AVR_ATtiny85__)
    ADMUX = _BV(MUX3) | _BV(MUX2);
  #else
    ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  #endif  

  delay(2); // Wait for Vref to settle
  ADCSRA |= _BV(ADSC); // Start conversion
  while (bit_is_set(ADCSRA,ADSC)); // measuring

  uint8_t low  = ADCL; // must read ADCL first - it then locks ADCH  
  uint8_t high = ADCH; // unlocks both

  long result = (high<<8) | low;

  result = 1125300L / result; // Calculate Vcc (in mV); 1125300 = 1.1*1023*1000
  return result; // Vcc in millivolts
}

void Blink(byte PIN, int DELAY_MS)
{
  pinMode(PIN, OUTPUT);
  digitalWrite(PIN, HIGH);
  delay(DELAY_MS);
  digitalWrite(PIN, LOW);
}
