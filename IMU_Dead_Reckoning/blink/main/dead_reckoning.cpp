/* Blink Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "led_strip.h"
#include "sdkconfig.h"
#include <driver/i2c.h>
#include "MPU6050.h"
#include "I2Cdev.h"
#include "MPU6050_6Axis_MotionApps20.h"

#define PIN_SDA 21
#define PIN_CLK 22

Quaternion q;           // [w, x, y, z]         quaternion container
VectorFloat gravity;    // [x, y, z]            gravity vector
float ypr[3];           // [yaw, pitch, roll]   yaw/pitch/roll container and gravity vector
uint16_t packetSize = 42;    // expected DMP packet size (default is 42 bytes)
uint16_t fifoCount;     // count of all bytes currently in FIFO
uint8_t fifoBuffer[64]; // FIFO storage buffer
uint8_t mpuIntStatus;   // holds actual interrupt status byte from MPU
int16_t *a_x;
int16_t *a_y;
int16_t *a_z;
int16_t *g_r;
int16_t *g_p;
int16_t *g_y;



void task_initI2C(void *ignore) {
	i2c_config_t conf;
	conf.mode = I2C_MODE_MASTER;
	conf.sda_io_num = (gpio_num_t)PIN_SDA;
	conf.scl_io_num = (gpio_num_t)PIN_CLK;
	conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
	conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
	conf.master.clk_speed = 400000;
	ESP_ERROR_CHECK(i2c_param_config(I2C_NUM_0, &conf));
	ESP_ERROR_CHECK(i2c_driver_install(I2C_NUM_0, I2C_MODE_MASTER, 0, 0, 0));
    printf("Setup finished!\n");
	vTaskDelete(NULL);
}

void task_display(void *) {
	MPU6050 mpu = MPU6050();
	mpu.initialize();
	//mpu.initialize();
	/*
	mpu.initialize();
	mpu.dmpInitialize();
	mpu.CalibrateAccel(6); // from documentation, 6-7 loops (600-700 iterations) is fine
	mpu.CalibrateGyro(6); // from documentation 6-7 loops (600-700 iterations) is fine
	a_x = (int16_t *)malloc(sizeof(int16_t));
	a_y = (int16_t *)malloc(sizeof(int16_t));
	a_z = (int16_t *)malloc(sizeof(int16_t));

	g_r = (int16_t *)malloc(sizeof(int16_t));
	g_p = (int16_t *)malloc(sizeof(int16_t));
	g_y = (int16_t *)malloc(sizeof(int16_t));
	
	while(1) {
		
		mpuIntStatus = mpu.getIntStatus();
		// FIFO Setup - need FIFO as gyrosocope and accelerometer on desynced timers (gyro runs 1khz, accelerometer 8khz)
		if ((mpuIntStatus & 0x10) || fifoCount == 1024) {
			mpu.resetFIFO(); // resets FIFO register for new data
		}
		else if (mpuIntStatus & 0x02) {
			while (fifoCount < packetSize) {
				fifoCount = mpu.getFIFOCount();
			}
			mpu.getFIFOBytes(fifoBuffer, packetSize);
			//dead reckoning code goes here
		}
		
		mpu.getAcceleration(a_x, a_y, a_z);
		mpu.getRotation(g_p, g_r, g_y);
		printf("Accelerations: %d, %d, %d", (int)(*a_x), (int)(*a_y), (int)(*a_z));
		printf("Rotations: %d, %d, %d", (int)(*g_p), (int)(*g_r), (int)(*g_y));
	}
	
	mpu.setDMPEnabled(true);
	while(1){
		mpuIntStatus = mpu.getIntStatus();
		// get current FIFO count
		fifoCount = mpu.getFIFOCount();

		if ((mpuIntStatus & 0x10) || fifoCount == 1024) {
		// reset so we can continue cleanly
			mpu.resetFIFO();

	// otherwise, check for DMP data ready interrupt frequently)
		} else if (mpuIntStatus & 0x02) {
		// wait for correct available data length, should be a VERY short wait
			while (fifoCount < packetSize) fifoCount = mpu.getFIFOCount();

		// read a packet from FIFO

		
	}
	mpu.getFIFOBytes(fifoBuffer, packetSize);
	mpu.dmpGetQuaternion(&q, fifoBuffer);
	mpu.dmpGetGravity(&gravity, &q);
	mpu.dmpGetYawPitchRoll(ypr, &q, &gravity);
	printf("YAW: %3.1f, ", ypr[0] * 180/M_PI);
	printf("PITCH: %3.1f, ", ypr[1] * 180/M_PI);
	printf("ROLL: %3.1f \n", ypr[2] * 180/M_PI);
	//Best result is to match with DMP refresh rate
	// Its last value in components/MPU6050/MPU6050_6Axis_MotionApps20.h file line 310
	// Now its 0x13, which means DMP is refreshed with 10Hz rate
	// vTaskDelay(5/portTICK_PERIOD_MS);
}
*/
	
	vTaskDelete(NULL);
}
