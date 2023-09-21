#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/adc.h"
#include <math.h>


void app_main() {
    // Initialize ADC
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(ADC1_CHANNEL_0, ADC_ATTEN_DB_11);
    
    // Read ADC value from channel 0
    // int raw_adc_value = adc1_get_raw(ADC1_CHANNEL_0);

    // Convert ADC value to voltage (assuming 3.3V reference)




    while(1){
        for (int i = 0; i < 5; i++)
        {
            int raw_adc_value = adc1_get_raw(ADC1_CHANNEL_0);

            // Convert ADC value to voltage (assuming 3.3V reference)
            float voltage = (raw_adc_value / 4095.0) * 3.3;

            float resistance = 10000.0 * voltage / (3.0 - voltage);                            // R1 * V_out / (V_s - V_out)
            float temp = 1.0 / ((1.0/298.15) + (1.0/3435.0) * (log(resistance/10000.0)));       // Steinhart-hart's equation: T = 1/(1/To + log(Rt/Ro) *  1/B)
            float celTemp = temp - 273.1;
            printf("Temp: %f\tResistance: %f\n", celTemp, resistance);
                
            // Convert Kelvin to Celsius or Fahrenheit if needed
            // float temperatureCelsius = temperatureKelvin - 273.15;
            vTaskDelay(100);
        }

    }
}
