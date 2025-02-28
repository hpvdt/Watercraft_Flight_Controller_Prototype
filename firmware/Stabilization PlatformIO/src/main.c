#include "Arduino.h"
#include <stdint.h>
#include <Wire.h>

#define SENSOR_COUNT 4
#define TIMEOUT 1000
#define MAX_RETRIES 3

const uint8_t SENSOR_ADDRESSES[SENSOR_COUNT] = {0x40, 0x41, 0x42, 0x43};
const float X_POS[SENSOR_COUNT] = {0, 0, 0, 0};
const float Y_POS[SENSOR_COUNT] = {0, 0, 0, 0};
const float Z_POS[SENSOR_COUNT] = {0, 0, 0, 0};

typedef struct {
    float sensorReadings[SENSOR_COUNT];
    bool sensorStatus[SENSOR_COUNT];
    int sensorsWorking;
} Payload;

Payload data = { {0}, {false}, SENSOR_COUNT };

bool readSensor(uint8_t address) {
    Wire.beginTransmission(address);
    uint8_t status = Wire.endTransmission();
    
    if (status != 0) return false;

    if (Wire.requestFrom(address, 2) != 2) return false;

    uint16_t sensorData = Wire.read() << 8 | Wire.read();
    return true;
}

void handleSensor(uint8_t index) {
    bool success = false;

    for (int attempt = 0; attempt < MAX_RETRIES; attempt++) {
        if (readSensor(SENSOR_ADDRESSES[index])) {
            success = true;
            break;
        }
    }
    data.sensorStatus[index] = success;
}

void calculate3DNormal(int p1, int p2, int p3, float sReadings[], float normal[3]) {
    float v1[3] = {
        X_POS[p2] - X_POS[p1], 
        Y_POS[p2] - Y_POS[p1], 
        (Z_POS[p2] + sReadings[p2]) - (Z_POS[p1] + sReadings[p1])
    };

    float v2[3] = {
        X_POS[p3] - X_POS[p2], 
        Y_POS[p3] - Y_POS[p2], 
        (Z_POS[p3] + sReadings[p3]) - (Z_POS[p2] + sReadings[p2])
    };

    normal[0] = (v1[1] * v2[2]) - (v1[2] * v2[1]);
    normal[1] = (v1[2] * v2[0]) - (v1[0] * v2[2]);
    normal[2] = (v1[0] * v2[1]) - (v1[1] * v2[0]);
}

void getTotalNormal(float sReadings[]) {
    if (data.sensorsWorking > 2) {
        for (int i = 0; i < data.sensorsWorking - 2; i++) {
            int points[3];
            int index = 0;
            for (int n = 0; n < SENSOR_COUNT; n++) {
                if (data.sensorStatus[n]) {
                    points[index++] = n;
                    if (index == 3) break;
                }
            }
            float normal[3];
            calculate3DNormal(points[0], points[1], points[2], sReadings, normal);
        }
    } else {
        Serial.println("ERROR! Sensor Failure.");
        while (1);
    }
}

void setup() {
    Serial.begin(9600);
    Wire.begin();

    for (uint8_t i = 0; i < SENSOR_COUNT; i++) {
        data.sensorStatus[i] = true;
        data.sensorReadings[i] = 0.0;
    }
}

void loop() {
    for (uint8_t i = 0; i < SENSOR_COUNT; i++) {
        bool success = false;
        for (int attempt = 0; attempt < MAX_RETRIES; attempt++) {
            if (readSensor(SENSOR_ADDRESSES[i])) {
                success = true;
                data.sensorReadings[i] = 0; // Replace with actual sensor reading
                break;
            }
        }
        if (!success) {
            Serial.println("ERROR! Sensor Failure.");
            while (1);
        }
    }
    delay(1000);
}
