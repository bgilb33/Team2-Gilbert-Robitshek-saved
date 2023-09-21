// This code is from the esp-idf ADC1 example. I changed the pins to fit my circuit.
#include <stdio.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/adc.h"
#include "esp_adc_cal.h"
#include <math.h>

#define DEFAULT_VREF 1100 // Use adc2_vref_to_gpio() to obtain a better estimate
#define NO_OF_SAMPLES 64  // Multisampling
#define ALARM_PIN 13
#define THERMO_ALARM_PIN 12

static esp_adc_cal_characteristics_t *adc_chars;
static const adc1_channel_t thermistor_channel = ADC_CHANNEL_5;
static const adc_channel_t channel = ADC_CHANNEL_6; // 34
static const adc_atten_t atten = ADC_ATTEN_DB_11;
static const adc_unit_t unit = ADC_UNIT_1;

int light_alarm_state = 0;
int thermo_alarm_state = 0;

void init()
{
    gpio_reset_pin(ALARM_PIN);
    gpio_set_direction(ALARM_PIN, GPIO_MODE_OUTPUT);
}

static void check_efuse(void)
{
    // Check TP is burned into eFuse
    if (esp_adc_cal_check_efuse(ESP_ADC_CAL_VAL_EFUSE_TP) == ESP_OK)
    {
        printf("eFuse Two Point: Supported\n");
    }
    else
    {
        printf("eFuse Two Point: NOT supported\n");
    }

    // Check Vref is burned into eFuse
    if (esp_adc_cal_check_efuse(ESP_ADC_CAL_VAL_EFUSE_VREF) == ESP_OK)
    {
        printf("eFuse Vref: Supported\n");
    }
    else
    {
        printf("eFuse Vref: NOT supported\n");
    }
}

static void print_char_val_type(esp_adc_cal_value_t val_type)
{
    if (val_type == ESP_ADC_CAL_VAL_EFUSE_TP)
    {
        printf("Characterized using Two Point Value\n");
    }
    else if (val_type == ESP_ADC_CAL_VAL_EFUSE_VREF)
    {
        printf("Characterized using eFuse Vref\n");
    }
    else
    {
        printf("Characterized using Default Vref\n");
    }
}

void photoresistor_task()
{
    // Continuously sample ADC1
    while (1)
    {
        uint32_t adc_reading_photo = 0;
        // Multisampling
        for (int i = 0; i < NO_OF_SAMPLES; i++)
        {
            if (unit == ADC_UNIT_1)
            {
                adc_reading_photo += adc1_get_raw((adc1_channel_t)channel);
            }
            else
            {
                int raw;
                adc2_get_raw((adc2_channel_t)channel, ADC_WIDTH_BIT_12, &raw);
                adc_reading_photo += raw;
            }
        }
        adc_reading_photo /= NO_OF_SAMPLES;
        // Convert adc_reading to voltage in mV
        uint32_t voltage = esp_adc_cal_raw_to_voltage(adc_reading_photo, adc_chars);
        printf("Raw: %ld\tVoltage: %ldmV\n", adc_reading_photo, voltage);
        if (voltage >= 500)
        {

            light_alarm_state = 1;
        }
        else
        {
            light_alarm_state = 0;
        }
        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
}

void thermistor_task()
{
    // Continuously sample ADC1
    while (1)
    {
        uint32_t adc_reading_thermo = 0;
        // Multisampling
        for (int i = 0; i < NO_OF_SAMPLES; i++)
        {
            if (unit == ADC_UNIT_1)
            {
                adc_reading_thermo += adc1_get_raw((adc1_channel_t)thermistor_channel);
            }
            else
            {
                int raw;
                adc2_get_raw((adc2_channel_t)thermistor_channel, ADC_WIDTH_BIT_12, &raw);
                adc_reading_thermo += raw;
            }
        }
        adc_reading_thermo /= NO_OF_SAMPLES;
        // Convert adc_reading to voltage in mV
        float voltage = esp_adc_cal_raw_to_voltage(adc_reading_thermo, adc_chars);
        float resistance = 10000 * voltage / (3300 - voltage);
        float temperature = (1 / ((1 / 298.15) + (1 / 3435) * log(resistance / 10000))) - 273.1;
        printf("Thermo Raw: %ld\tTemperature: %f C\n", adc_reading_thermo, temperature);
        if (temperature > 27)
        {

            thermo_alarm_state = 1;
        }
        else
        {
            thermo_alarm_state = 0;
        }
        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
}

void sound_alarm_task()
{
    while (1)
    {
        if (light_alarm_state == 1)
        {
            gpio_set_level(ALARM_PIN, 1);
        }
        else
        {
            gpio_set_level(ALARM_PIN, 0);
        }

        if (thermo_alarm_state == 1)
        {
            gpio_set_level(THERMO_ALARM_PIN, 1);
        }
        else
        {
            gpio_set_level(THERMO_ALARM_PIN, 0);
        }
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}

void app_main(void)
{
    // Check if Two Point or Vref are burned into eFuse
    check_efuse();
    init();

    // Configure ADC
    if (unit == ADC_UNIT_1)
    {
        adc1_config_width(ADC_WIDTH_BIT_12);
        adc1_config_channel_atten(channel, atten);
    }
    else
    {
        adc2_config_channel_atten((adc2_channel_t)channel, atten);
    }

    // Characterize ADC
    adc_chars = calloc(1, sizeof(esp_adc_cal_characteristics_t));
    esp_adc_cal_value_t val_type = esp_adc_cal_characterize(unit, atten, ADC_WIDTH_BIT_12, DEFAULT_VREF, adc_chars);
    print_char_val_type(val_type);

    // Create Tasks
    xTaskCreate(sound_alarm_task, "sound_alarm_task", 1024 * 2, NULL, configMAX_PRIORITIES, NULL);
    xTaskCreate(photoresistor_task, "photoresistor_task", 1024 * 2, NULL, configMAX_PRIORITIES - 1, NULL);
    xTaskCreate(thermistor_task, "thermistore_task", 1024 * 2, NULL, configMAX_PRIORITIES - 2, NULL);
}