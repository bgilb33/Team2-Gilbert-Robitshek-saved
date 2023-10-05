#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "driver/gpio.h"

// Define the GPIO pin connected to the buzzer
#define BUZZER_GPIO 27  // Change this to the GPIO pin you are using

void app_main() {
    // Configure the GPIO pin for the buzzer as output
    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = (1ULL << BUZZER_GPIO);
    io_conf.pull_down_en = 0;
    io_conf.pull_up_en = 0;
    gpio_config(&io_conf);

    // Turn on the buzzer
    gpio_set_level(BUZZER_GPIO, 1);

    // Delay for a few seconds
    vTaskDelay(5000 / portTICK_PERIOD_MS); // 5000 milliseconds (5 seconds)

    // Turn off the buzzer
    gpio_set_level(BUZZER_GPIO, 0);

    // End of program
    vTaskDelete(NULL);
}


// #include <stdio.h>
// #include "freertos/FreeRTOS.h"
// #include "freertos/task.h"
// #include "driver/adc.h"
// #include <math.h>


// void app_main() {
//     // Initialize ADC
//     adc1_config_width(ADC_WIDTH_BIT_12);
//     adc1_config_channel_atten(ADC1_CHANNEL_0, ADC_ATTEN_DB_11);
    
//     // Read ADC value from channel 0
//     // int raw_adc_value = adc1_get_raw(ADC1_CHANNEL_0);

//     // Convert ADC value to voltage (assuming 3.3V reference)




//     while(1){
//         for (int i = 0; i < 5; i++)
//         {
//             int raw_adc_value = adc1_get_raw(ADC1_CHANNEL_0);

//             // Convert ADC value to voltage (assuming 3.3V reference)
//             float voltage = (raw_adc_value / 4095.0) * 3.3;

//             float resistance = 10000.0 * voltage / (3.0 - voltage);                           
//             float temp = 1.0 / ((1.0/298.15) + (1.0/3435.0) * (log(resistance/10000.0)));       
//             printf("%f\n", (temp - 275.1));
                
//             // Convert Kelvin to Celsius or Fahrenheit if needed
//             // float temperatureCelsius = temperatureKelvin - 273.15;
//             vTaskDelay(100);
//         }

//     }
// }