#include <Arduino.h>
#include <SPI.h>
#include "SmartRC_CC1101.h"

// ============================================================
// ESP32-C3 <-> CC1101
// ============================================================

#define CC1101_SCK   6
#define CC1101_MISO  2
#define CC1101_MOSI  7
#define CC1101_CS    8
#define CC1101_GDO0  1

#define BUTTON_PIN   9
#define LED_PIN      3

// ============================================================
// CC1101
// ============================================================

SmartRC_CC1101 radio;

// ============================================================
// TIMER
// ============================================================

hw_timer_t *timer = nullptr;

volatile bool transmitting = false;
volatile bool timerDone = false;

// ============================================================
// TEST OOK
//
// Format :
// positif  = ON
// négatif  = OFF
//
// Cette séquence est volontairement générique.
// ============================================================

const int16_t DATA_RAW[] = {
/*    500, -500,
    500, -500,
    500, -500,
    500, -1500,

    1500, -500,
    500, -1500,

    500, -500,
    1500, -500,

    500, -3000,

    500, -500,
    500, -500,
    1500, -500,
    500, -1500
    */
    443, -423, 361, -482, 339, -530, 325, -538, 357, -526, 341, -486, 385, -504, 355, -498, 367, -516, 351, -502, 387, -478, 403, -452, 413, -4376, 409, -872, 413, -912, 411, -880, 407, -878, 437, -878, 861, -450, 851, -448, 413, -884, 417, -872, 873, -428, 453, -848, 449, -874, 415, -878, 447, -846, 869, -454, 855, -452, 877, -412, 441, -868, 873, -436, 879, -432, 871, -432, 411, -882, 883, -424, 885, -422, 425, -872, 899, -412, 869, -446, 413, -888, 879, -412, 875, -450, 439, -848, 461, -846, 883, -418, 451, -856, 449, -856, 447, -852, 877, -442, 417, -870, 901, -430, 845, -460, 869, -434, 417, -884, 853, -450, 441, -876, 869, -422, 885, -418, 869, -426, 443, -882, 851, -450, 443, -874, 869, -418, 443, -880, 431, -872, 411, -886, 883, -416, 445, -846, 891, -418, 881, -448, 875, -412, 875, -440, 875, -438, 875, -434, 877, -434, 417, -884, 857, -452, 855, -17344, 463, -414, 443, -440, 467, -380, 479, -428, 415, -448, 453, -388, 479, -412, 441, -420, 451, -416, 453, -416, 451, -430, 455, -4326, 439, -874, 439, -846, 439, -874, 439, -872, 443, -858, 881, -396, 905, -410, 459, -844, 465, -844, 893, -422, 451, -856, 449, -856, 445, -876, 417, -886, 855, -416, 897, -424, 901, -398, 449, -880, 885, -414, 865, -428, 913, -396, 451, -852, 893, -414, 889, -416, 451, -856, 883, -452, 871, -422, 449, -854, 877, -436, 877, -436, 415, -878, 425, -878, 889, -420, 449, -850, 451, -844, 479, -842, 865, -422, 455, -854, 911, -420, 875, -414, 907, -410, 447, -868, 875, -414, 445, -856, 881, -426, 877, -428, 913, -396, 453, -878, 855, -416, 473, -850, 897, -410, 439, -876, 439, -874, 445, -854, 877, -430, 441, -844, 905, -422, 887, -420, 879, -414, 877, -442, 875, -436, 875, -436, 875, -434, 415, -880, 883, -418
};

const size_t DATA_RAW_LENGTH =
    sizeof(DATA_RAW) / sizeof(DATA_RAW[0]);

volatile size_t rawIndex = 0;

// ============================================================
// INTERRUPTION TIMER
// ============================================================

