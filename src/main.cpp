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

uint8_t checksum(const uint8_t *buf, int len);

void setup() {
    Serial.begin(115200);

    radio.begin();
    radio.openWritingPipe(writeAddr);
    radio.openReadingPipe(1, readAddr);
    radio.setPALevel(RF24_PA_HIGH);
    radio.startListening();

    while (Serial.available()) Serial.read();
}

void loop() {
    if (Serial.available()) {
        Serial.println("Sending.");
        delay(10);
        int nbytes = 0;
        uint8_t command = Serial.read();

        buffer[0] = command;
        switch (command) {
            case 's':
            case 'd':
                nbytes = 6;
                *((float*) &(buffer[2])) = Serial.parseFloat();
                break;
            case 'c':
                nbytes = 4;
                *((uint16_t *) &(buffer[2])) = Serial.parseInt();
                break;
            case 't':
                nbytes = 10;
                *((float*) &(buffer[2])) = Serial.parseFloat();
                *((uint32_t *) &(buffer[6])) = Serial.parseInt();
                break;
            default:
                break;
            case 'h':
            case 'r':
            case 'q':
                nbytes = 2;
                break;
        }
        buffer[1] = nbytes;
        buffer[31] = 0;
        Serial.println((char *)buffer);

        radio.stopListening();
        if(radio.write(buffer, 32)) {
            Serial.println("Sent.");
        }
        radio.startListening();
    }

    if (radio.available()) {
        delay(100);
        radio.read(buffer, 32);

        if (checksum(buffer, 30) == buffer[30]) {
            if (buffer[0] == 13)
                Serial.println();

            for (int i = 2; i < 30; i += 4) {
                Serial.print(*((float *) (&buffer[i])));
                Serial.print("\t");
            }


        }
    }

}

uint8_t checksum(const uint8_t *buf, int len) {
    unsigned int acc = 0;

    for (int i = 0; i < len; i++)
        acc += buf[i];

    return acc % 256;
}
