#include <Wire.h>
#include <Arduino.h>
#include <cmath>

// const double width = ??;
// const double length = ??;

class Payload {

    public:
    static constexpr int SENSOR_COUNT = 2;
    static constexpr float TIMEOUT = 1000;
    static constexpr int MAX_RETRIES = 3;

    static constexpr uint8_t SENSOR_ADDRESSES[SENSOR_COUNT] = {0x40, 0x41};
    static constexpr float X_POS[SENSOR_COUNT] = {0.0, 1.0};
    static constexpr float Y_POS[SENSOR_COUNT] = {0.0, 0.0};
    static constexpr float Z_POS[SENSOR_COUNT] = {0.0, 0.0};

    float sensorReadings[SENSOR_COUNT] = {0.0, 0.0};
    bool sensorStatus[SENSOR_COUNT] = {false};
    int sensorsWorking = SENSOR_COUNT;
};

struct Output {
    float bisectorPosition[2] = {0.0, 0.0};
    float normalVector[3] = {0.0,0.0,0.0};
    float pitch = 0.0;
    float roll = 0.0;
};

Payload data;
Output result;

bool readSensor(uint8_t address, float &reading) {
    Wire.beginTransmission(address);
    if (Wire.endTransmission() != 0) return false;

    if (Wire.requestFrom(address, (uint8_t)2) != 2) return false;

    uint16_t rawData = Wire.read() << 8 | Wire.read();
    reading = static_cast<float>(rawData) / 100.0; // Example conversion
    return true;
}

void handleSensor(uint8_t index) {
    float reading = 0.0;
    bool success = false;

    for (int attempt = 0; attempt < data.MAX_RETRIES; attempt++) {
        if (readSensor(Payload::SENSOR_ADDRESSES[index], reading)) {
            success = true;
            data.sensorReadings[index] = reading;
            break;
        }
    }

    data.sensorStatus[index] = success;
    if (!success) {
        data.sensorsWorking--;
        Serial.printf("Sensor %d failed!\n", index);
    }
}

void calculatePerpendicularBisector(float endPoint[2]) {
    float a = Payload::Z_POS[0] + data.sensorReadings[0];
    float b = Payload::Z_POS[1] + data.sensorReadings[1];
    float h = sqrt(pow(Payload::X_POS[1] - Payload::X_POS[0], 2) + pow(Payload::Z_POS[1] - Payload::Z_POS[0], 2));
    float theta = asin((b - a) / h);

    endPoint[0] = h / 2 * cos(theta) - h / 2 * sin(theta);
    endPoint[1] = (b - a) / 2 + h / 2 * cos(theta);
}

void calculate3DNormal(const float p1[3], const float p2[3], const float p3[3]) {
    '''Take the xyz position of each sensor and calculate the normal to the plane
       that they form together.'''

    float v1[3] = {p2[0] - p1[0], p2[1] - p1[1], p2[2] - p1[2]};
    float v2[3] = {p3[0] - p1[0], p3[1] - p1[1], p3[2] - p1[2]};
    normal[3] = {0,0,0};

    normal[0] = (v1[1] * v2[2]) - (v1[2] * v2[1]);
    normal[1] = (v1[2] * v2[0]) - (v1[0] * v2[2]);
    normal[2] = (v1[0] * v2[1]) - (v1[1] * v2[0]);

    return normal;
}

float calculateRoll(const Output &result) {
    float length = sqrt(pow(Payload::X_POS[1] - Payload::X_POS[0], 2) + pow(Payload::Z_POS[1] - Payload::Z_POS[0], 2));
    return acos(result.bisectorPosition[0] / (length / 2));
}

double calculate_height() {
    '''Take the sensors from opposite corners (e.g., front left & back right)
       and determine the height of the center point of the watercraft'''

    return (data.Z_POS[0] + data.Z_POS[1]) / 2;
}

double calculate_pitch(double length) {
    '''Take the sensors from parallel corners (e.g., front left & back left)
       and determine the pitch of the watercraft'''

    double height = fabs(data.Z_POS[0] - data.Z_POS[1]);
    double pitch = tan(height / length); 

    return pitch;
}

void setup() {
    Serial.begin(115200);
    Wire.begin();

    for (uint8_t i = 0; i < data.SENSOR_COUNT; i++) {
        handleSensor(i);
    }
}

void loop() {
    calculatePerpendicularBisector(result.bisectorPosition);
    result.roll = calculateRoll(result);
    delay(1000);
}
