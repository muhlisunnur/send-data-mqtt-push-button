#include <WiFi.h>          // Gunakan <ESP8266WiFi.h> jika menggunakan NodeMCU ESP8266
#include <PubSubClient.h>

// --- Konfigurasi Wi-Fi & MQTT ---
const char* ssid         = "TECNO CAMON 40 Pro 5G";     // Ganti dengan SSID WiFi Anda
const char* password     = "santrifiwa"; // Ganti dengan Password WiFi Anda
const char* mqtt_server  = "test.mosquitto.org"; // Contoh: "192.168.1.50" atau domain broker
const int mqtt_port      = 1883;

const int buttonPin = 14;  // Nomor pin tombol

int buttonState = LOW;      // Menyimpan status tombol saat ini
int lastButtonState = LOW;  // Menyimpan status tombol pada loop sebelumnya

unsigned long lastDebounceTime = 0;  
const unsigned long debounceDelay = 50; // Waktu tunggu debounce (50 milidetik)

WiFiClient espClient;
PubSubClient client(espClient);

// Fungsi untuk menghubungkan ke Wi-Fi
void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

// Fungsi untuk menghubungkan ulang ke MQTT Broker jika terputus
void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Membuat Client ID unik menggunakan MAC Address agar tidak bentrok
    String clientId = "ESP32Client-" + String(random(0xffff), HEX);
    
    if (client.connect(clientId.c_str())) {
      Serial.println("connected to Mosquitto");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void setup() {
  pinMode(buttonPin, INPUT);
  Serial.begin(115200);
  
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
}

void loop() {
  // Pastikan ESP tetap terhubung ke Mosquitto broker
  if (!client.connected()) {
    reconnect();
  }
  client.loop(); // Menjaga koneksi MQTT tetap aktif

  // 1. Baca status tombol saat ini
  int reading = digitalRead(buttonPin);

  // 2. Cek apakah ada perubahan status tombol
  if (reading != lastButtonState) {
    lastDebounceTime = millis(); // Reset timer debounce
  }

  // 3. Cek apakah status sudah stabil selama waktu debounce
  if ((millis() - lastDebounceTime) > debounceDelay) {
    
    // Jika status tombol memang benar-benar berubah secara stabil
    if (reading != buttonState) {
      buttonState = reading;

      // 4. HANYA bertindak saat tombol ditekan (transisi ke HIGH)
      if (buttonState == HIGH) {
        Serial.println("Button Pressed. Sending MQTT..."); 
        
        // 5. Mengirim data ke Mosquitto Broker
        // Topic: /santrifiwa, Payload: data terkirim
        if (client.publish("/santrifiwa", "data terkirim")) {
          Serial.println("MQTT Publish Success!");
        } else {
          Serial.println("MQTT Publish Failed!");
        }
      }
    }
  }

  // 5. Simpan status pembacaan untuk loop berikutnya
  lastButtonState = reading;
}
