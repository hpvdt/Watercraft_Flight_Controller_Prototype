
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "esp_timer.h"

#define TRIGGER_1 GPIO_NUM_4  // Sensor 1 Trigger
#define ECHO_1    GPIO_NUM_5  // Sensor 1 Echo

#define TRIGGER_2 GPIO_NUM_19 // Sensor 2 Trigger
#define ECHO_2    GPIO_NUM_18 // Sensor 2 Echo

static const char *TAG = "DUAL_AJ_SR04M";

void init_sensors() {
    // Configure Sensor 1
    gpio_set_direction(TRIGGER_1, GPIO_MODE_OUTPUT);
    gpio_set_direction(ECHO_1, GPIO_MODE_INPUT);

    // Configure Sensor 2
    gpio_set_direction(TRIGGER_2, GPIO_MODE_OUTPUT);
    gpio_set_direction(ECHO_2, GPIO_MODE_INPUT);
}

float measure_distance_cm(gpio_num_t trigger, gpio_num_t echo) {
    int64_t start_time, pulse_start, pulse_end;

    // Send a 10µs pulse to trigger measurement
    gpio_set_level(trigger, 1);
    esp_rom_delay_us(10);
    gpio_set_level(trigger, 0);

    // Wait for the echo pulse to go HIGH
    start_time = esp_timer_get_time();
    while (gpio_get_level(echo) == 0) {
        if (esp_timer_get_time() - start_time > 50000) { // 50ms timeout
            ESP_LOGE(TAG, "Sensor timeout (no echo detected) on GPIO %d", echo);
            return -1;
        }
    }
    pulse_start = esp_timer_get_time();

    // Wait for the echo pulse to go LOW
    while (gpio_get_level(echo) == 1);
    pulse_end = esp_timer_get_time();

    // Calculate distance: duration (us) / 58 = distance in cm
    float distance_cm = (pulse_end - pulse_start) / 58.0;
    return distance_cm;
}
