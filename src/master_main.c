#include "main.h"
#include "../include/lin_protocol.h"

extern UART_HandleTypeDef huart2;
extern TIM_HandleTypeDef  htim2;
extern ADC_HandleTypeDef  hadc1;

static volatile uint8_t send_frame_flag = 0;

static void LIN_Master_SendFrame(uint8_t id, const uint8_t *data, uint8_t len);

static void LIN_Master_SendFrame(uint8_t id, const uint8_t *data, uint8_t len)
{
    uint8_t sync = LIN_SYNC_BYTE;
    uint8_t pid = LIN_ComputePID(id);
    uint8_t checksum = LIN_ComputeChecksum(pid, data, len);

    HAL_LIN_SendBreak(&huart2);
    HAL_UART_Transmit(&huart2, &sync, 1, 10);
    HAL_UART_Transmit(&huart2, &pid, 1, 10);
    HAL_UART_Transmit(&huart2, (uint8_t *)data, len, 10);
    HAL_UART_Transmit(&huart2, &checksum, 1, 10);
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM2) {
        send_frame_flag = 1;
    }
}

/*
    HAL_TIM_Base_Start_IT(&htim2);
    HAL_ADC_Start(&hadc1);
*/

/*
    while (1)
    {
        if (send_frame_flag)
        {
            send_frame_flag = 0;

            HAL_ADC_PollForConversion(&hadc1, 10);
            uint16_t adc_raw = HAL_ADC_GetValue(&hadc1);
            uint8_t switch_data = (uint8_t)(adc_raw >> 4);

            LIN_Master_SendFrame(LIN_ID_SWITCH_STATE, &switch_data, 1);

            HAL_ADC_Start(&hadc1);
        }
    }
*/
