#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include "ajsr04m_read2.h"
#include "stabilization.h"
#include "PID_Controler.h"

bool run = true;

/*

============================================================
---------------------Struct-Definitions---------------------
============================================================

Act as containers for 'global' variables. 

- Payload contains offloaded data from the sensors.
- Output contains the results of transformations on the data from payload.

*/


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

// Instances of structs
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

// Renamed sensor_task to main_loop
void main_loop(void *arg) { 
    while (run) {
        float distance1 = measure_distance_cm(TRIGGER_1, ECHO_1);
        float distance2 = measure_distance_cm(TRIGGER_2, ECHO_2);
        //ESP_LOGI(TAG, "D1: %.2f cm, D2: %.2f", distance1, distance2);
        calculatePerpendicularBisector(data, result.bisectorPosition);
        result.roll = calculateRoll(result);
        result.pitch = calculate_pitch(result);
        result.height = calculate_height();
    

        float p1[3] = {X_POS[0], Y_POS[0], Z_POS[0] + data.sensorReadings[0]};
        float p2[3] = {X_POS[1], Y_POS[1], Z_POS[1] + data.sensorReadings[1]};
        float p3[3] = {0.0f, 1.0f, 0.0f}; // Example third point

        calculate3DNormal(p1, p2, p3, result.normalVector);
    
        vTaskDelay(pdMS_TO_TICKS(100)); // Measure every 1 second
    }
}

void app_main(void) {
    init_sensors();
    xTaskCreate(main_loop, "main_loop", 4096, NULL, configMAX_PRIORITIES - 1, NULL);
}

