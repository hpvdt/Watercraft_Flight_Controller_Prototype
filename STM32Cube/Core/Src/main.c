/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

// ============================================================
// PIN CONFIGURATION
// ============================================================
// Ultrasonic 1: TRIG=PA9, ECHO=PB6 (TIM4 CH1)
#define TRIG1_PORT     GPIOA
#define TRIG1_PIN      GPIO_PIN_9
#define ECHO1_PORT     GPIOB
#define ECHO1_PIN      GPIO_PIN_6

// Ultrasonic 2: TRIG=PA8, ECHO=PA7 (TIM3 CH2)
#define TRIG2_PORT     GPIOA
#define TRIG2_PIN      GPIO_PIN_8
#define ECHO2_PORT     GPIOA
#define ECHO2_PIN      GPIO_PIN_7

// Servo 1: PB10 (TIM2 CH3)
#define SERVO1_CHANNEL TIM_CHANNEL_3

// Servo 2: PB3 (TIM2 CH2)
#define SERVO2_CHANNEL TIM_CHANNEL_2

// ============================================================
// SERVO PULSE SETTINGS
// Clock=16MHz, Prescaler=15 -> 1MHz timer clock
// Period=19999 -> 50Hz PWM
// 1ms=1000 counts=0deg, 1.5ms=1500=90deg, 2ms=2000=180deg
// ============================================================
#define SERVO_PULSE_MIN  1000
#define SERVO_PULSE_MAX  2000

// ============================================================
// PID + SYSTEM SETTINGS
// ============================================================
#define TARGET_HEIGHT_1  30.0   // cm - change per system
#define TARGET_HEIGHT_2  30.0   // cm - change per system
#define SAMPLE_TIME      20     // ms

#define SERVO_DEFAULT    90
#define SERVO_MIN_ANGLE  0
#define SERVO_MAX_ANGLE  180

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim3;
TIM_HandleTypeDef htim4;

UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */
typedef struct {
    double Kp, Ki, Kd;
    double integral;
    double prev_error;
} PIDController;

PIDController pid1;
PIDController pid2;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_TIM2_Init(void);
static void MX_TIM3_Init(void);
static void MX_TIM4_Init(void);
static void MX_USART2_UART_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

// ============================================================
// MICROSECOND DELAY using DWT cycle counter
// ============================================================
void DWT_Init(void) {
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CYCCNT = 0;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
}

void delay_us(uint32_t us) {
    uint32_t start = DWT->CYCCNT;
    uint32_t ticks = us * (HAL_RCC_GetHCLKFreq() / 1000000);
    while ((DWT->CYCCNT - start) < ticks);
}

// ============================================================
// ULTRASONIC: read distance in cm
// ============================================================
double read_height(GPIO_TypeDef* trig_port, uint16_t trig_pin,
                   TIM_HandleTypeDef* echo_tim,
                   GPIO_TypeDef* echo_port, uint16_t echo_pin) {

    // Send 10us trigger pulse
    HAL_GPIO_WritePin(trig_port, trig_pin, GPIO_PIN_RESET);
    delay_us(2);
    HAL_GPIO_WritePin(trig_port, trig_pin, GPIO_PIN_SET);
    delay_us(10);
    HAL_GPIO_WritePin(trig_port, trig_pin, GPIO_PIN_RESET);

    // Reset timer counter
    __HAL_TIM_SET_COUNTER(echo_tim, 0);

    // Wait for ECHO HIGH with timeout
    uint32_t timeout = HAL_GetTick() + 30;
    while (HAL_GPIO_ReadPin(echo_port, echo_pin) == GPIO_PIN_RESET) {
        if (HAL_GetTick() > timeout) return -1.0;
    }

    // Start counting
    HAL_TIM_Base_Start(echo_tim);

    // Wait for ECHO LOW with timeout
    timeout = HAL_GetTick() + 30;
    while (HAL_GPIO_ReadPin(echo_port, echo_pin) == GPIO_PIN_SET) {
        if (HAL_GetTick() > timeout) {
            HAL_TIM_Base_Stop(echo_tim);
            return -1.0;
        }
    }

    // Stop and read
    HAL_TIM_Base_Stop(echo_tim);
    uint32_t count = __HAL_TIM_GET_COUNTER(echo_tim);

    // Each count = 1us (1MHz timer), distance in cm
    return (count * 0.0343) / 2.0;
}

// ============================================================
// SERVO: write angle as PWM pulse
// ============================================================
void servo_write(TIM_HandleTypeDef* tim, uint32_t channel, int angle) {
    if (angle < SERVO_MIN_ANGLE) angle = SERVO_MIN_ANGLE;
    if (angle > SERVO_MAX_ANGLE) angle = SERVO_MAX_ANGLE;
    uint32_t pulse = SERVO_PULSE_MIN +
        ((angle * (SERVO_PULSE_MAX - SERVO_PULSE_MIN)) / 180);
    __HAL_TIM_SET_COMPARE(tim, channel, pulse);
}

// ============================================================
// PID
// ============================================================
void initialize_PID(PIDController* pid, double Kp, double Ki, double Kd) {
    pid->Kp = Kp;
    pid->Ki = Ki;
    pid->Kd = Kd;
    pid->integral = 0;
    pid->prev_error = 0;
}

double compute_PID(PIDController* pid, double target, double current, double dt) {
    double error = target - current;
    double P = pid->Kp * error;

    double raw_output = P + pid->Ki * pid->integral
                          + pid->Kd * (error - pid->prev_error) / dt;
    double final_angle = SERVO_DEFAULT + raw_output;

    bool is_saturated = (final_angle > SERVO_MAX_ANGLE) ||
                        (final_angle < SERVO_MIN_ANGLE);
    bool same_sign = (error > 0) == (raw_output > 0);

    if (!(is_saturated && same_sign)) {
        pid->integral += error * dt;
    }

    double I = pid->Ki * pid->integral;
    double D = pid->Kd * (error - pid->prev_error) / dt;
    pid->prev_error = error;

    return P + I + D;
}

