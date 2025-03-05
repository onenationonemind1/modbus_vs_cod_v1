#include "main.h"
#include "uart.h"

#define RXBUFFERSIZE 30
// #define RXBUFFERSIZE                    1
#define test 10

extern UART_HandleTypeDef huart2;
extern UART_HandleTypeDef huart3;
extern UART_HandleTypeDef huart4;

extern DMA_HandleTypeDef hdma_usart2_rx;
extern DMA_HandleTypeDef hdma_usart3_rx;
extern DMA_HandleTypeDef hdma_usart4_rx;

uint8_t buffer_usart_1[RXBUFFERSIZE] = {
    0,
};
uint8_t buffer_usart_2[RXBUFFERSIZE] = {
    0,
};
uint8_t buffer_usart_3[RXBUFFERSIZE] = {
    0,
};
uint8_t buffer_usart_4[test] = {
    0,
};

uint8_t g_send_data[MAX_DATA];
uint8_t g_uart_04_index = 0;

int __io_putchar(int ch)
{
    HAL_UART_Transmit(&huart3, &ch, 1, 1000);
    return ch;
}

void uart_interrut_init(void)
{

    HAL_UART_Receive_IT(&huart2, buffer_usart_2, 1);
    HAL_UART_Receive_IT(&huart3, buffer_usart_3, 1);
    HAL_UART_Receive_IT(&huart4, buffer_usart_4, 1);

    // HAL_UART_Receive_DMA(&huart2, buffer_usart_2, 30);
    // HAL_UART_Receive_DMA(&huart3, buffer_usart_3, 1);
    // HAL_UART_Receive_DMA(&huart4, buffer_usart_4, test);
}

uint8_t arr[30];
uint8_t i = 0;
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART2)
    {
        sensor_parser(buffer_usart_2[0]);
        HAL_UART_Receive_IT(&huart2, buffer_usart_2, 1);
    }
    else if (huart->Instance == USART3)
    {
        HAL_UART_Receive_IT(&huart3, buffer_usart_3, 1);
    }
    else if (huart->Instance == USART4)
    {
        parser_modbus(buffer_usart_4[0]);
        HAL_UART_Receive_IT(&huart4, buffer_usart_4, 1);
    }
}

void send_packet_to_sensor(uint8_t *pdata, uint8_t length)
{
    uint8_t data;

    for (int i = 0; i < length; i++)
    {
        data = pdata[i];

        HAL_UART_Transmit(&huart2, &data, 1, 0xFFFF);
        while (__HAL_UART_GET_FLAG(&huart2, USART_ISR_BUSY) == SET)
            ;
    }
}