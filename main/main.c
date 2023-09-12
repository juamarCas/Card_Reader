/**
 * @author Juan D. Martín
 * @details NFC reader host controller for PN532. 
*/

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "usart_device.h"
#include "stdint.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"
#include "PN532.h"

#define UART2_NUM 2
#define UART2_TX_PIN 17
#define UART2_RX_PIN 16

USART_DEVICE usart_dev;

void Write(uint8_t * data, uint32_t size);
uint32_t Read(uint8_t * data);
void Config_UART2();


void app_main(void)
{
    usart_dev.Write = &Write;
    usart_dev.Read  = &Read;

    gpio_set_direction(GPIO_NUM_4, GPIO_MODE_OUTPUT);

    Config_UART2();
     
    pn532_SendWakeUpCommand(&usart_dev);
    vTaskDelay(15 / portTICK_PERIOD_MS);

    uint8_t response = pn532_SendSAMConfiguration(&usart_dev);
    vTaskDelay(100 / portTICK_PERIOD_MS);



    if(response <= 0){
        //some error with the reader
        while(1){
            gpio_set_level(GPIO_NUM_4, 1);
            vTaskDelay(500 / portTICK_PERIOD_MS);
            gpio_set_level(GPIO_NUM_4, 0);
            vTaskDelay(500 / portTICK_PERIOD_MS);
        }
    }

    vTaskDelay(15 / portTICK_PERIOD_MS);

    pn532_ConfigRF(&usart_dev, PN532_RF_CONF_TWO_TRIES);;

    
   
    
    while(1){

        if(pn532_DetectCard(&usart_dev, PN532_ONE_CARD)){
            uint8_t * uid = pn532_GetUID();
            uint8_t keyA[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
            if(pn532_mifare_authenticate_key_a(&usart_dev, 1, keyA, uid)){
                pn532_mifare_write_16(&usart_dev, keyA, 3, 1);
                gpio_set_level(GPIO_NUM_4, 1);

            }
        }
        
        vTaskDelay(500 / portTICK_PERIOD_MS);
    }
    
}

void Config_UART2(){
    //by default the PN532 with communicates with 115200 bps for usart
    uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
    };

    ESP_ERROR_CHECK(uart_param_config(UART2_NUM, &uart_config));
    ESP_ERROR_CHECK(uart_set_pin(UART2_NUM, UART2_TX_PIN, UART2_RX_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));

    const uint32_t uart_buffer_size = (1024 * 2);
    QueueHandle_t uart_queue;

    ESP_ERROR_CHECK(uart_driver_install(UART2_NUM, uart_buffer_size, \
                                        uart_buffer_size, 10, &uart_queue, 0));
}

void Write(uint8_t * data, uint32_t size){
    uart_write_bytes(UART2_NUM, (const char *)data, size);
}

uint32_t Read(uint8_t * data){
    uint8_t length_check = 0;
    while(length_check == 0){
        ESP_ERROR_CHECK(uart_get_buffered_data_len(UART2_NUM, (size_t *)&length_check));
        length_check = uart_read_bytes(UART2_NUM, data, length_check, 100);

    }
    uart_flush(UART2_NUM);
    return length_check;
}
