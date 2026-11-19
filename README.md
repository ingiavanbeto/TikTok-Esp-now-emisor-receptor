# 📡 ESP-NOW: Emisor y Receptor con ESP32

![Arduino IDE](https://img.shields.io/badge/Arduino_IDE-00979D?style=for-the-badge&logo=arduino&logoColor=white)
![ESP32](https://img.shields.io/badge/ESP32-E7352C?style=for-the-badge&logo=espressif&logoColor=white)
![ESP-NOW](https://img.shields.io/badge/Protocolo-ESP--NOW-007BFF?style=for-the-badge)
![C++](https://img.shields.io/badge/C++-00599C?style=for-the-badge&logo=cplusplus&logoColor=white)

> **Dos placas ESP32 hablando entre ellas SIN router, SIN internet y SIN cables.**
> El emisor manda un contador cada segundo por **ESP-NOW** y el receptor lo muestra en el monitor serial, en un LED… ¡y hasta en una pantallita OLED! 🖥️

Este proyecto acompaña al video de **ESP-NOW** del canal de TikTok **«Profe Iván Cruz»** 🎓: primero la teoría y ahora la práctica, con código real y probado en hardware real.

---

## 🔁 ¿Cómo funciona?

```
┌──────────────────┐      ESP-NOW · Canal 1       ┌────────────────────┐
│      EMISOR      │  ──────────────────────────▶  │      RECEPTOR      │
│  ESP32-S3 mini   │   contador (4 bytes) cada 1 s │  ESP32-WROOM-32    │
│                  │                               │  o Heltec V2 📺    │
└──────────────────┘                               └────────────────────┘
        ▲                                                    │
        └────────────  ACK (acuse de recibo)  ◀──────────────┘
```

1. El **emisor** arma un paquetito con un contador (`uint32_t`, apenas 4 bytes de los 250 que permite ESP-NOW) y lo envía con `esp_now_send()`.
2. El **receptor** recibe el paquete con el callback `OnDataRecv`, que se dispara solo en cuanto llega un paquete: no hay que estar preguntando.
3. El emisor confirma cada envío con el callback `OnDataSent`: si el receptor contestó el **ACK**, el envío fue exitoso. ✅

---

## 🧪 Hardware probado

| Rol | Placa | Sketch | Notas |
|---|---|---|---|
| 📤 **Emisor** | **ESP32-S3 mini** | [`esp_now_emisor/`](esp_now_emisor/esp_now_emisor.ino) | LED en GPIO 8, parpadea con cada envío |
| 📥 **Receptor sin pantalla** | **ESP32-WROOM-32** | [`esp_now_receptor/`](esp_now_receptor/esp_now_receptor.ino) | Salida por monitor serial; LED en GPIO 25 |
| 📥 **Receptor con pantalla** | **Heltec WiFi LoRa 32 V2** | [`esp_now_receptor_pantallita/`](esp_now_receptor_pantallita/esp_now_receptor_pantallita.ino) | OLED SSD1306 de 128×64 integrada |

---

## 📂 Los 3 sketches

### 1️⃣ `esp_now_emisor` — el que envía 📤

- WiFi en **modo estación** (`WIFI_STA`) y **canal fijo 1** con `esp_wifi_set_channel()`.
- Registra al receptor como **par** con `esp_now_add_peer()` usando su dirección MAC.
- Cada **1 segundo** envía el contador con `esp_now_send()` (sin `delay()` bloqueante: usa `millis()` 😉).
- El callback `OnDataSent` imprime por Serial si el envío tuvo **ACK OK** o **FALLO**.

### 2️⃣ `esp_now_receptor` — el que recibe, sin pantalla 📥

- Registra el callback `OnDataRecv` con `esp_now_register_recv_cb()`.
- Valida que el paquete tenga el tamaño correcto, guarda el contador e imprime por Serial: `Paquete #N de XX:XX:XX:XX:XX:XX`.
- El **LED (GPIO 25)** permanece encendido mientras lleguen paquetes y se apaga si pasan 3 segundos sin recibir nada: un vistazo basta para saber si el enlace está vivo. 💡

### 3️⃣ `esp_now_receptor_pantallita` — el que recibe, con pantalla 📺

- Todo lo del receptor normal, **más** la pantalla OLED del Heltec (SSD1306 128×64, I2C por software en los pines 15/4/16) usando la librería **U8g2**.
- La pantalla muestra: su propia MAC, el último contador recibido, el total de paquetes y la MAC del emisor. Se refresca cada 500 ms.

---

## ⚠️ Antes de subir el código: la dirección MAC

ESP-NOW no usa contraseñas ni se conecta a una red: cada placa se identifica con su **dirección MAC**, y el emisor necesita saber la MAC del receptor.

1. Sube primero el sketch del **receptor** y abre el monitor serial a **115200 baudios**: al arrancar imprime su MAC (`MAC receptor: ... <-- esta MAC va en el emisor`).
2. Pega esa MAC en el arreglo `receptorMAC[]` del **emisor**.
3. Sube el sketch del emisor… ¡y a ver paquetes volar! 🚀

> En este repositorio el emisor viene apuntando a la MAC del **Heltec con pantalla** (`E0:E2:E6:00:8E:3C`) y trae **comentada** la MAC del receptor sin pantalla (`24:0A:C4:60:CE:30`): descomenta la que vayas a usar.
>
> 🔑 **Clave:** emisor y receptor fijan el **canal 1** del WiFi. Si los canales no coinciden, no se escuchan.

---

## 🛠️ Requisitos

- **Arduino IDE** con el **core de ESP32 de Espressif versión 3.x** (el código usa la API actual de ESP-NOW: `esp_now_recv_info_t`, `wifi_tx_info_t`).
- Solo para la versión con pantalla: la librería **U8g2** (de *olikraus*), instalable desde el Gestor de Librerías del IDE.

---

## 📚 Documentación oficial de Espressif

- 📖 **ESP-NOW · Referencia de la API (ESP-IDF)** — todas las funciones: `esp_now_init()`, `esp_now_add_peer()`, `esp_now_send()`, los callbacks y más:
  https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/network/esp_now.html
- 📘 **ESP-NOW User Guide (PDF oficial)** — el protocolo a fondo: emparejamiento, cifrado, tasas y alcance:
  https://www.espressif.com/sites/default/files/documentation/esp-now_user_guide_en.pdf
- ❓ **FAQ oficial de ESP-NOW** — preguntas frecuentes y datos prácticos de alcance y velocidad:
  https://docs.espressif.com/projects/esp-faq/en/latest/application-solution/esp-now.html
- 💻 **Repositorio oficial esp-now de Espressif** (componente y ejemplos para ESP-IDF):
  https://github.com/espressif/esp-now
- 🔧 **Core de ESP32 para Arduino** (repositorio oficial de Espressif):
  https://github.com/espressif/arduino-esp32
- 🧠 **ESP32-S3** — página oficial del chip del emisor:
  https://www.espressif.com/en/products/socs/esp32-s3
- 🧠 **ESP32** — página oficial del chip de los receptores:
  https://www.espressif.com/en/products/socs/esp32
- 📺 **Heltec WiFi LoRa 32** — documentación oficial de la placa con pantalla:
  https://docs.heltec.org/en/node/esp32/wifi_lora_32/index.html

---

## 🎬 Créditos

Hecho por el **Profe Iván Cruz** para sus clases y su canal de TikTok **«Profe Iván Cruz»**, donde este código se explica paso a paso. 🎓

¿Lo probaste con otras placas ESP32? ¿Qué mandarías tú por ESP-NOW? 👀
