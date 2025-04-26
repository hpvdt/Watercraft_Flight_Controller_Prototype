#include <Wire.h>
#include <math.h>
#include <stdio.h>

// Constants
#define SENSOR_COUNT 2
#define TIMEOUT 1000
#define MAX_RETRIES 3

const uint8_t SENSOR_ADDRESSES[SENSOR_COUNT] = {0x40, 0x41};
const float X_POS[SENSOR_COUNT] = {0.0f, 1.0f};
const float Y_POS[SENSOR_COUNT] = {0.0f, 0.0f};
const float Z_POS[SENSOR_COUNT] = {0.0f, 0.0f};

// Structs definitions
// Act as containers for global variables
typedef struct {
    float sensorReadings[SENSOR_COUNT];
    int sensorStatus[SENSOR_COUNT];
    int sensorsWorking;
} Payload;

typedef struct {
    float bisectorPosition[2];
    float normalVector[3];
    float pitch;
    float roll;
    float height;
} Output;

// Global instances of structs
Payload data = {
    .sensorReadings = {0.0f, 0.0f},
    .sensorStatus = {0, 0},
    .sensorsWorking = SENSOR_COUNT
};

Output result = {
    .bisectorPosition = {0.0f, 0.0f},
    .normalVector = {0.0f, 0.0f, 0.0f},
    .pitch = 0.0f,
    .roll = 0.0f,
    .height = 0.0f
};

void calculatePerpendicularBisector(Payload data, Output result) {
    float a = Payload::Z_POS[0] + data.sensorReadings[0];
    float b = Payload::Z_POS[1] + data.sensorReadings[1];
    float h = sqrtf((X_POS[1] - X_POS[0]) * (X_POS[1] - X_POS[0]) + (Z_POS[1] - Z_POS[0]) * (Z_POS[1] - Z_POS[0]));
    float theta = asinf((b - a) / h);

    result.bisectorPosition[0] = h / 2 * cosf(theta) - h / 2 * sinf(theta);
    result.bisectorPosition[1] = (b - a) / 2 + h / 2 * cosf(theta);
}

void calculate3DNormal(const float p1[3], const float p2[3], const float p3[3], float normal[3]) {
    // Take the xyz position of each sensor and calculate the normal to the plane
    // that they form together.

    float v1[3] = {p2[0] - p1[0], p2[1] - p1[1], p2[2] - p1[2]};
    float v2[3] = {p3[0] - p1[0], p3[1] - p1[1], p3[2] - p1[2]};

    normal[0] = (v1[1] * v2[2]) - (v1[2] * v2[1]);
    normal[1] = (v1[2] * v2[0]) - (v1[0] * v2[2]);
    normal[2] = (v1[0] * v2[1]) - (v1[1] * v2[0]);

}

float calculate_roll(Output result) {
    float length = sqrtf((X_POS[1] - X_POS[0]) * (X_POS[1] - X_POS[0]) + (Z_POS[1] - Z_POS[0]) * (Z_POS[1] - Z_POS[0]));
    acosf(result.bisectorPosition[0] / (length / 2));
}

float calculate_pitch(float length) {
    // Take the sensors from parallel corners (e.g., front left & back left)
    // and determine the pitch of the watercraft

    float height = fabs(Z_POS[0] - Z_POS[1]);
    return atan(height / length);
}

float calculate_height(Payload data) {
    // Take the sensors from opposite corners (e.g., front left & back right)
    // and determine the height of the center point of the watercraft

    return (Z_POS[0] + data.sensorReadings[0] + Z_POS[1] + data.sensorReadings[1]) / 2;
}

void setup() {
    Serial.begin(115200);
    Wire.begin();

    for (uint8_t i = 0; i < data.SENSOR_COUNT; i++) {
        handleSensor(i);
    }
}

void loop() {
    calculatePerpendicularBisector(data, result.bisectorPosition);
    result.roll = calculateRoll(result);
    result.pitch = calculate_pitch(result);
    result.height = calculate_height();
    

    float p1[3] = {X_POS[0], Y_POS[0], Z_POS[0] + data.sensorReadings[0]};
    float p2[3] = {X_POS[1], Y_POS[1], Z_POS[1] + data.sensorReadings[1]};
    float p3[3] = {0.0f, 1.0f, 0.0f}; // Example third point

    calculate3DNormal(p1, p2, p3, result.normalVector);
    delay(1000);
}
