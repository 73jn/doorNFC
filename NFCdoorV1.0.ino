#include <MFRC522.h>
#include <SPI.h>
#include <EEPROM.h>
#include <DS3231.h>
#include <Wire.h>

DS3231 Clock;
bool Century = false;
bool h12;
bool PM;
byte ADay, AHour, AMinute, ASecond, ABits;
bool ADy, A12h, Apm;

int state = 0;
byte COD[10];
byte AUX[10];

#define RST_PIN 9
#define SS_PIN 10

MFRC522 mfrc522(SS_PIN, RST_PIN);
#define NEW_UID {0xDE, 0xAD, 0xBE, 0xEF}
MFRC522::MIFARE_Key key;

void setup() {
  Wire.begin();
  pinMode(2, INPUT);
  Serial.begin(9600);
  while (!Serial);
  SPI.begin();
  mfrc522.PCD_Init();
  for (byte i = 0; i < 6; i++) {
    key.keyByte[i] = 0xFF;
  }
}
//-------------------------------------------------------------//
//-FUNCTION     : printDate
//-AUTOR        : Nanchen Jean
//-DESCRIPTION  : This function print the actual date and hour
//-               the format is : [DD.MM.20YY HH:MM:SS]
//-------------------------------------------------------------//
void printDate() {
  //Separator
  Serial.print("[");
  //Print the date
  Serial.print(Clock.getDate(), DEC);
  Serial.print(".");

  //Print the month
  Serial.print(Clock.getMonth(Century), DEC);
  Serial.print(".");

  //Print the year
  Serial.print("20");
  Serial.print(Clock.getYear(), DEC);

  //Separator
  Serial.print(" ");

  // Finally the hour, minute, and second
  Serial.print(Clock.getHour(h12, PM), DEC);
  Serial.print(":");
  Serial.print(Clock.getMinute(), DEC);
  Serial.print(':');
  Serial.print(Clock.getSecond(), DEC);
  //Separator
  Serial.print("] : ");
}
//-------------------------------------------------------------//
//-FUNCTION     : readNFC
//-AUTOR        : -
//-DESCRIPTION  : This function read and store the UID car
//-               in the tab COD[], COD[] will be print too
//-------------------------------------------------------------//
void readNFC() {
  for (byte i = 0; i < (mfrc522.uid.size); i++) {
    COD[i] = mfrc522.uid.uidByte[i];
  }
  Serial.print("UID : ");
  Serial.print(COD[0], DEC);
  Serial.print(".");
  Serial.print(COD[1], DEC);
  Serial.print(".");
  Serial.print(COD[2], DEC);
  Serial.print(".");
  Serial.println(COD[3], DEC);
}