// ============================================================
// DEBUG UART
// ============================================================
void debug_print(const char* msg) {
    HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  MX_GPIO_Init();
  MX_TIM2_Init();
  MX_TIM3_Init();
  MX_TIM4_Init();
  MX_USART2_UART_Init();

  /* USER CODE BEGIN 2 */

  DWT_Init();

  initialize_PID(&pid1, 5.0, 0.1, 0.5);
  initialize_PID(&pid2, 5.0, 0.1, 0.5);

  // Start PWM on both servo channels
  HAL_TIM_PWM_Start(&htim2, SERVO1_CHANNEL);  // PB10 TIM2 CH3
  HAL_TIM_PWM_Start(&htim2, SERVO2_CHANNEL);  // PB3  TIM2 CH2

  // Move both servos to 90 degrees default
  servo_write(&htim2, SERVO1_CHANNEL, SERVO_DEFAULT);
  servo_write(&htim2, SERVO2_CHANNEL, SERVO_DEFAULT);

  HAL_Delay(1000);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  uint32_t last_pid_time = 0;

  while (1)
  {
    uint32_t current_time = HAL_GetTick();

    if (current_time - last_pid_time >= SAMPLE_TIME) {
        double dt = (current_time - last_pid_time) / 1000.0;
        last_pid_time = current_time;

        // --- Sensor 1 (PA9 trig, PB6 echo) + Servo 1 (PB10) ---
        double height1 = read_height(
            TRIG1_PORT, TRIG1_PIN,
            &htim4,
            ECHO1_PORT, ECHO1_PIN
        );

        if (height1 >= 0 && height1 <= 100) {
            double correction1 = compute_PID(&pid1, TARGET_HEIGHT_1, height1, dt);
            double angle1 = SERVO_DEFAULT + correction1;
            if (angle1 < SERVO_MIN_ANGLE) angle1 = SERVO_MIN_ANGLE;
            if (angle1 > SERVO_MAX_ANGLE) angle1 = SERVO_MAX_ANGLE;
            servo_write(&htim2, SERVO1_CHANNEL, (int)angle1);

            char buf[80];
            snprintf(buf, sizeof(buf),
                "S1 | Target:%.1f Current:%.2f Angle:%.0f\r\n",
                TARGET_HEIGHT_1, height1, angle1);
            debug_print(buf);
        }

        // --- Sensor 2 (PA8 trig, PA7 echo) + Servo 2 (PB3) ---
        double height2 = read_height(
            TRIG2_PORT, TRIG2_PIN,
            &htim3,
            ECHO2_PORT, ECHO2_PIN
        );

        if (height2 >= 0 && height2 <= 100) {
            double correction2 = compute_PID(&pid2, TARGET_HEIGHT_2, height2, dt);
            double angle2 = SERVO_DEFAULT + correction2;
            if (angle2 < SERVO_MIN_ANGLE) angle2 = SERVO_MIN_ANGLE;
            if (angle2 > SERVO_MAX_ANGLE) angle2 = SERVO_MAX_ANGLE;
            servo_write(&htim2, SERVO2_CHANNEL, (int)angle2);

            char buf[80];
            snprintf(buf, sizeof(buf),
                "S2 | Target:%.1f Current:%.2f Angle:%.0f\r\n",
                TARGET_HEIGHT_2, height2, angle2);
            debug_print(buf);
        }
    }

    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief TIM2 Initialization Function
  */
static void MX_TIM2_Init(void)
{
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 15;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 19999;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_PWM_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_2) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_3) != HAL_OK)
  {
    Error_Handler();
  }
  HAL_TIM_MspPostInit(&htim2);
}

/**
  * @brief TIM3 Initialization Function
  */
static void MX_TIM3_Init(void)
{
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_IC_InitTypeDef sConfigIC = {0};

  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 15;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 65535;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_IC_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigIC.ICPolarity = TIM_INPUTCHANNELPOLARITY_RISING;
  sConfigIC.ICSelection = TIM_ICSELECTION_DIRECTTI;
  sConfigIC.ICPrescaler = TIM_ICPSC_DIV1;
  sConfigIC.ICFilter = 0;
  if (HAL_TIM_IC_ConfigChannel(&htim3, &sConfigIC, TIM_CHANNEL_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief TIM4 Initialization Function
  */
static void MX_TIM4_Init(void)
{
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_IC_InitTypeDef sConfigIC = {0};

  htim4.Instance = TIM4;
  htim4.Init.Prescaler = 15;
  htim4.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim4.Init.Period = 65535;
  htim4.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim4.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_IC_Init(&htim4) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim4, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigIC.ICPolarity = TIM_INPUTCHANNELPOLARITY_RISING;
  sConfigIC.ICSelection = TIM_ICSELECTION_DIRECTTI;
  sConfigIC.ICPrescaler = TIM_ICPSC_DIV1;
  sConfigIC.ICFilter = 0;
  if (HAL_TIM_IC_ConfigChannel(&htim4, &sConfigIC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief USART2 Initialization Function
  */
static void MX_USART2_UART_Init(void)
{
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief GPIO Initialization Function
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8|GPIO_PIN_9, GPIO_PIN_RESET);

  GPIO_InitStruct.Pin = GPIO_PIN_8|GPIO_PIN_9;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef USE_FULL_ASSERT
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
