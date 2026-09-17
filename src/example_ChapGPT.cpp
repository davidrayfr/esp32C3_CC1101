// Transfer to Mac

#include <Arduino.h>
#include <SPI.h>
#include "SmartRC_CC1101.h"

// --------------------------------------------------
// ESP32-C3 <-> CC1101
// --------------------------------------------------

#define CC1101_SCK   4
#define CC1101_MISO  5
#define CC1101_MOSI  6
#define CC1101_CS    7
#define CC1101_GDO0  10

// Bouton de test
#define BUTTON_PIN   9
#define LED_PIN      3

SmartRC_CC1101 radio;

// ===============================
// Trame RAW DE TEST
// Les valeurs sont en microsecondes.
//
// Une valeur positive = porteuse ON
// Une valeur négative = porteuse OFF
// ===============================
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

// --------------------------------------------------
// Initialisation
// --------------------------------------------------

void setup()
{
    Serial.begin(115200);
    delay(1000);

    pinMode(BUTTON_PIN, INPUT_PULLUP);
    pinMode(LED_PIN, OUTPUT);

    digitalWrite(LED_PIN, LOW);

    Serial.println();
    Serial.println("==============================");
    Serial.println(" ESP32-C3 + CC1101");
    Serial.println(" SmartRC CC1101 Driver");
    Serial.println("==============================");

    // Configuration SPI personnalisée
    radio.setSpiPin(
        CC1101_SCK,
        CC1101_MISO,
        CC1101_MOSI,
        CC1101_CS
    );

    // GDO0 si nécessaire
    radio.setGDO0(CC1101_GDO0);

    // Initialisation
    radio.Init();

    // Vérification du CC1101
    if (radio.getCC1101())
    {
        Serial.println("CC1101 : OK");
    }
    else
    {
        Serial.println("CC1101 : ERREUR");
        Serial.println("Verifier le cablage.");
        return;
    }

    // ------------------------------------------------
    // Configuration radio
    // ------------------------------------------------

    radio.setMHZ(868.350);
    //radio.setMHZ(433.92);

    // 2 = ASK/OOK
    radio.setModulation(2);

    // Puissance volontairement faible pour le test
    radio.setPA(-10);

    Serial.println();
    Serial.println("Configuration radio :");
    Serial.print("Frequence : ");
    Serial.print(radio.getMHZ());
    Serial.println(" MHz");

    Serial.println("Modulation : OOK");
    Serial.println("Puissance : -10 dBm");

    Serial.println();
    Serial.println("Pret.");
    Serial.println("Appuyer sur le bouton pour lancer");
    Serial.println("une trame OOK de TEST.");
}

// ===============================
// Génération du signal RAW
// ===============================

void sendRawData()
{
    Serial.println();
    Serial.println("===== TEST RAW OOK =====");

    digitalWrite(LED_PIN, HIGH);

    radio.SetTx();

    for (size_t i = 0; i < DATA_RAW_LENGTH; i++)
    {
        int duration = DATA_RAW[i];

        if (duration > 0)
        {
            // Porteuse ON
            //
            // Le CC1101 reste en émission pendant
            // la durée indiquée.
            delayMicroseconds(duration);
        }
        else
        {
            // Porteuse OFF
            radio.SetRx();

            delayMicroseconds(-duration);

            if (i + 1 < DATA_RAW_LENGTH)
            {
                radio.SetTx();
            }
        }
    }

    radio.SetRx();

    digitalWrite(LED_PIN, LOW);

    Serial.println("Data RAW termine.");
}

// --------------------------------------------------
// Emission OOK de TEST
// --------------------------------------------------

void sendTestSignal()
{
    Serial.println();
    Serial.println("Emission du signal TEST...");

    digitalWrite(LED_PIN, HIGH);

    /*
     * Ceci est volontairement une trame de test.
     *
     * Elle ne correspond PAS au contenu du fichier
     * RAW-ouverture-chambre.sub.
     *
     * Elle sert uniquement a verifier que le CC1101
     * peut passer correctement en emission.
     */

    uint8_t testData[] =
    {
        0xAA,
        0x55,
        0xAA,
        0x55
    };

    radio.SetTx();

    // Transmission d'un paquet de test
    radio.SendData(
        testData,
        sizeof(testData)
    );

    delay(50);

    radio.SetRx();

    digitalWrite(LED_PIN, LOW);

    Serial.println("Emission terminee.");
}

// --------------------------------------------------
// Boucle principale
// --------------------------------------------------

void loop()
{
    static bool previousButtonState = HIGH;

    bool buttonState = digitalRead(BUTTON_PIN);

    if (previousButtonState == HIGH &&
        buttonState == LOW)
    {
        sendTestSignal();
        sendRawData();
        delay(300);
    }

    previousButtonState = buttonState;
}