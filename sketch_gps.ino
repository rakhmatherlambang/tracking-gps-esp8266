#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>
#include <TinyGPS++.h>
#include <SoftwareSerial.h>

const char* ssid = "RIDHO FERNANDO";
const char* password = "ridho1710";

// Inisialisasi Bot Token
#define BOTtoken "7206639866:AAEK1GmIGdeKUc_CTlO3FPQKQIKZtFf2XSw"  // Bot Token dari BotFather
#define CHAT_ID "6293314881"

WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

int botRequestDelay = 1000; // Delay antara request ke bot
unsigned long lastTimeBotRan;

// Set up SoftwareSerial on pins D1 (RX) and D2 (TX)
SoftwareSerial GPSSerial(D2, D1);  // (RX, TX)
TinyGPSPlus gps;

void handleNewMessages(int numNewMessages) {
  Serial.println("handleNewMessages");
  Serial.println(String(numNewMessages));

  for (int i = 0; i < numNewMessages; i++) {
    String chat_id = String(bot.messages[i].chat_id);
    if (chat_id != CHAT_ID) {
      bot.sendMessage(chat_id, "Unauthorized user", "");
      continue;
    }
    String text = bot.messages[i].text;
    Serial.println("Message: " + text);

    String from_name = bot.messages[i].from_name;
    
    while (GPSSerial.available()) {
      char c = GPSSerial.read();
      gps.encode(c);
      Serial.print(c);  // Output setiap karakter yang diterima dari GPS
    }
    
    if (gps.charsProcessed() > 10) {
      float currentLat = gps.location.lat();
      float currentLng = gps.location.lng();

      if (text == "/start") {
        String control = "Selamat Datang, " + from_name + ".\n";
        control += "Gunakan Commands Di Bawah Untuk Monitoring Lokasi GPS\n\n";
        control += "/Lokasi Untuk Mengetahui lokasi saat ini \n";
        bot.sendMessage(chat_id, control, "");
        Serial.println("Sent /start response");
      } else if (text == "/Lokasi") {
        String lokasi = "Lokasi : https://www.google.com/maps/@";
        lokasi += String(currentLat, 6);
        lokasi += ",";
        lokasi += String(currentLng, 6);
        lokasi += ",21z?entry=ttu";
        bot.sendMessage(chat_id, lokasi, "");
        Serial.println("Sent /Lokasi response");
      }
    } else {
      Serial.println("No GPS data available");
    }
  }
}

void setup() {
  Serial.begin(9600);
  GPSSerial.begin(9600);

  // Koneksi ke WiFi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }
  // Print ESP8266 Local IP Address
  Serial.println("Connected to WiFi");
  Serial.println(WiFi.localIP());

  // Set waktu root certificate dari server Telegram
  client.setInsecure();
}

void loop() {
  if (millis() > lastTimeBotRan + botRequestDelay) {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

    while (numNewMessages) {
      Serial.println("Got response from Telegram");
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
    lastTimeBotRan = millis();
  }
}
