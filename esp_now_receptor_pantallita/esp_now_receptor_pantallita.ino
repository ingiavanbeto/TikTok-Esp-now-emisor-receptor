#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>
#include <U8g2lib.h>   

U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0,  15, 4, 16);

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

char receptorMACtxt[18] = "---";
char emisorMACtxt[18] = "---";
volatile bool hayPaquete = false;

void macATexto(const uint8_t *mac, char *txt) {
  snprintf(txt, 18, "%02X:%02X:%02X:%02X:%02X:%02X",
           mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
}

void OnDataRecv(const esp_now_recv_info_t *info,const uint8_t *data,int len) {
  if (len == sizeof(Mensaje)) {
    Mensaje recibido;
    memcpy(&recibido, data, sizeof(recibido));
    ultimoContador = recibido.contador;
    ultimoPaquete = millis();
    paquetes++;
    macATexto(info->src_addr, emisorMACtxt);
    hayPaquete = true;

    Serial.print("Paquete #");
    Serial.print(recibido.contador);
    Serial.print(" de ");
    Serial.println(emisorMACtxt);
  } 
  else {
    paquetesInvalidos++;
    ultimoTamano = len;
  }
}

void dibujarPantalla() {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_6x10_tf);

  u8g2.drawStr(0, 10, "RECEPTOR ESP-NOW");
  u8g2.drawStr(0, 24, "MAC RX:");
  u8g2.drawHLine(0, 13, 128);
  u8g2.drawStr(0, 34, receptorMACtxt);
  u8g2.drawHLine(0, 37, 128);

  char linea[24];
  snprintf(linea, sizeof(linea), "PAQ #%lu  TOT %lu",(unsigned long)ultimoContador, (unsigned long)paquetes);
  u8g2.drawStr(0, 48, linea);

  // MAC del emisor del ultimo paquete
  u8g2.drawStr(0, 59, "DE:");
  if (hayPaquete) {
    u8g2.drawStr(24, 59, emisorMACtxt);
  } else {
    u8g2.drawStr(24, 59, "esperando...");
  }

  u8g2.sendBuffer();
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  pinMode(LED, OUTPUT);
  digitalWrite(LED, LOW);

  // Iniciar pantalla
  u8g2.begin();

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
    macATexto(mac, receptorMACtxt);
    Serial.print("MAC receptor: ");
    Serial.print(receptorMACtxt);
    Serial.println("  <-- esta MAC va en el emisor");
  }

  Serial.println("HELTEC WIFI LORA 32 V2");

  if (esp_now_init() != ESP_OK) {
    Serial.println("Error iniciando ESP-NOW");
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_6x10_tf);
    u8g2.drawStr(0, 10, "RECEPTOR ESP-NOW");
    u8g2.drawStr(0, 24, "Error ESP-NOW");
    u8g2.sendBuffer();
    return;
  }

  esp_now_register_recv_cb(OnDataRecv);
  Serial.println("ESP-NOW iniciado");
  Serial.println("Esperando paquetes...");

  dibujarPantalla();
}

void loop() {
  static uint32_t ultimoReporte = 0;
  static uint32_t ultimoDibujo = 0;
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

  // Redibujar la pantalla cada 500 ms
  if (ahora - ultimoDibujo >= 500) {
    ultimoDibujo = ahora;
    dibujarPantalla();
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

