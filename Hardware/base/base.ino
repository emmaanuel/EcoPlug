#include <RFM69.h>    //get it here: https://www.github.com/lowpowerlab/rfm69
#include <SPI.h>
#include <SoftwareSerial.h>
#include <ECOCommons.h>

#define NODEID        1    //unique for each node on same network
#define NETWORKID     100  //the same on all nodes that talk to each other
#define FREQUENCY     RF69_868MHZ
#define IS_RFM69HW    //uncomment only for RFM69HW! Leave out if you have RFM69W!
#define SERIAL_BAUD   115200
#define LED           9 // Leds PIN

RFM69 radio;
bool promiscuousMode = false; //set to 'true' to sniff all packets on the same network
SoftwareSerial cptSerial(4, 3);
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
unsigned long hc_value = 0, hp_value = 0;
unsigned int power_value = 0;
char tarif_value[3] = "";


void setup() {
  Serial.begin(SERIAL_BAUD);
  delay(10);
  radio.initialize(FREQUENCY, NODEID, NETWORKID);
#ifdef IS_RFM69HW
  radio.setHighPower(); //only for RFM69HW!
#endif
  radio.encrypt(ENCRYPTKEY);
  radio.promiscuous(promiscuousMode);
  cptSerial.begin(1200);
}

void loop() {
  if (radio.receiveDone())
  {
    Serial.print("N:");
    Serial.print(radio.SENDERID, DEC);
    Serial.print("|");
    for (byte i = 0; i < radio.DATALEN; i++)
      Serial.print((char)radio.DATA[i]);
    Serial.print("|RX:"); Serial.println(radio.RSSI);

    if (radio.ACKRequested())
    {
      byte theNodeID = radio.SENDERID;
      radio.sendACK();
    }
    Blink(LED, 3);
  }

  getTeleinfo();
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
      if (cptSerial.available()) {
        CaractereRecu = cptSerial.read() & 0x7F;
      }
    }

    i = 0;
    while (CaractereRecu != 0x03) // || !trame_ok ) // Tant qu'on est pas arrivé à "EndText 003" Fin de trame ou que la trame est incomplète
    {
      if (cptSerial.available()) {
        CaractereRecu = cptSerial.read() & 0x7F;
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
      Serial.print("N:1");
      Serial.print("|PW|");
      Serial.print("hc:");
      Serial.print(hc_value);
      Serial.print("|hp:");
      Serial.print(hp_value);
      Serial.print("|pw:");
      Serial.print(power_value);
      Serial.print("|tf:");
      Serial.println(tarif_value);
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
    hc_value = atol(valeur);
    check[0] = 1;
  }
  else if (strcmp(etiquette, "HCHP") == 0) {
    hp_value = atol(valeur);
    check[1] = 1;
  }
  else if (strcmp(Etiquette, "IINST") == 0) {
    check[2] = 1;
  }
  else if (strcmp(Etiquette, "PAPP") == 0) {
    power_value = atol(valeur);
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

