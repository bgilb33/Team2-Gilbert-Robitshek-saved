#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/uart.h"
#include "lwip/inet.h"

// UART definitions
#define UART_TX_GPIO_NUM 26 // A0
#define BUF_SIZE (1024)

// Default IP address
#define IP_ADDRESS "192.168.1.100"

// Variables for my IP address
ip4_addr_t myIP;

// Start byte
char start = 0x1B;

// Mutex for current values when sending
SemaphoreHandle_t mux = NULL;

// Configure UART
static void uart_init()
{
    const uart_config_t uart_config = {
        .baud_rate = 1200, // Slow BAUD rate
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT};
    uart_param_config(UART_NUM_1, &uart_config);

    uart_set_pin(UART_NUM_1, UART_TX_GPIO_NUM, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);

    uart_driver_install(UART_NUM_1, BUF_SIZE * 2, 0, 0, NULL, 0);
}

// Send task -- sends payload | Start | myIP
void send_task()
{
    while (1)
    {
        char data_out[5]; // Assuming IPv4 address (xxx.xxx.xxx.xxx)
        xSemaphoreTake(mux, portMAX_DELAY);

        // Convert IP address string to ip4_addr_t
        if (ip4addr_aton(IP_ADDRESS, &myIP))
        {
            data_out[0] = start;
            memcpy(data_out + 1, &myIP, sizeof(ip4_addr_t));
            uart_write_bytes(UART_NUM_1, data_out, sizeof(ip4_addr_t) + 1);
        }
        else
        {
            ESP_LOGE("send_task", "Invalid IP address");
        }

        xSemaphoreGive(mux);

        vTaskDelay(5 / portTICK_PERIOD_MS);
    }
}

// CREATE A TASK FOR RECEIVING STATUS FROM AUTH SERVER AND LIGHT UP CORRECT LEDs
// DON'T SEND DATA IF ALREADY PARKED

void app_main()
{
    mux = xSemaphoreCreateMutex();
    uart_init();

    xTaskCreate(send_task, "uart_tx_task", 1024 * 2, NULL, configMAX_PRIORITIES, NULL);

    while (1)
    {
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}
