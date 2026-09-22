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

    481, -494, 303, -552, 321, -568, 289, -562, 301, -554, 337, -526, 353, -536, 327, -520, 343, -520, 353, -510, 387, -476, 373, -4388, 407, -910, 833, -444, 851, -448, 415, -892, 879, -412, 871, -444, 879, -436, 417, -884, 853, -454, 429, -872, 417, -896, 845, -450, 441, -846, 887, -418, 879, -448, 413, -884, 417, -882, 427, -874, 853, -454, 415, -876, 877, -452, 435, -846, 423, -880, 425, -880, 423, -884, 883, -418, 877, -446, 413, -886, 419, -882, 429, -872, 409, -888, 451, -848, 879, -418, 431, -874, 459, -846, 457, -870, 861, -418, 453, -856, 881, -448, 843, -444, 875, -438, 417, -872, 869, -460, 409, -884, 853, -448, 869, -426, 879, -424, 441, -886, 855, -450, 429, -872, 869, -418, 445, -886, 417, -880, 445, -842, 869, -452, 425, -882, 881, -418, 877, -446, 841, -442, 875, -438, 875, -436, 415, -886, 885, -414, 871, -424, 901, -430, 877, -17328, 467, -420, 449, -408, 447, -422, 441, -452, 425, -426, 445, -416, 471, -420, 449, -418, 447, -416, 447, -420, 441, -452, 421, -4354, 433, -844, 891, -422, 879, -450, 413, -862, 903, -412, 875, -440, 873, -438, 415, -888, 885, -416, 441, -852, 435, -870, 877, -424, 445, -880, 877, -420, 895, -422, 449, -856, 415, -872, 441, -888, 857, -414, 469, -838, 905, -408, 447, -882, 421, -878, 447, -840, 441, -876, 865, -422, 883, -450, 413, -888, 415, -876, 453, -850, 435, -866, 449, -880, 877, -410, 447, -876, 445, -842, 441, -874, 863, -456, 421, -852, 909, -420, 877, -416, 905, -410, 445, -856, 881, -422, 439, -888, 853, -416, 891, -450, 887, -416, 439, -844, 899, -418, 441, -884, 857, -414, 467, -874, 421, -860, 451, -876, 873, -420, 465, -846, 883, -420, 877, -416, 905, -412, 875, -440, 873, -438, 417, -872, 899, -432, 875, -432, 879, -398, 451, -17794, 467, -420, 445, -428, 441, -446, 409, -446, 447, -430, 439, -414, 443, -442, 445, -410, 443, -448, 409, -446, 447, -428, 441, -4346, 447, -846, 877, -440, 873, -440, 427, -870, 865, -450, 879, -418, 879, -448, 413, -886, 875, -434, 417, -882, 427, -874, 865, -446, 439, -874, 871, -418, 881, -424, 443, -880, 445, -842, 441, -876, 863, -454, 423, -884, 879, -420, 447, -868, 419, -882, 421, -882, 419, -870, 887, -420, 881, -448, 415, -886, 415, -886, 425, -876, 453, -870, 415, -878, 873, -454, 433, -868, 429, -872, 431, -870, 863, -454, 421, -886, 877, -418, 877, -446, 873, -412, 445, -854, 879, -426, 441, -882, 869, -424, 899, -434, 877, -432, 417, -874, 867, -426, 443, -884, 885, -416, 441, -884, 431, -874, 419, -860, 883, -452, 439, -846, 891, -418, 875, -450, 871, -416, 907, -410, 877, -438, 415, -886, 883, -420, 885, -422, 899, -426
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
    //radio.setPA(-10);
    radio.setPA(12);

    Serial.println();
    Serial.println("Configuration radio :");
    Serial.print("Frequence : ");
    Serial.print(radio.getMHZ());
    Serial.println(" MHz");

    Serial.println("Modulation : OOK");
    Serial.println("Puissance : 12 dBm");

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