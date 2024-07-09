#include <SPI.h>
#include <MFRC522.h>
#include <DeneyapTelegram.h>
#include <WiFiClientSecure.h>
#include <WiFi.h>

#define BOTtoken "7442722529:AAHPq9MROvJ4fl1Z2JVtbO_4PsNdWmijdTI"
#define CHAT_ID "5632619913"

const char* ssid = "Eren";
const char* password = "eren723134";

WiFiClientSecure client;
DeneyapTelegram telegram;

// Pin Tanımlamaları
#define RST_PIN D0
#define SS_PIN D4
#define BUZZER_PIN D12

MFRC522 rfid(SS_PIN, RST_PIN);

// Kullanıcı UID'leri ve isimleri
const String userUIDs[] = {"534477a3", "f33321b7", "13ca9e0d"}; // Kendi kart UID'lerinizi yazın
const String userNames[] = {"Eren", "Tuba", "Rıdvan"};

void setup() {
  Serial.begin(115200);
  SPI.begin();
  rfid.PCD_Init();

  pinMode(BUZZER_PIN, OUTPUT);

  client.setInsecure();
  Serial.print("Internete baglaniyor: ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  
  Serial.println("");
  Serial.println("WiFi baglandi");
  Serial.print("IP adresi: ");
  Serial.println(WiFi.localIP());

  telegram.begin(BOTtoken);
  if (telegram.send(CHAT_ID, "Cihaz Baglandi")) {
    Serial.println("Telegram baglantisi basarili");
  } else {
    Serial.println("Telegram baglantisi basarisiz");
  }
}

String readRFID() {
  rfid.PICC_ReadCardSerial();
  String cardUID = "";
  for (byte i = 0; i < rfid.uid.size; i++) {
    cardUID += (rfid.uid.uidByte[i] < 0x10 ? "0" : "") + String(rfid.uid.uidByte[i], HEX);
  }
  return cardUID;
}

void handleAccess(bool authorized) {
  if (authorized) {
    tone(BUZZER_PIN, 224);
    delay(500); // Buzzer'ı 0.5 saniye çalıştır
    noTone(BUZZER_PIN);
  } else {
    tone(BUZZER_PIN, 224);
    delay(5000); // Buzzer'ı 5 saniye çalıştır
    noTone(BUZZER_PIN);
  }
}

void loop() {
  if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial()) {
    return;
  }

  String cardUID = readRFID();
  Serial.print("Kart UID: ");
  Serial.println(cardUID);

  bool authorized = false;
  String userName = "Yetkisiz Kart";

  // Kart UID'sini kullanıcı UID'leriyle karşılaştır
  for (int i = 0; i < 3; i++) {
    if (cardUID == userUIDs[i]) {
      userName = userNames[i];
      authorized = true;
      break;
    }
  }

  if (telegram.send(CHAT_ID, "Kart Okundu: " + userName)) {
    Serial.println("Telegram mesaji gönderildi: " + userName);
  } else {
    Serial.println("Telegram mesaji gönderilemedi");
  }

  handleAccess(authorized);
  delay(1000); // Okuma tekrarını önlemek için bekleme süresi
}
