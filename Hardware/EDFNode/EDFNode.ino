#include <RFM69.h>    //get it here: https://www.github.com/lowpowerlab/rfm69
#include <SPI.h>
#include <ECOCommons.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#define NODEID        NODE_EDF    //unique for each node on same network
#define NETWORKID     100  //the same on all nodes that talk to each other
#define GATEWAYID     1
#define FREQUENCY     RF69_868MHZ
#define IS_RFM69HW    //uncomment only for RFM69HW! Leave out if you have RFM69W!
#define SERIAL_BAUD   1200
#define LED           9 // Leds PIN
#define ONE_WIRE_BUS 3


RFM69 radio;
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
bool promiscuousMode = false; //set to 'true' to sniff all packets on the same network

/***************** Teleinfo configuration part *******************/
char CaractereRecu = '\0';
char Checksum[32] = "";
char Ligne[32] = "";
char Etiquette[9] = "";
char Donnee[13] = "";
char Trame[512] = "";
int count = 0;

int check[5];  // Checksum by etiquette
int trame_ok = 1; // global trame checksum flag
int finTrame = 0;
char hc_value[10] = "", hp_value[10] = "";
char power_value[10] = "";
char tarif_value[3] = "";
unsigned long lastMillisTemp = millis();
unsigned long lastMillisEDF = millis();
unsigned long lastMinute = 4;
String currentAction = "";
char buff[50];


int tour = 0;
void setup() {
  Serial.begin(SERIAL_BAUD);
  delay(10);
  radio.initialize(FREQUENCY, NODEID, NETWORKID);
#ifdef IS_RFM69HW
  radio.setHighPower(); //only for RFM69HW!
#endif
  radio.encrypt(ENCRYPTKEY);
  sensors.begin();
  Blink(LED, 1000);
}

void loop() {

    if (Serial.available()>0){
      CaractereRecu = Serial.read();
      getTeleinfo();
    }
    checkLocalTemp();

}

void checkLocalTemp() {  // Verifie si c'est le moment de mesurer la temperature localement
  unsigned long delta = millis() - lastMillisTemp;
  if (delta > 60000) { // Toutes les minutes
    lastMinute = lastMinute + 1 ;
    lastMillisTemp = millis();
    if (lastMinute >= 10) { // Toutes les 10go minutes
      sendLocalTemp();
      lastMinute = 0;
    }
  }
}

void sendLocalTemp() {  // Mesure et envoie la temperature locale
  sensors.requestTemperatures();
  float temp = sensors.getTempCByIndex(0) / 1;
  char str_temp[6];
  dtostrf(temp, 0, 2, str_temp);
  sprintf(buff, "T|%s||", str_temp );
  byte sendSize = strlen(buff);
  radio.sendWithRetry(GATEWAYID, buff, sendSize, RFM69Retry, RFM69RetryTimeout);
}



void Blink(byte PIN, int DELAY_MS)
{
  pinMode(PIN, OUTPUT);
  digitalWrite(PIN, HIGH);
  delay(DELAY_MS);
  digitalWrite(PIN, LOW);
}

/*------------------------------------------------------------------------------*/
/* Test checksum d'un message (Return 1 si checkum ok)				*/
/*------------------------------------------------------------------------------*/
int checksum_ok(char *etiquette, char *valeur, char checksum)
{
  unsigned char sum = 32 ;		// Somme des codes ASCII du message + un espace
  int i ;

  for (i = 0; i < strlen(etiquette); i++) sum = sum + etiquette[i] ;
  for (i = 0; i < strlen(valeur); i++) sum = sum + valeur[i] ;
  sum = (sum & 63) + 32 ;
  if ( sum == checksum) return 1 ;	// Return 1 si checkum ok.
  return 0 ;
}

/***********************************************
 * getTeleinfo
 * Decode Teleinfo from serial
 * Input : n/a
 * Output : n/a
 ***********************************************/
