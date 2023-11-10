#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/adc.h"

#define DEFAULT_VREF    1100 // Default reference voltage (in mV) for ADC
#define ADC_UNIT        ADC_UNIT_1 // ADC unit
#define ADC_CHANNEL     ADC1_CHANNEL_0
#define ADC_WIDTH       ADC_WIDTH_BIT_12 // ADC bit width
#define ADC_ATTEN       ADC_ATTEN_DB_11 // ADC attenuation


int count = 0;
int previousColor = 0;

int returnColor(uint16_t adc_value){
    int color;
    if(adc_value < 300){color = 1;}
    else{color = 0;}

    if(previousColor == 0 && color){count++;}
    previousColor = color;
    return color;
}

void timerTask(void *pvParameters) {
    int startCount = 0; // Initialize startCount
    
    while (1) {
        startCount = count;
        vTaskDelay(pdMS_TO_TICKS(1000)); // Delay the task for 1000 milliseconds

        // Calculate the number of negative edges in 1 second
        float difference = count - startCount;

        // negative edges / second
        float nedEdges = difference / 1;
        float rotationPerSec = nedEdges * (1.0/6.0);
        float inchesPerSecond = rotationPerSec * (7);

        printf("The count : %i\n", count);
        vTaskDelay(pdMS_TO_TICKS(10)); // Delay the task for 1000 milliseconds
    }
}

 
static void adc_task(void *arg) {
    uint16_t adc_value;
    uint16_t color;

    adc1_config_width(ADC_WIDTH);
    adc1_config_channel_atten(ADC_CHANNEL, ADC_ATTEN);

    while (1) {
        adc_value = adc1_get_raw(ADC_CHANNEL);
        if(adc_value < 300){
            color = 1;
        }
        else{
            color = 0;
        }
        
        if(previousColor == 0 && color){
            count++;
        }

        color = returnColor(adc_value);

        // printf("The count value is: %i\n", count);
        vTaskDelay(pdMS_TO_TICKS(10)); // Read and print ADC value every second

    }
}





void app_main() {
    xTaskCreate(adc_task, "adc_task", 2048, NULL, 5, NULL);
    xTaskCreate(timerTask, "timmerTask", 2048, NULL, 1, NULL);

}
