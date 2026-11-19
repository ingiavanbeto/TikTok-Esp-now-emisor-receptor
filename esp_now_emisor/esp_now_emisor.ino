// ============================================================
// EMISOR ESP-NOW - ESP32-C3 SuperMini
// Envía un contador cada 500 ms al receptor Heltec WiFi LoRa 32 V2
// ============================================================
#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>
#define LED 8 

// uint8_t receptorMAC[] = { 0x24, 0x0A, 0xC4, 0x60, 0xCE, 0x30 }; //MAC sin pantalla
uint8_t receptorMAC[] = { 0xE0, 0xE2, 0xE6, 0x00, 0x8E, 0x3C };
uint32_t tiempoAnterior = 0;
uint32_t contador = 0;

typedef struct {
  uint32_t contador;
} Mensaje;
Mensaje datos;

void OnDataSent(const wifi_tx_info_t *info, esp_now_send_status_t status) {
  Serial.print("Envio a ");
  for (int i = 0; i < 6; i++) {
    Serial.printf("%02X", info->des_addr[i]);
    if (i < 5) Serial.print(":");
  }
  if (status == ESP_NOW_SEND_SUCCESS) 
    Serial.println(" -> ACK OK");
  else 
    Serial.println(" -> FALLO (sin ACK: revisa MAC y canal)");
}

void setup() {
  Serial.begin(115200);
  delay(500);
  pinMode(LED, OUTPUT);

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  esp_wifi_set_channel(1, WIFI_SECOND_CHAN_NONE);

  Serial.print("MAC emisor: ");
  Serial.println(WiFi.macAddress());
  Serial.print("MAC receptor objetivo: ");
  for (int i = 0; i < 6; i++) {
    Serial.printf("%02X", receptorMAC[i]);
    if (i < 5) Serial.print(":");
  }
  Serial.println();

  if (esp_now_init() != ESP_OK) {
    Serial.println("Error iniciando ESP-NOW");
    return;
  }

  esp_now_register_send_cb(OnDataSent);
  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, receptorMAC, 6);
  peerInfo.channel = 1;
  peerInfo.encrypt = false;

  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Error agregando peer");
    return;
  }
  Serial.println("Emisor iniciado, canal 1");
}

void loop() {
  if (millis() - tiempoAnterior >= 1000) {
    tiempoAnterior = millis();
    digitalWrite(LED, !digitalRead(LED));
    datos.contador = contador+=2;

    esp_err_t res = esp_now_send(receptorMAC, (uint8_t *)&datos, sizeof(datos));
    Serial.print("Enviando paquete: ");
    Serial.print(datos.contador);
    Serial.print("  esp_now_send: ");
    Serial.println(res == ESP_OK ? "OK" : "ERROR");
  }
}
