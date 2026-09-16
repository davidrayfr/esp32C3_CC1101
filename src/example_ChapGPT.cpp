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

        delay(300);
    }

    previousButtonState = buttonState;
}