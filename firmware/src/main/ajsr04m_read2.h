#ifndef AJSR04M_READ2_H
#define AJSR04M_READ2_H

#include "driver/gpio.h"

#define TRIGGER_1 GPIO_NUM_4  // Sensor 1 Trigger
#define ECHO_1    GPIO_NUM_5  // Sensor 1 Echo

#define TRIGGER_2 GPIO_NUM_19 // Sensor 2 Trigger
#define ECHO_2    GPIO_NUM_18 // Sensor 2 Echo

extern void init_sensors();

extern float measure_distance_cm(gpio_num_t trigger, gpio_num_t echo);

extern void sensor_task(void *arg);

#endif
