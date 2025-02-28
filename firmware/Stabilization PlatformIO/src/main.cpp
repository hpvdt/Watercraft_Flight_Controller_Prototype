#include "Arduino.h"
#include <stdint.h>

struct Payload {
    static const int SENSOR_COUNT = 4;
    static const int TIMEOUT = 1000;     
    static const int MAX_RETRIES = 3;    

    static const uint8_t SENSOR_ADDRESSES[SENSOR_COUNT];
    float sensorReadings[SENSOR_COUNT] = {0}; 
    bool sensorStatus[SENSOR_COUNT] = {false}; 
    int sensorsWorking = SENSOR_COUNT;

    static const float X_POS[SENSOR_COUNT];
    static const float Y_POS[SENSOR_COUNT];
    static const float Z_POS[SENSOR_COUNT];
};

// Definition of static arrays
const uint8_t Payload::SENSOR_ADDRESSES[Payload::SENSOR_COUNT] = {0x40, 0x41, 0x42, 0x43}; 
const float Payload::X_POS[Payload::SENSOR_COUNT] = {0, 0, 0, 0};
const float Payload::Y_POS[Payload::SENSOR_COUNT] = {0, 0, 0, 0};
const float Payload::Z_POS[Payload::SENSOR_COUNT] = {0, 0, 0, 0};

Payload data;

bool readSensor(uint8_t address) {
    Wire.beginTransmission(address);
    uint8_t status = Wire.endTransmission();
    
    if (status != 0) return false;

    if (Wire.requestFrom(address, 2) != 2) return false;

    uint16_t data = Wire.read() << 8 | Wire.read();
    return true;
}

bool readFromCSV() {

}

void handleSensor(uint8_t index) {
    bool success = false;

    for (int attempt = 0; attempt < MAX_RETRIES; attempt++) {
        if (readSensor(SENSOR_ADDRESSES[index])) {
            success = true;
            break;
        }
    }

    sensorStatus[index] = success;
}

std::array<float, 3> calculate3DNormal(int p1, int p2, int p3, float sReadings[]) {
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

    return { 
        (v1[1] * v2[2]) - (v1[2] * v2[1]), 
        (v1[2] * v2[0]) - (v1[0] * v2[2]), 
        (v1[0] * v2[1]) - (v1[1] * v2[0])
    };
}

void getTotalNormal(float sReadings[], float X_POS[], float Y_POS[], float Z_POS[]) {
    if (sensorsWorking > 2) {
        for (int i = 0; i < sensorsWorking - 2; i++) {
            int points[3];
            int index = 0;
            for (int n = 0; n < SENSOR_COUNT; n++) {
                if (sensorStatus[n]) {
                    points[index++] = n;
                    if (index == 3) break;
                }
            }
            auto normal = calculate3DNormal(points[0], points[1], points[2], sReadings);
        }
    } else {
        Serial.println("ERROR! Sensor Failure.");
        while (true);
    }
}

void setup() {
    Serial.begin(9600);
    Wire.begin();

    for (uint8_t i = 0; i < SENSOR_COUNT; i++) {
        sensorStatus[i] = true;
        sensorReadings[i] = 0.0;
    }
}

void loop() {
    for (uint8_t i = 0; i < SENSOR_COUNT; i++) {
        bool success = false;
        for (int attempt = 0; attempt < MAX_RETRIES; attempt++) {
            if (readSensor(SENSOR_ADDRESSES[i])) {
                success = true;
                sensorReadings[i] = 0; // Replace with actual sensor reading
                break;
            }
        }

        if (!success) {
            Serial.println("ERROR! Sensor Failure.");
            while (true);
        }
    }

    delay(1000);
}
