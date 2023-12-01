#include <CTBot.h>
#include <SPI.h>
#include <MFRC522.h>

#define SS_PIN 4      // D2
#define RST_PIN 5     // D1
#define BTN_PIN 0     // D3
#define relay 2       // D4
#define LED_GREEN 10  // S3 atau SD3
#define LED_RED 9     // S2 atau SD2

CTBot mybot;
const String ssid = "MITRATEL JABO 2.4G";
const String pass = "123mitratel";
const String token = "6411164664:AAF5t7QhS7HEv7ftK0GsOjEt3mYmF4iRTSo";
const int ChatId = 863031403;

MFRC522 rfid(SS_PIN, RST_PIN);
MFRC522::MIFARE_Key key;

bool accessGranded = false;

void setup() {
  Serial.begin(9600);
  SPI.begin();
  rfid.PCD_Init();

  mybot.wifiConnect(ssid, pass);
  mybot.setTelegramToken(token);

  pinMode(relay, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_RED, OUTPUT);

  Serial.println("Starting Telegram Bot.....");
  Serial.println("Waiting card ......");

  digitalWrite(LED_RED, HIGH);
  digitalWrite(relay, HIGH);

  if (mybot.testConnection()) {
    Serial.println("Koneksi bagus");
  } else {
    Serial.println("Konensi kurang bagus");
  }

  mybot.sendMessage(ChatId, "Test kirim pesan ke telegram bot");
  Serial.println("Pesan terkirim");
}

void loop() {

  if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial())
    return;

  Serial.print(F("PICC type: "));
  MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak);
  Serial.println(rfid.PICC_GetTypeName(piccType));
  // Check is the PICC of Classic MIFARE type
  if (piccType != MFRC522::PICC_TYPE_MIFARE_MINI && piccType != MFRC522::PICC_TYPE_MIFARE_1K && piccType != MFRC522::PICC_TYPE_MIFARE_4K) {
    Serial.println(F("Your tag is not of type MIFARE Classic."));
    return;
  }
  String strID = "";
  for (byte i = 0; i < 4; i++) {
    strID +=
      (rfid.uid.uidByte[i] < 0x10 ? "0" : "") + String(rfid.uid.uidByte[i], HEX) + (i != 3 ? ":" : "");
  }
  strID.toUpperCase();
  Serial.print("Tap card key: ");
  Serial.println(strID);
}

void unlock() {
  Serial.println("Authorized Success");
  digitalWrite(LED_GREEN, HIGH);
  digitalWrite(LED_RED, LOW);
  digitalWrite(relay, LOW);
  delay(5000);
  digitalWrite(LED_GREEN, LOW);
  digitalWrite(LED_RED, HIGH);
  digitalWrite(relay, HIGH);
}