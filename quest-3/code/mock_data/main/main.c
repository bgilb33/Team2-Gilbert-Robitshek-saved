#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_wifi.h"

void app_main(void)
{
    while (1)
    {
        printf("1, 25.05678, 0\n");
        printf("2, 25.82947, 1\n");
        printf("3, 24.37947, 0\n");
        vTaskDelay(3000 / portTICK_PERIOD_MS);

        printf("1, 26.75689, 8\n");
        printf("2, 25.82947, 4\n");
        printf("3, 25.02684, 7\n");
        vTaskDelay(3000 / portTICK_PERIOD_MS);

        printf("1, 26.05678, 15\n");
        printf("2, 25.92947, 6\n");
        printf("3, 25.07947, 12\n");
        vTaskDelay(3000 / portTICK_PERIOD_MS);

        printf("1, 26.45674, 20\n");
        printf("2, 26.02674, 14\n");
        printf("3, 25.37947, 16\n");
        vTaskDelay(3000 / portTICK_PERIOD_MS);

        printf("1, 26.75838, 28\n");
        printf("2, 26.06374, 20\n");
        printf("3, 25.35636, 19\n");
        vTaskDelay(3000 / portTICK_PERIOD_MS);

        printf("1, 27.00478, 35\n");
        printf("2, 26.12352, 30\n");
        printf("3, 25.55367, 23\n");
        vTaskDelay(3000 / portTICK_PERIOD_MS);
    }
}