//-------------------------------------------------------------//
//-FUNCTION     : pairNFC
//-AUTOR        : -
//-DESCRIPTION  : This function add a new card UID in the
//-               eeprom, we detect if the card is already in.
//-------------------------------------------------------------//
void pairNFC() {
  //PRINT THE UID CARD
  printDate();
  Serial.print("Card detected, UID : ");
  Serial.print(COD[0], DEC);
  Serial.print(".");
  Serial.print(COD[1], DEC);
  Serial.print(".");
  Serial.print(COD[2], DEC);
  Serial.print(".");
  Serial.println(COD[3], DEC);
  long r = 0;
  int c = 0;
  for (int i = 1; i <= EEPROM.read(0); i++) {
    switch (i % 4) {
      case 1 : {
          AUX[0] = EEPROM.read(i);
          break;
        }
      case 2 : {
          AUX[1] = EEPROM.read(i);
          break;
        }
      case 3 : {
          AUX[2] = EEPROM.read(i);
          break;
        }
      case 0 : {
          AUX[3] = EEPROM.read(i);
          break;
        }
    }
    if ((i) % 4 == 0) {
      Serial.println(r);
      if ( AUX[0] == COD[0] && AUX[1] == COD[1] && AUX[2] == COD[2] && AUX[3] == COD[3] ) {
        printDate();
        Serial.println("CARD ALREADY IN THE SYSTEM");
        delay(2000);
        c = 1;
        break;
      }
    }
  }
  if (c == 0) {
    int aux2 = EEPROM.read(0);
    printDate();
    Serial.println("CARD HAS BEEN STORED, UID : ");
    Serial.print(COD[0], DEC);
    Serial.print(".");
    Serial.print(COD[1], DEC);
    Serial.print(".");
    Serial.print(COD[2], DEC);
    Serial.print(".");
    Serial.print(COD[3], DEC);
    EEPROM.write(aux2 + 1, COD[0]);
    EEPROM.write(aux2 + 2, COD[1]);
    EEPROM.write(aux2 + 3, COD[2]);
    EEPROM.write(aux2 + 4, COD[3]);
    aux2 = aux2 + 4; // Position for a new code
    Serial.println("aux2");
    Serial.println(aux2);
    EEPROM.write(0, 0);
    EEPROM.write(0, aux2);
    delay(2000);
  }
}
//-------------------------------------------------------------//
//-FUNCTION     : validationNFC
//-AUTOR        : -
//-DESCRIPTION  : This function return true if the CARD is in
//-               the EEPROM
//-------------------------------------------------------------//
boolean validationNFC() {
  boolean c = false;
  for (int i = 1; i <= EEPROM.read(0); i++) {
    switch (i % 4) {
      case 1 : {
          AUX[0] = EEPROM.read(i);
          break;
        }
      case 2 : {
          AUX[1] = EEPROM.read(i);
          break;
        }
      case 3 : {
          AUX[2] = EEPROM.read(i);
          break;
        }
      case 0 : {
          AUX[3] = EEPROM.read(i);
          break;
        }
    }

    if ((i) % 4 == 0)
    {
      if ( AUX[0] == COD[0] && AUX[1] == COD[1] && AUX[2] == COD[2] && AUX[3] == COD[3])
        c = true; //Verify if the code is in EEPROM and make flag=true;
    }
  }
  return c;
}
//-------------------------------------------------------------//
//-FUNCTION     : loop
//-AUTOR        : -
//-DESCRIPTION  : loop function, contains a switch case (state),
//-               state = 0, we check if there is a card, and
//-                          look if the card is inside the
//-                          eeprom. If valid card, state=1.
//-                          If we press the btn (2), state = 2
//-               state = 1, sequence of the opening and closing
//-                          door
//-               state = 2, We go here if we press the btn in
//-                          state = 0. In this state we add
//-                          a new card and check if already in.
//-------------------------------------------------------------//
void loop() {
  // put your main code here, to run repeatedly:
  switch (state) {

    case 0: {
        mfrc522.PCD_Init();

        if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
          readNFC();
          if (validationNFC())
          {
            state = 1;
            printDate();
            Serial.println("VALID NFC CARD");
            delay(1000);
            return;
          }
          else {
            printDate();
            Serial.println("INVALID NFC CARD");
            delay(1000);
            printDate();
            Serial.println("BLOCKED");
            return;
          }
        }
        if (digitalRead(2) == 1)
        {
          Serial.println("MODE 2");
          delay(500);
          state = 2;
        }

        break;
      }


    case 1: {
        printDate();
        Serial.println( "UNLOCKED");

        /*

              digitalWrite(A3,LOW);

              digitalWrite(A1,LOW); //The red LED will be off

              digitalWrite(A2,HIGH); //The green LED will be on

              tone(6,3000,5010); //The buzzer will make a sound
        */

        delay(5000); //After 5 seconds the system will be blocked
        /*

              digitalWrite(A3,HIGH);

              digitalWrite(A1,HIGH);

              digitalWrite(A2,LOW);
        */
        state = 0;
        printDate();
        Serial.println( "BLOCKED");
        break;
      }
    case 2: {

        mfrc522.PCD_Init();

        if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
          readNFC();
          pairNFC();
          state = 0;
          delay(2000);
          printDate();
          Serial.println( "BLOCKED");
        }
        break;

      }

  }
}
