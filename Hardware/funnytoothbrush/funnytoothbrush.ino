#include <SPIFlash.h>

#include <LowPower.h>
#include <RFM69.h>    //get it here: https://www.github.com/lowpowerlab/rfm69
#include <SPI.h>
#include <TOMCommons.h>
#define LED 9
#define SEUIL 400
#define SLEEP_TIMEOUT 50000
#define SEND_TIMEOUT 500
#define NODEID        NODE_BAD1    //unique for each node on same network
#define GATEWAYID     NODE_BASE
#define FREQUENCY   RF69_868MHZ
#define ACCVCC 4

const int ap1 = A3;
const int ap2 = A2;
const int ap3 = A1;

int sv1 = 0;
int sv2 = 0;
int sv3 = 0;
unsigned long sleeptimer = millis();
unsigned long sendtimer = 0;
unsigned int count = 0;
RFM69 radio;
SPIFlash flash(5); 

void setup() {
  radio.initialize(FREQUENCY, NODEID, NETWORKID);
  radio.encrypt(ENCRYPTKEY);
  radio.sleep();
  pinMode(LED, OUTPUT);
  pinMode(ACCVCC, OUTPUT);
  digitalWrite(ACCVCC, HIGH);
  Blink(LED, 100);
  //Serial.begin(9600);
  if (flash.initialize())     
{    
//    DEBUGln("Flash Init OK!");    
    flash.sleep(); 
} 
}

void loop() {
  sv1 = analogRead(ap1);
  delay(2);
  if (sv1 < SEUIL || sv1 > (1024 - SEUIL)) {
    //Blink(LED, 100);
    sleeptimer = millis();
    count = count + 1;
  }
  if (millis() > (sleeptimer + SLEEP_TIMEOUT)) {
    Blink(LED, 1000);
    goDodo();
  }

  if (millis() > (sendtimer + SEND_TIMEOUT)) {
    sendScore(count);
    count=0;
    sendtimer = millis();
  }

}

void goDodo(){
    digitalWrite(ACCVCC, LOW);
    radio.sleep();
    flash.sleep();
    LowPower.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_OFF);
}

void Blink(byte PIN, int DELAY_MS)
{
  pinMode(PIN, OUTPUT);
  digitalWrite(PIN, HIGH);
  delay(DELAY_MS);
  digitalWrite(PIN, LOW);
}

void sendScore(int score)
{
  char str_score[6];
  dtostrf(score, 0, 0, str_score);
  char buff[50];
  sprintf(buff, "BAD|%s", str_score);
  byte sendSize = strlen(buff);
  if (radio.sendWithRetry(GATEWAYID, buff, sendSize, RFM69Retry, RFM69RetryTimeout)) {
    //Blink(LED, 30);
  }
}

