//
// Created by Misha on 12/10/2020.
//

#include <Arduino.h>

#include <nRF24L01.h>
#include <RF24.h>

#define N_CHANNELS  64
#define R           100
int num_interference[N_CHANNELS];

RF24 radio(7, 8);
uint8_t readAddr[] = "DNODE";
uint8_t writeAddr[] = "UNODE";
byte buffer[32];

uint8_t checksum(const uint8_t *buf, int len);

void setup() {
    Serial.begin(115200);
    while (!Serial);
    while (Serial.available()) Serial.read();

    radio.begin();
    radio.setAutoAck(false);
    radio.startListening();
    radio.stopListening();

    memset(num_interference, 0, sizeof num_interference);

    for (int j = 0; j < R; j++) {
        for (int i = 1; i <= N_CHANNELS; i++) {
            radio.setChannel(i);

            radio.startListening();
            delayMicroseconds(128);
            radio.stopListening();

            if (radio.testCarrier())
                num_interference[i - 1]++;
        }
    }

    for (int i = 1; i <= N_CHANNELS; i++) {
        Serial.print("Channel ");
        Serial.print(i);
        Serial.print(": ");
        Serial.print(num_interference[i - 1]);
        Serial.println(" interferences");
        delay(1);
    }
    Serial.println("Channel selection: ");

    while (!Serial.available());
    uint32_t channel = Serial.parseInt();
    radio.setChannel(channel);
    Serial.print("Connecting to channel ");
    Serial.println(channel);

    radio.setAutoAck(true);
    radio.openWritingPipe(writeAddr);
    radio.openReadingPipe(1, readAddr);
    radio.setPALevel(RF24_PA_HIGH);
    radio.startListening();
}

void loop() {
    if (Serial.available()) {
        Serial.println("Sending.");
        delay(10);
        int nbytes = 0;
        uint8_t command = Serial.read();

        buffer[0] = command;
        switch (command) {
            case 'v':
            case 'd':
            case 'h':
                nbytes = 6;
                *((float *) &(buffer[2])) = Serial.parseFloat();
                break;
            case 'c':
                nbytes = 4;
                *((uint16_t *) &(buffer[2])) = Serial.parseInt();
                break;
            case 't':
            case 'q':
                nbytes = 10;
                *((float *) &(buffer[2])) = Serial.parseFloat();
                *((uint32_t *) &(buffer[6])) = Serial.parseInt();
                break;
            default:
                break;
            case 'r':
            case 's':
                nbytes = 2;
                break;
        }
        buffer[1] = nbytes;
        buffer[31] = 0;
        Serial.println((char *) buffer);

        radio.stopListening();
        if (radio.write(buffer, 32)) {
            Serial.println("Sent.");
        }
        radio.startListening();
    }

    uint8_t pipenum;
    if (radio.available(&pipenum)) {
        if (pipenum == 1) {
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

}

uint8_t checksum(const uint8_t *buf, int len) {
    unsigned int acc = 0;

    for (int i = 0; i < len; i++)
        acc += buf[i];

    return acc % 256;
}
