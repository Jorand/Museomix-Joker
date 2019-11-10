#include <Wire.h>
#include "Adafruit_TCS34725.h"
#include "SoftwareSerial.h"
#include "DFRobotDFPlayerMini.h"


SoftwareSerial mySoftwareSerial(10, 11); // RX, TX
DFRobotDFPlayerMini myDFPlayer;

const int TCS_LED = A0;
const int CARD_BTN = 2;
const int REMOTE_BTN = 4;
const int REMOTE_LED = 7;

const int sample = 973;

int buttonCardState = 0;
int buttonRemoteState = 0;

static unsigned long timer = millis();
static unsigned long timerReadDFPlayer = millis();
bool lastState = true;
bool playState = false;
bool play2State = false;

/* Example code for the Adafruit TCS34725 breakout library */

/* Connect SCL    to analog 5
   Connect SDA    to analog 4
   Connect VDD    to 3.3V DC
   Connect GROUND to common ground */

/* Initialise with default values (int time = 2.4ms, gain = 1x) */
// Adafruit_TCS34725 tcs = Adafruit_TCS34725();

/* Initialise with specific int time and gain values */
Adafruit_TCS34725 tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_700MS, TCS34725_GAIN_1X);

void setup(void) {
  mySoftwareSerial.begin(9600);
  Serial.begin(115200);

  pinMode(TCS_LED, OUTPUT);
  pinMode(REMOTE_LED, OUTPUT);
  pinMode(CARD_BTN, INPUT_PULLUP);
  pinMode(REMOTE_BTN, INPUT_PULLUP);


  if (tcs.begin()) {
    Serial.println("Found sensor");
  } else {
    Serial.println("No TCS34725 found ... check your connections");
    while (1);
  }

  if (!myDFPlayer.begin(mySoftwareSerial)) {  //Use softwareSerial to communicate with mp3.
    Serial.println(F("Unable to begin:"));
    Serial.println(F("1.Please recheck the connection!"));
    Serial.println(F("2.Please insert the SD card!"));
    while(true);
  }
  Serial.println(F("DFPlayer Mini online."));

  myDFPlayer.volume(30);  //Set volume value. From 0 to 30
  //myDFPlayer.play(1);  //Play the first mp3

  digitalWrite(REMOTE_LED, HIGH);
  delay(200);
  digitalWrite(REMOTE_LED, LOW);
  delay(200);
  digitalWrite(REMOTE_LED, HIGH);
  delay(500);
  digitalWrite(REMOTE_LED, LOW);
}

void loop(void) {

  //Serial.println(myDFPlayer.readState());

  if (millis() - timerReadDFPlayer > 500) {
    timerReadDFPlayer = millis();
    //Serial.println(myDFPlayer.readState());
    if (myDFPlayer.readState() != 513) {
      playState = false;
      digitalWrite(REMOTE_LED, LOW);
    }
  }

  if (!play2State) {
    buttonRemoteState = digitalRead(REMOTE_BTN);

    if (buttonRemoteState != lastState && buttonRemoteState == LOW && millis() - timer > 500) {
      timer = millis();
  
      if (!playState) {
        playState = true;
        digitalWrite(REMOTE_LED, HIGH);
        myDFPlayer.playMp3Folder(1);
      }
      lastState = buttonRemoteState;
    }
  }

  // Card in
  buttonCardState = digitalRead(CARD_BTN);
  if (buttonCardState == LOW) {
    // turn LED on:
    digitalWrite(TCS_LED, HIGH);
    delay(200);
    readSensor();
  } else {
    // turn LED off:
    digitalWrite(TCS_LED, LOW);
    play2State = false;
    if (!playState) {
      myDFPlayer.pause();
    }
  }

  if (!playState && !play2State) {
    //myDFPlayer.pause();
  }

}

void readSensor() {
  uint16_t r, g, b, c, colorTemp, lux;

  tcs.getRawData(&r, &g, &b, &c);
  // colorTemp = tcs.calculateColorTemperature(r, g, b);
  colorTemp = tcs.calculateColorTemperature_dn40(r, g, b, c);
  lux = tcs.calculateLux(r, g, b);
  /*
  Serial.print("Color Temp: "); Serial.print(colorTemp, DEC); Serial.print(" K - ");
  Serial.print("Lux: "); Serial.print(lux, DEC); Serial.print(" - ");
  Serial.print("R: "); Serial.print(r, DEC); Serial.print(" ");
  Serial.print("G: "); Serial.print(g, DEC); Serial.print(" ");
  Serial.print("B: "); Serial.print(b, DEC); Serial.print(" ");
  Serial.print("C: "); Serial.print(c, DEC); Serial.print(" ");
  Serial.println(" ");
  */  

  int colorDistance = abs(r - sample);

  if (colorDistance < 0) {
    colorDistance = -colorDistance;
  }

  //Serial.println(colorDistance);

  // Check color
  if (colorDistance < 50) {
    // GOOD
    // Play story
    if (!play2State) {
      myDFPlayer.playMp3Folder(2);
      play2State = true;
    }
  }
  else {
    if (!play2State) {
      myDFPlayer.playMp3Folder(3);
      play2State = true;
    }
  }
}
