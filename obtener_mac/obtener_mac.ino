// ============================================================
// OBTENER MAC - Paso 0 obligatorio antes de usar ESP-NOW
// ------------------------------------------------------------
//
// Abre el monitor serial a 115200 baudios. Imprime la MAC
// del modo estacion (la que usa ESP-NOW) en dos formatos:
//   1) Normal:            24:0A:C4:60:CE:30
//   2) Lista para pegar:  { 0x24, 0x0A, 0xC4, 0x60, 0xCE, 0x30 }
//      (asi, tal cual, en el arreglo receptorMAC[] del emisor)
//
// Probado en ESP32-S3 mini, ESP32-WROOM-32 y Heltec WiFi
// LoRa 32 V2: funciona en cualquier ESP32.
// ============================================================
#include <WiFi.h>
#include <esp_wifi.h>

uint32_t ultimoReporte = 0;

void imprimirMAC() {
  uint8_t mac[6];
  esp_wifi_get_mac(WIFI_IF_STA, mac);

  Serial.println("============================================");
  Serial.print("Placa: ");
  Serial.println(ESP.getChipModel());
  Serial.print("MAC (formato normal):       ");
  Serial.println(WiFi.macAddress());
  Serial.print("MAC (para pegar en emisor): ");
  Serial.print("{ ");
  for (int i = 0; i < 6; i++) {
    Serial.printf("0x%02X", mac[i]);
    if (i < 5) Serial.print(", ");
  }
  Serial.println(" }");
  Serial.println("============================================");
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  WiFi.mode(WIFI_STA);  // La MAC de ESP-NOW es la del modo estacion
  WiFi.disconnect();
  delay(100);

  imprimirMAC();
}

void loop() {
  if (millis() - ultimoReporte >= 5000) {
    ultimoReporte = millis();
    imprimirMAC();
  }
}