void getTeleinfo() {
  int i;
  /* vider les infos de la dernière trame lue */
  memset(Ligne, '\0', 32);
  memset(Trame, '\0', 512);
  int trameComplete = 0;
  while (!trameComplete) {
    while (CaractereRecu != 0x02) // boucle jusqu'a "Start Text 002" début de la trame
    {
      if (Serial.available()) {
        CaractereRecu = Serial.read() & 0x7F;
      }
    }

    i = 0;
    while (CaractereRecu != 0x03) // || !trame_ok ) // Tant qu'on est pas arrivé à "EndText 003" Fin de trame ou que la trame est incomplète
    {
      if (Serial.available()) {
        CaractereRecu = Serial.read() & 0x7F;
        Trame[i++] = CaractereRecu;
      }
    }
    finTrame = i;
    Trame[i++] = '\0';

    lireTrame(Trame);

    // on vérifie si on a une trame complète ou non
    for (i = 0; i < 5; i++) {
      trameComplete += check[i];
    }

    for (i = 0; i < 5; i++) {
      check[i] = 0; // on remet à 0 les check.
    }

    if (trameComplete < 5) trameComplete = 0; // on a pas les 5 valeurs, il faut lire la trame suivante
    else {
      trameComplete = 1;
      unsigned long delta = millis() - lastMillisEDF;
      if (delta > 10000) { // Toutes les 10 s
        lastMillisEDF = millis();
        sprintf(buff, "P|%s|%s|%s|%s", power_value , tarif_value,hc_value, hp_value  );
        byte sendSize = strlen(buff);
        radio.sendWithRetry(GATEWAYID, buff, sendSize, RFM69Retry, RFM69RetryTimeout);
        Blink(LED, 30);
      }
    }
  }
}

void lireTrame(char *trame) {
  int i;
  int j = 0;
  for (i = 0; i < strlen(trame); i++) {
    if (trame[i] != 0x0D) { // Tant qu'on est pas au CR, c'est qu'on est sur une ligne du groupe
      Ligne[j++] = trame[i];
    }
    else { //On vient de finir de lire une ligne, on la décode (récupération de l'etiquette + valeur + controle checksum
      decodeLigne(Ligne);
      memset(Ligne, '\0', 32); // on vide la ligne pour la lecture suivante
      j = 0;
    }

  }
}

int decodeLigne(char *ligne) {
  int debutValeur;
  int debutChecksum;
  // Décomposer en fonction pour lire l'étiquette etc ...
  debutValeur = lireEtiquette(ligne);
  debutChecksum = lireValeur(ligne, debutValeur);
  lireChecksum(ligne, debutValeur + debutChecksum - 1);

  if (checksum_ok(Etiquette, Donnee, Checksum[0])) { // si la ligne est correcte (checksum ok) on affecte la valeur à l'étiquette
    return affecteEtiquette(Etiquette, Donnee);
  }
  else return 0;

}

int lireEtiquette(char *ligne) {
  int i;
  int j = 0;
  memset(Etiquette, '\0', 9);
  for (i = 1; i < strlen(ligne); i++) {
    if (ligne[i] != 0x20) { // Tant qu'on est pas au SP, c'est qu'on est sur l'étiquette
      Etiquette[j++] = ligne[i];
    }
    else { //On vient de finir de lire une etiquette
      //  Serial.print("Etiquette : ");
      //  Serial.println(Etiquette);
      return j + 2; // on est sur le dernier caractère de l'etiquette, il faut passer l'espace aussi (donc +2) pour arriver à la valeur
    }

  }
}

int lireValeur(char *ligne, int offset) {
  int i;
  int j = 0;
  memset(Donnee, '\0', 13);
  for (i = offset; i < strlen(ligne); i++) {
    if (ligne[i] != 0x20) { // Tant qu'on est pas au SP, c'est qu'on est sur l'étiquette
      Donnee[j++] = ligne[i];
    }
    else { //On vient de finir de lire une etiquette
      //  Serial.print("Valeur : ");
      //  Serial.println(Donnee);
      return j + 2; // on est sur le dernier caractère de la valeur, il faut passer l'espace aussi (donc +2) pour arriver à la valeur
    }

  }
}

void lireChecksum(char *ligne, int offset) {
  int i;
  int j = 0;
  memset(Checksum, '\0', 32);
  for (i = offset; i < strlen(ligne); i++) {
    Checksum[j++] = ligne[i];
    //  Serial.print("Chekcsum : ");
    //  Serial.println(Checksum);
  }

}

int affecteEtiquette(char *etiquette, char *valeur) {

  if (strcmp(etiquette, "HCHC") == 0) {
    sprintf(hc_value, "%s", valeur);
    check[0] = 1;
  }
  else if (strcmp(etiquette, "HCHP") == 0) {
    sprintf(hp_value, "%s", valeur);
    check[1] = 1;
  }
  else if (strcmp(Etiquette, "IINST") == 0) {
    check[2] = 1;
  }
  else if (strcmp(Etiquette, "PAPP") == 0) {
    sprintf(power_value, "%s", valeur);
    check[3] = 1;
  }
  else if (strcmp(Etiquette, "PTEC") == 0) {
    sprintf(tarif_value, "%s", valeur);
    tarif_value[2] = '\0';
    check[4] = 1;
  }
  else
    return 0;
  return 1;
}



