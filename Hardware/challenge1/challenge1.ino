#include "U8glib.h"


#define LED           9 // Moteinos have LEDs on D9


char ligne1[20] = "";
char ligne2[20] = "";
char ligne3[20] = "";
char ligne4[20] = "";

const int pin_A = 3;
const int pin_B = 4;
int n = 0;
unsigned char encoder_A;
unsigned char encoder_B;
unsigned char encoder_A_prev = 0;
unsigned long currentTime;
unsigned long buttonLoopTime;
unsigned long sleepTime;
unsigned long refreshScreenTime;
unsigned long nbMessage = 0;

const int buttonpin = 5;
int buttonreading;           // the current reading from the input pin
int buttonprevious = LOW;    // the previous reading from the input pin
long buttontime = 0;         // the last time the output pin was toggled
long buttondebounce = 200;   // the debounce time, increase if the output flickers

U8GLIB_SSD1306_128X64 u8g(U8G_I2C_OPT_NO_ACK);  // Display which does not send ACK

uint8_t inputcode[2][4] = {{0, 0, 0, 0}, {0, 0, 0, 0}};
uint8_t code[2][4] = {{77, 4, 36, 89}, {5, 28, 88, 50}};

uint8_t level = 0;
int n_inputcode[2] = {0, 0};

void setup() {
  delay(100);
  pinMode(LED, OUTPUT);
#ifdef SERIAL_EN
  Serial.begin(SERIAL_BAUD);
#endif
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
  sleepTime = currentTime;
  refreshScreenTime = currentTime;
  Blink(LED, 500);
}

void loop() {
  currentTime = millis();
  checkButton();
  delay(0.3);
}

// Verification des actions sur le boutton
void checkButton(void) {
  encoder_A = digitalRead(pin_A);    // Read encoder pins
  encoder_B = digitalRead(pin_B);
  if ((!encoder_A) && (encoder_A_prev)) {
    // A has gone from high to low
    if (encoder_B) {
      // B is high so clockwise
      inputcode[level][n_inputcode[level]] += 1;
      if (inputcode[level][n_inputcode[level]] > 99) inputcode[level][n_inputcode[level]] = 0;
      updateScreen();
    }
    else {
      // B is low so counter-clockwise
      inputcode[level][n_inputcode[level]] -= 1;
      if (inputcode[level][n_inputcode[level]] > 99) inputcode[level][n_inputcode[level]] = 99;
      updateScreen();
    }
  }
  encoder_A_prev = encoder_A;     // Store value of A for next time

  //Lecture du boutton push
  buttonreading = digitalRead(buttonpin);
  if (buttonreading == HIGH && buttonprevious == LOW && millis() - buttontime > buttondebounce) {
    n_inputcode[level] += 1;
    if (n_inputcode[level] == 4) {
      n_inputcode[level] = 0;
      checkresult();
      updateScreen();
    }
    buttontime = millis();
  }
  buttonprevious = buttonreading;
}

// Dessine l'écran
void draw(void) {
  u8g.setFont(u8g_font_unifont);
  u8g.drawStr( 0, 15, ligne1);
  u8g.drawStr( 0, 30, ligne2);
  u8g.drawStr( 0, 45, ligne3);
  u8g.drawStr( 0, 60, ligne4);
}


// Actualise le texte à afficher
void updateScreen() {
  if (level == 0) {
    sprintf(ligne1, "     code1 ");
    sprintf(ligne2, "  %02d %02d %02d %02d", inputcode[0][0], inputcode[0][1], inputcode[0][2], inputcode[0][3]);
    sprintf(ligne3, "");
    sprintf(ligne4, "");
  } else if (level == 1) {
    sprintf(ligne1, "     code1");
    sprintf(ligne2, "  %02d %02d %02d %02d OK", inputcode[0][0], inputcode[0][1], inputcode[0][2], inputcode[0][3]);
    sprintf(ligne3, "     code2");
    sprintf(ligne4, "  %02d %02d %02d %02d", inputcode[1][0], inputcode[1][1], inputcode[1][2], inputcode[1][3]);
  } else if (level > 1) {
    sprintf(ligne1, "Bravo champion");
    sprintf(ligne2, "//GEB/public/");
    sprintf(ligne3, "/emmanuel_cor");
    sprintf(ligne4, "DGRD-SQE54!");
  }
  u8g.firstPage();
  do {
    draw();
  } while ( u8g.nextPage() );

}

// Verifie si le resultat est correct
void checkresult() {
  boolean r = true;
  if (level == 0) {
    for (int i = 0; i < 4; i++) {
      if (inputcode[level][i] != code[level][i])
        r = false;
    }
  } else if (level == 1) {
    for (int i = 0; i < 4; i++) {
      if (encrypt(inputcode[level][i], level, i) != code[level][i])
        r = false;
    }
  }
  if (level<2 && r)
    level += 1;
  else
    Blink(LED, 100);
}

uint8_t encrypt (uint8_t v, uint8_t n, uint8_t m) {
  for (int x = 0; x < m; x++) {
    v += ((v << 4) + n) ^ (v) ^ ((v >> 5) + n);
  }
  return v;
}

void Blink(byte PIN, int DELAY_MS)
{
  digitalWrite(PIN, HIGH);
  delay(DELAY_MS);
  digitalWrite(PIN, LOW);
}
