// ============================================================
// RECEPTOR ESP-NOW - Heltec WiFi LoRa 32 V2 (ESP32)
// Recibe el contador del emisor ESP32-C3 SuperMini
// ============================================================
#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>

#define LED 25
#define CANAL_WIFI 1

typedef struct {
  uint32_t contador;
} Mensaje;

volatile uint32_t ultimoPaquete = 0;
volatile uint32_t paquetes = 0;
volatile uint32_t ultimoContador = 0;
volatile uint32_t paquetesInvalidos = 0;
volatile uint32_t ultimoTamano = 0;

// Callback de recepcion
void OnDataRecv(const esp_now_recv_info_t *info,const uint8_t *data,int len) {
  if (len == sizeof(Mensaje)) {
    Mensaje recibido;
    memcpy(&recibido, data, sizeof(recibido));
    ultimoContador = recibido.contador;
    ultimoPaquete = millis();
    paquetes++;

    Serial.print("Paquete #");
    Serial.print(recibido.contador);
    Serial.print(" de ");
    for (int i = 0; i < 6; i++) {
      Serial.printf("%02X", info->src_addr[i]);
      if (i < 5) Serial.print(":");
    }
    Serial.println();
  } 
  else {
    paquetesInvalidos++;
    ultimoTamano = len;
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  pinMode(LED, OUTPUT);
  digitalWrite(LED, LOW);

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(500);

  esp_err_t resultado = esp_wifi_set_channel(CANAL_WIFI, WIFI_SECOND_CHAN_NONE);
  if (resultado == ESP_OK) {
    Serial.println("Canal WiFi configurado: 1");
  } else {
    Serial.printf("Error canal: %d\n", resultado);
  }
  uint8_t mac[6];
  if (esp_wifi_get_mac(WIFI_IF_STA, mac) == ESP_OK) {
    Serial.print("MAC receptor: ");
    for (int i = 0; i < 6; i++) {
      Serial.printf("%02X", mac[i]);
      if (i < 5) Serial.print(":");
    }
    Serial.println("  <-- esta MAC va en el emisor");
  }

  Serial.println("HELTEC WIFI LORA 32 V2");

  if (esp_now_init() != ESP_OK) {
    Serial.println("Error iniciando ESP-NOW");
    return;
  }

  esp_now_register_recv_cb(OnDataRecv);
  Serial.println("ESP-NOW iniciado");
  Serial.println("Esperando paquetes...");
}

void loop() {
  static uint32_t ultimoReporte = 0;
  uint32_t ahora = millis();
  uint32_t ultimo = ultimoPaquete;
  uint32_t contador = ultimoContador;
  uint32_t total = paquetes;
  uint32_t invalidos = paquetesInvalidos;
  uint32_t tamano = ultimoTamano;

  if (total > 0 && (uint32_t)(ahora - ultimo) < 3000) {
    digitalWrite(LED, HIGH);
  } else {
    digitalWrite(LED, LOW);
  }

  if (ahora - ultimoReporte >= 1000) {
    ultimoReporte = ahora;
    //Serial.print("Paquetes recibidos: ");
    //Serial.println(total);
    //Serial.print("Paquetes invalidos: ");
    //Serial.println(invalidos);

    //if (total > 0) {
    //  Serial.print(" Ultimo contador: ");
    //  Serial.println(contador);
    //}
    if (invalidos > 0) {
      Serial.print(" Ultimo tamano recibido: ");
      Serial.println(tamano);
    }
  }
}
24:0A:C4:60:CE:30