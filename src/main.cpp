//
// Created by Misha on 12/10/2020.
//

#include <Arduino.h>

#include <nRF24L01.h>
#include <RF24.h>

RF24 radio(7, 8);
uint8_t readAddr[] = "DNODE";
uint8_t writeAddr[] = "UNODE";
byte buffer[32];

void setup() {
    Serial.begin(115200);

    radio.begin();
    radio.openWritingPipe(writeAddr);
    radio.openReadingPipe(1, readAddr);
    radio.setPALevel(RF24_PA_MIN);
    radio.startListening();
}

void loop() {
    if (Serial.available()) {
        delay(10);
        int nbytes = min(sizeof buffer, Serial.available());
        Serial.readBytes(buffer, nbytes);

        radio.stopListening();
        radio.write(buffer, nbytes);
        radio.startListening();
    }

    if (radio.available()) {
        delay(10);
        radio.read(buffer, 32);

        Serial.write(buffer, 32);
    }

}