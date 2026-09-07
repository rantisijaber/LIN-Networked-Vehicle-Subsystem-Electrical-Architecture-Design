#include "main.h"
#include "../include/lin_protocol.h"

extern UART_HandleTypeDef huart2;
extern TIM_HandleTypeDef  htim1;

static volatile uint8_t rx_buffer[1 + LIN_MAX_DATA_LEN + 1];
static volatile uint8_t frame_ready = 0;

static void LIN_Slave_ProcessFrame(void);
static void Motor_SetSpeed(uint8_t value_0_255);

static void LIN_Slave_ProcessFrame(void)
{
    uint8_t pid = rx_buffer[0];
    uint8_t id = pid & 0x3F;
    uint8_t *data = (uint8_t *)&rx_buffer[1];
    uint8_t checksum = rx_buffer[1 + LIN_MAX_DATA_LEN];

    if (!LIN_ValidateChecksum(pid, data, 1, checksum)) {
        return;
    }

    if (id == LIN_ID_SWITCH_STATE) {
        Motor_SetSpeed(data[0]);
    }
}

static void Motor_SetSpeed(uint8_t value_0_255)
{
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, value_0_255);
}

void LIN_Slave_OnBreakDetected(void)
{
    HAL_UART_Receive_IT(&huart2, (uint8_t *)rx_buffer, sizeof(rx_buffer));
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART2) {
        frame_ready = 1;
    }
}

/*
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
*/

/*
    while (1)
    {
        if (frame_ready)
        {
            frame_ready = 0;
            LIN_Slave_ProcessFrame();
        }
    }
*/
