// Exemple CHATGPT
#include <Arduino.h>
#include "SmartRC_CC1101.h"
#include "driver/rmt.h"
#include "esp_err.h"
#include <stdint.h>

static constexpr gpio_num_t TX_DATA_PIN = GPIO_NUM_10;
static constexpr rmt_channel_t RMT_CHANNEL = RMT_CHANNEL_0;

// Positif = ON, négatif = OFF. Toutes les durées sont en µs.
const int16_t pulsesUs[] = {
  443, -423, 361, -482, 339, -530, 325, -538, 357, -526, 341, -486,
  385, -504, 355, -498, 367, -516, 351, -502, 387, -478, 403, -452,
  413, -4376, 409, -872, 413, -912, 411, -880, 407, -878, 437, -878,
  861, -450, 851, -448, 413, -884, 417, -872, 873, -428, 453, -848,
  449, -874, 415, -878, 447, -846, 869, -454, 855, -452, 877, -412,
  441, -868, 873, -436, 879, -432, 871, -432, 411, -882, 883, -424,
  885, -422, 425, -872, 899, -412, 869, -446, 413, -888, 879, -412,
  875, -450, 439, -848, 461, -846, 883, -418, 451, -856, 449, -856,
  447, -852, 877, -442, 417, -870, 901, -430, 845, -460, 869, -434,
  417, -884, 853, -450, 441, -876, 869, -422, 885, -418, 869, -426,
  443, -882, 851, -450, 443, -874, 869, -418, 443, -880, 431, -872,
  411, -886, 883, -416, 445, -846, 891, -418, 881, -448, 875, -412,
  875, -440, 875, -438, 875, -434, 877, -434, 417, -884, 857, -452,
  855, -17344, 463, -414, 443, -440, 467, -380, 479, -428, 415, -448,
  453, -388, 479, -412, 441, -420, 451, -416, 453, -416, 451, -430,
  455, -4326, 439, -874, 439, -846, 439, -874, 439, -872, 443, -858,
  881, -396, 905, -410, 459, -844, 465, -844, 893, -422, 451, -856,
  449, -856, 445, -876, 417, -886, 855, -416, 897, -424, 901, -398,
  449, -880, 885, -414, 865, -428, 913, -396, 451, -852, 893, -414,
  889, -416, 451, -856, 883, -452, 871, -422, 449, -854, 877, -436,
  877, -436, 415, -878, 425, -878, 889, -420, 449, -850, 451, -844,
  479, -842, 865, -422, 455, -854, 911, -420, 875, -414, 907, -410,
  447, -868, 875, -414, 445, -856, 881, -426, 877, -428, 913, -396,
  453, -878, 855, -416, 473, -850, 897, -410, 439, -876, 439, -874,
  445, -854, 877, -430, 441, -844, 905, -422, 887, -420, 879, -414,
  877, -442, 875, -436, 875, -436, 875, -434, 415, -880, 883, -418
};

constexpr size_t pulseCount = sizeof(pulsesUs) / sizeof(pulsesUs[0]);
constexpr size_t itemCount = pulseCount / 2;

void setupRmt() {
  rmt_config_t config = RMT_DEFAULT_CONFIG_TX(TX_DATA_PIN, RMT_CHANNEL);

  // ESP32 classique : horloge 80 MHz / diviseur 80 = 1 MHz,
  // donc un tick RMT correspond à 1 µs.
  config.clk_div = 80;
  config.tx_config.loop_en = false;
  config.tx_config.carrier_en = false;
  config.tx_config.idle_output_en = true;
  config.tx_config.idle_level = RMT_IDLE_LEVEL_LOW;

  ESP_ERROR_CHECK(rmt_config(&config));
  ESP_ERROR_CHECK(rmt_driver_install(RMT_CHANNEL, 0, 0));
}

void sendPulseSequence() {
  static rmt_item32_t items[itemCount];

  for (size_t i = 0; i < pulseCount; i += 2) {
    const int16_t onUs = pulsesUs[i];
    const int16_t offUs = pulsesUs[i + 1];

    // Vérifie que la séquence alterne bien ON positif et OFF négatif.
    if (onUs <= 0 || offUs >= 0) {
      Serial.println("Erreur : format des durées ON/OFF inattendu.");
      return;
    }

    // Les champs duration RMT acceptent jusqu'à 32767 ticks.
    // La plus longue durée fournie est 17344 µs, donc elle tient.
    items[i / 2].duration0 = onUs;
    items[i / 2].level0 = 1;
    items[i / 2].duration1 = -offUs;
    items[i / 2].level1 = 0;
  }

  ESP_ERROR_CHECK(
    rmt_write_items(RMT_CHANNEL, items, itemCount, true)
  );
}

void setup() {
  Serial.begin(115200);
  delay(200);

  static_assert(pulseCount % 2 == 0, "Il faut un nombre pair de durées.");

  setupRmt();

  ELECHOUSE_cc1101.Init();
  ELECHOUSE_cc1101.setMHZ(868.350);
  ELECHOUSE_cc1101.setModulation(2);  // ASK/OOK
  ELECHOUSE_cc1101.setPktFormat(2);   // Mode asynchrone

  delay(10);
  ELECHOUSE_cc1101.SetTx();
  delay(2);

}

void loop() {
  sendPulseSequence();
  Serial.println("Séquence émise une fois.");
  for (int i=0; i++;i<10)
    {
    delay(1000);
    Serial.print('.');
    }
}