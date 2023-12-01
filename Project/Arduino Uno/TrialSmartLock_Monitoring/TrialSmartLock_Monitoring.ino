// #include <TelegramBot.h>
// #include <TelegramKeyboard.h>

#include <SPI.h>
#include <MFRC522.h>
#include <RTClib.h>
#include <Wire.h>
#include <SoftwareSerial.h>
// #include <UniversalTelegramBot.h>

#define SS_PIN 10
#define RST_PIN 9
#define TX_PIN 3
#define RX_PIN 2
#define SensorGetar 6
#define buzzer 5
#define RELAY_PIN 4
#define LED_RED_PIN 8
#define LED_GREEN_PIN 7
const String BotToken = "";
const String Chat_Id = "";

SoftwareSerial sim(RX_PIN, TX_PIN);
// UniversalTelegramBot bot(BotToken,sim);
int x;
String waktu;
String kartu1 = "RFID Tag";
String kartu2 = "RFID Card ";

RTC_DS3231 rtc;

MFRC522 rfid(SS_PIN, RST_PIN);
MFRC522::MIFARE_Key key;
bool accessGranted = false; // Variabel flag untuk mengontrol akses
bool isVibrationDetected = false;

void setup()
{
  Serial.begin(9600);
  sim.begin(9600);
  SPI.begin();
  rfid.PCD_Init();
  pinMode(SensorGetar, INPUT);
  pinMode(LED_GREEN_PIN, OUTPUT);
  pinMode(LED_RED_PIN, OUTPUT);
  pinMode(RELAY_PIN, OUTPUT);

  Serial.println("I am waiting for card...");
  
  digitalWrite(LED_RED_PIN, HIGH);
  digitalWrite(RELAY_PIN, HIGH);

  if (!rtc.begin())
  {
    Serial.println("Tidak menemukan modul RTC");
    while (1);
  }
  if (rtc.lostPower())
  {
    Serial.println("Modul RTC kehabisan daya, mohon diisi ulang");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

  delay(1000);
  sendATCommand("AT","OK");
  sendATCommand("ATEO","OK"); 
  sendATCommand("AT+CMGF=1","OK");
  sendATCommand("AT+CNMI=2,2,0,0,0","OK");
}

void loop()
{
  int statusSensorGetar = digitalRead(SensorGetar);
  if (statusSensorGetar == HIGH)
  {
    isVibrationDetected = true;
    Serial.print("Getaran Terdeteksi pada ");
    getTime();
    for (int i = 0; i <= 4; i++)
    {
      digitalWrite(LED_RED_PIN, HIGH);
      tone(buzzer,1000);
      
      delay(1000);
      digitalWrite(LED_RED_PIN, LOW);
      noTone(buzzer);
      delay(1000);
    }
    return;
  }
  else
  {
    isVibrationDetected = false;
    digitalWrite(buzzer, LOW);
    digitalWrite(LED_RED_PIN, LOW);
    // return;
  }
  if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial())
    return;

  Serial.print(F("PICC type: "));
  MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak);
  Serial.println(rfid.PICC_GetTypeName(piccType));
  // Check is the PICC of Classic MIFARE type
  if (piccType != MFRC522::PICC_TYPE_MIFARE_MINI && piccType != MFRC522::PICC_TYPE_MIFARE_1K && piccType != MFRC522::PICC_TYPE_MIFARE_4K)
  {
    Serial.println(F("Your tag is not of type MIFARE Classic."));
    return;
  }
  String strID = "";
  for (byte i = 0; i < 4; i++)
  {
    strID +=
        (rfid.uid.uidByte[i] < 0x10 ? "0" : "") + String(rfid.uid.uidByte[i], HEX) + (i != 3 ? ":" : "");
  }
  strID.toUpperCase();
  Serial.print("Tap card key: ");
  Serial.println(strID);
  if (strID.indexOf("B7:31:60:99") >= 0)
  {
    delay(2000);
    unlock();
    accessGranted = true;
    x = 0;
    // return;
  }
  else if (strID.indexOf("BA:78:6A:97") >= 0)
  {
    delay(2000);
    unlock();
    // Serial.print("dengan ");
    // Serial.println(kartu2);
    accessGranted = true;
    x = 0;
    // return;
  }
  else
  {
    // Kartu tidak diotorisasi
    delay(2000);
    accessGranted;
    Serial.println("Access denied");
    digitalWrite(LED_RED_PIN, HIGH);
    delay(5000);
    digitalWrite(LED_RED_PIN, LOW);
    return;
  }
}

void unlock()
{
  Serial.print("Pintu diakses dengan ");
  Serial.print(kartu2);
  Serial.print("pada ");
  getTime();
  tone(buzzer, 2000);
  delay(1000);
  noTone(buzzer);
  digitalWrite(LED_GREEN_PIN, HIGH);
  digitalWrite(LED_RED_PIN, LOW);
  digitalWrite(RELAY_PIN, LOW);
  delay(5000);
  digitalWrite(LED_GREEN_PIN, LOW);
  digitalWrite(LED_RED_PIN, HIGH);
  digitalWrite(RELAY_PIN, HIGH);
}

void getTime()
{
  // waktu = "";
  DateTime now = rtc.now();

  Serial.print(now.day(), DEC);
  Serial.print('-');
  Serial.print(now.month(), DEC);
  Serial.print('-');
  Serial.print(now.year(), DEC);
  Serial.print(" ");
  Serial.print(now.hour(), DEC);
  Serial.print(':');
  Serial.print(now.minute(), DEC);
  Serial.print(':');
  Serial.print(now.second(), DEC);
  Serial.println();
  // delay (1000);
}

// void alarm() {
//   for (int i = 0; i <= 3; i++) {
//     digitalWrite(LED_RED_PIN, HIGH);
//     Serial.println("Getaran terdeteksi");
//     delay(1000);
//     digitalWrite(LED_RED_PIN, LOW);
//   }
// }

void kirimPesan(String pesan)
{
  String url =  "https://api.telegram.org/bot" + BotToken + "/sendMessage?chat_id=" + Chat_Id + "&text=" + pesan;

  sendATCommand("AT+HTTPINIT","OK");
  sendATCommand("AT+HTTPPARA=\"CID\",1","OK");
  sendATCommand("AT+HTTPPARA=\"URL\",\""+url+"\"","OK");
  sendATCommand("AT+HTTPACTION=0","OK");
  sendATCommand("AT+HTTPTERM","OK");
}

void sendATCommand(String command, String expectedResponse)
{
  sim.println(command);
  delay(500);

  while (sim.available() > 0)
  {
    String response = sim.readStringUntil('\n');
    if (response.indexOf(expectedResponse) != -1)
    {
      break;
    }
  }
}
