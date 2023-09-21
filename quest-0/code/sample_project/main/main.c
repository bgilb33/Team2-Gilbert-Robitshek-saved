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
#define BUTTON_ALARM_PIN 15
#define RESET_BUTTON 39

// Button Code
#define GPIO_INPUT_IO_1 4
#define ESP_INTR_FLAG_DEFAULT 0
#define GPIO_INPUT_PIN_SEL 1ULL << GPIO_INPUT_IO_1
int button_alarm = 0;
int flag = 0; // Flag for signaling from ISR

static esp_adc_cal_characteristics_t *adc_chars;
static const adc1_channel_t thermistor_channel = ADC_CHANNEL_5;
static const adc_channel_t channel = ADC_CHANNEL_6; // 34
static const adc_atten_t atten = ADC_ATTEN_DB_11;
static const adc_unit_t unit = ADC_UNIT_1;

int light_alarm_state = 0;
int thermo_alarm_state = 0;
float temperature = 0;
uint32_t photo_voltage = 0;

static void IRAM_ATTR gpio_isr_handler(void *arg)
{
    button_alarm = 1;
    gpio_set_level(GPIO_INPUT_IO_1, 1);
}

void init()
{
    gpio_reset_pin(ALARM_PIN);
    gpio_set_direction(ALARM_PIN, GPIO_MODE_OUTPUT);
    gpio_reset_pin(THERMO_ALARM_PIN);
    gpio_set_direction(THERMO_ALARM_PIN, GPIO_MODE_OUTPUT);
    gpio_reset_pin(BUTTON_ALARM_PIN);
    gpio_set_direction(BUTTON_ALARM_PIN, GPIO_MODE_OUTPUT);
    gpio_reset_pin(RESET_BUTTON);
    gpio_set_direction(RESET_BUTTON, GPIO_MODE_INPUT);

    gpio_config_t io_conf;
    // interrupt of rising edge
    io_conf.intr_type = GPIO_INTR_POSEDGE;
    // bit mask of the pins, use GPIO4 here
    io_conf.pin_bit_mask = GPIO_INPUT_PIN_SEL;
    // set as input mode
    io_conf.mode = GPIO_MODE_INPUT;
    // enable pull-up mode
    io_conf.pull_up_en = 1;
    gpio_config(&io_conf);
    gpio_intr_enable(GPIO_INPUT_IO_1);
    // install gpio isr service
    gpio_install_isr_service(ESP_INTR_FLAG_LEVEL3);
    // hook isr handler for specific gpio pin
    gpio_isr_handler_add(GPIO_INPUT_IO_1, gpio_isr_handler, (void *)GPIO_INPUT_IO_1);
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
        photo_voltage = esp_adc_cal_raw_to_voltage(adc_reading_photo, adc_chars);
        if (photo_voltage >= 500)
        {
            light_alarm_state = 1;
        }
        else
        {
            light_alarm_state = 0;
        }
        vTaskDelay(100 / portTICK_PERIOD_MS);
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
        float voltage = (adc_reading_thermo / 4095.0) * 3.3;
        float resistance = 10000.0 * voltage / (3.0 - voltage);
        temperature = (1.0 / 298.15) + ((1.0 / 3435.0) * (log(resistance / 10000.0)));
        temperature = (1.0 / temperature) - 273.1;

        if (temperature > 27)
        {
            thermo_alarm_state = 1;
        }
        else
        {
            thermo_alarm_state = 0;
        }
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}

void sound_alarm_task()
{
    while (1)
    {
        gpio_set_level(ALARM_PIN, light_alarm_state);
        gpio_set_level(THERMO_ALARM_PIN, thermo_alarm_state);
        if (flag)
        {

            flag = 0;
        }
        gpio_set_level(BUTTON_ALARM_PIN, button_alarm);
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}

void print_reading_task()
{
    while (1)
    {
        printf("Button Pressed: %i   Temperature: %f oC\n", button_alarm, temperature);

        if (thermo_alarm_state)
        {
            printf("HEAT DETECTED\n");
        }
        if (light_alarm_state)
        {
            printf("LIGHT DETECTED\n");
        }
        if (button_alarm)
        {
            printf("PRESSURE PLATE DETECTED\n");
        }
        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
}

void reset_task()
{
    while (1)
    {
        if (gpio_get_level(RESET_BUTTON) && button_alarm)
        {
            printf("RESET PRESSURE PLATE\n");
            button_alarm = 0;
        }
        vTaskDelay(100 / portTICK_PERIOD_MS);
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
    xTaskCreate(reset_task, "reset_task", 1024 * 2, NULL, configMAX_PRIORITIES - 1, NULL);
    xTaskCreate(print_reading_task, "print_reading_task", 1024 * 2, NULL, configMAX_PRIORITIES - 2, NULL);
    xTaskCreate(photoresistor_task, "photoresistor_task", 1024 * 2, NULL, configMAX_PRIORITIES - 3, NULL);
    xTaskCreate(thermistor_task, "thermistore_task", 1024 * 2, NULL, configMAX_PRIORITIES - 4, NULL);
}