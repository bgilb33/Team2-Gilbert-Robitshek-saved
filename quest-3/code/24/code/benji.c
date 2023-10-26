#include <stdio.h>
#include <string.h>
#include "driver/uart.h"
#include "esp_vfs_dev.h" // This is associated with VFS -- virtual file system interface and abstraction -- see the docs
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"

int period = 1000;
int blink_state = 0;

void init()
{

    // Setup based on code from Console I/O Example from the skill assignment
    /* Install UART driver for interrupt-driven reads and writes */
    ESP_ERROR_CHECK(uart_driver_install(UART_NUM_0,
                                        256, 0, 0, NULL, 0));

    /* Tell VFS to use UART driver */
    esp_vfs_dev_uart_use_driver(UART_NUM_0);
    gpio_reset_pin(13);
    gpio_set_direction(13, GPIO_MODE_OUTPUT);
}

void blink_task()
{
    while (1)
    {
        blink_state = ~blink_state;
        gpio_set_level(13, blink_state);
        vTaskDelay(period / portTICK_PERIOD_MS);
    }
}

void get_input_task()
{
    while (1)
    {
        printf("Input period: ");
        scanf("%i", &period);
        printf("\nPeriod set to %i ms\n", period);
        vTaskDelay(50 / portTICK_PERIOD_MS);
    }
}

void app_main(void)
{
    init();
    xTaskCreate(blink_task, "blink_task", 1024 * 2, NULL, configMAX_PRIORITIES, NULL);
    xTaskCreate(get_input_task, "get_input_task", 1024 * 2, NULL, configMAX_PRIORITIES, NULL);
}