void IRAM_ATTR onTimer()
{
    if (!transmitting) {
        return;
    }

    // Fin de la séquence
    if (rawIndex >= DATA_RAW_LENGTH) {
        digitalWrite(CC1101_GDO0, LOW);

        transmitting = false;
        timerDone = true;

        timerAlarm(timer, 1000, false, 0);
        return;
    }

    int16_t duration = DATA_RAW[rawIndex];

    // Positif = ON
    // Négatif = OFF
    if (duration > 0) {
        digitalWrite(CC1101_GDO0, HIGH);
    } else {
        digitalWrite(CC1101_GDO0, LOW);
    }

    uint32_t nextDuration = abs(duration);

    rawIndex++;

    // Programmer la prochaine interruption
    timerAlarm(timer, nextDuration, false, 0);
}

// ============================================================
// ENVOI DU MOTIF DE TEST
// ============================================================

void sendTestRaw()
{
    if (transmitting) {
        return;
    }

    Serial.println();
    Serial.println("================================");
    Serial.println("Envoi test RAW OOK");
    Serial.println("================================");

    rawIndex = 0;
    timerDone = false;
    transmitting = true;

    digitalWrite(LED_PIN, HIGH);

    // Passage du CC1101 en émission
    radio.SetTx();

    // Premier état
    digitalWrite(CC1101_GDO0, HIGH);

    // Première durée : 500 us
    timerAlarm(timer, abs(DATA_RAW[0]), false, 0);
}

// ============================================================
// SETUP
// ============================================================

void setup()
{
    Serial.begin(115200);
    delay(1000);

    Serial.println();
    Serial.println("ESP32-C3 + CC1101");
    Serial.println("Test RAW OOK avec timer matériel");
    Serial.println();

    // --------------------------------------------------------
    // GPIO
    // --------------------------------------------------------

    pinMode(BUTTON_PIN, INPUT_PULLUP);
    pinMode(LED_PIN, OUTPUT);

    digitalWrite(LED_PIN, LOW);

    /*
     * Pour ce test, GDO0 est utilisé comme sortie de commande
     * du signal de test.
     */
    pinMode(CC1101_GDO0, OUTPUT);
    digitalWrite(CC1101_GDO0, LOW);

    // --------------------------------------------------------
    // CC1101
    // --------------------------------------------------------

    radio.setSpiPin(
        CC1101_SCK,
        CC1101_MISO,
        CC1101_MOSI,
        CC1101_CS
    );

    radio.setGDO0(CC1101_GDO0);

    radio.Init();

    if (!radio.getCC1101()) {
        Serial.println("ERREUR : CC1101 non détecté !");
        return;
    }

    Serial.println("CC1101 : OK");

    // --------------------------------------------------------
    // Configuration radio
    // --------------------------------------------------------

    radio.setMHZ(868.350);

    // 2 = ASK/OOK
    radio.setModulation(2);

    // Puissance faible pour le test
    radio.setPA(-10);

    Serial.println("Frequence : 868.350 MHz");
    Serial.println("Modulation : OOK");
    Serial.println("Puissance : -10 dBm");

    // --------------------------------------------------------
    // TIMER HARDWARE
    //
    // 1 MHz = 1 tick par microseconde
    // --------------------------------------------------------

    timer = timerBegin(1000000);

    if (timer == nullptr) {
        Serial.println("ERREUR : impossible de creer le timer !");
        return;
    }

    timerAttachInterrupt(timer, &onTimer);

    Serial.println("Timer materiel : OK");
    Serial.println();
    Serial.println("Appuie sur le bouton pour envoyer le test.");
}

// ============================================================
// LOOP
// ============================================================

void loop()
{
    // --------------------------------------------------------
    // Bouton
    // --------------------------------------------------------

    static bool lastButtonState = HIGH;

    bool buttonState = digitalRead(BUTTON_PIN);

    if (lastButtonState == HIGH && buttonState == LOW) {
        delay(20);

        if (digitalRead(BUTTON_PIN) == LOW) {
            sendTestRaw();
        }
    }

    lastButtonState = buttonState;

    // --------------------------------------------------------
    // Fin de transmission
    // --------------------------------------------------------

    if (timerDone) {
        timerDone = false;

        digitalWrite(CC1101_GDO0, LOW);
        digitalWrite(LED_PIN, LOW);

        radio.SetRx();

        Serial.println("Transmission terminee.");
    }

    delay(1);
}