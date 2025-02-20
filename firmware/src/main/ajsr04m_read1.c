#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "esp_timer.h"

#define TRIGGER_PIN GPIO_NUM_4  // Trigger pin (sends pulse)
#define ECHO_PIN    GPIO_NUM_5  // Echo pin (receives pulse width)

static const char *TAG = "AJ-SR04M_PWM";

void init_sensor() {
    gpio_set_direction(TRIGGER_PIN, GPIO_MODE_OUTPUT);
    gpio_set_direction(ECHO_PIN, GPIO_MODE_INPUT);
}

float measure_distance_cm() {
    int64_t start_time, pulse_start, pulse_end;
    
    // Send a 10µs pulse to trigger measurement
    gpio_set_level(TRIGGER_PIN, 1);
    esp_rom_delay_us(10);
    gpio_set_level(TRIGGER_PIN, 0);

    // Wait for the echo pulse to go HIGH
    start_time = esp_timer_get_time();
    while (gpio_get_level(ECHO_PIN) == 0) {
        if (esp_timer_get_time() - start_time > 50000) { // 50ms timeout
            ESP_LOGE(TAG, "Sensor timeout (no echo detected)");
            return -1;
        }
    }
    pulse_start = esp_timer_get_time();

    // Wait for the echo pulse to go LOW
    while (gpio_get_level(ECHO_PIN) == 1);
    pulse_end = esp_timer_get_time();

    // Calculate distance: duration (us) / 58 = distance in cm
    float distance_cm = (pulse_end - pulse_start) / 58.0;
    return distance_cm;
}

void sensor_task(void *arg) {
    while (1) {
        float distance = measure_distance_cm();
        if (distance >= 0) {
            ESP_LOGI(TAG, "Distance: %.2f cm", distance);
        }
        vTaskDelay(pdMS_TO_TICKS(1000)); // Measure every 1 second
    }
}

void app_main(void) {
    init_sensor();
    xTaskCreate(sensor_task, "sensor_task", 2048, NULL, configMAX_PRIORITIES - 1, NULL);
}
