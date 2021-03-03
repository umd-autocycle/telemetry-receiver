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
}

void loop() {
    if (Serial.available()) {
        Serial.print("Sending: ");
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
                nbytes = 3;
                buffer[2] = Serial.parseInt();
                break;
        }
        buffer[1] = nbytes;
        buffer[31] = 0;
        Serial.println((char *)buffer);

        radio.stopListening();
        if(radio.write(buffer, 32)) {
            Serial.print("Sent: ");
            Serial.println((char *)buffer);
        }
        radio.startListening();
    }

    if (radio.available()) {
        delay(100);
        radio.read(buffer, 32);

        if (checksum(buffer, 30) == buffer[30]) {
            for (int i = 2; i < 30; i += 4) {
                Serial.print(*((float *) (&buffer[i])));
                Serial.print("\t");
            }

            if (buffer[0] == 13)
                Serial.println();
        }
    }

}

uint8_t checksum(const uint8_t *buf, int len) {
    unsigned int acc = 0;

    for (int i = 0; i < len; i++)
        acc += buf[i];

    return acc % 256;
}
