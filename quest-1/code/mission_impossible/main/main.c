#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/adc.h"
#include "esp_adc_cal.h"
#include <math.h>

// Defines from ADC1 Example and are used in photocell and thermistor measurements
#define DEFAULT_VREF 1100 // Use adc2_vref_to_gpio() to obtain a better estimate
#define NO_OF_SAMPLES 64  // Multisampling

// LED Pin and Reset Button definitions
#define LIGHT_ALARM_PIN 15
#define THERMO_ALARM_PIN 13
#define BUTTON_ALARM_PIN 32
#define RESET_BUTTON 39
#define GREEN_PIN 12

// Definitions for Pressure Plate Button that uses a hardware interupt
#define GPIO_INPUT_IO_1 4
#define ESP_INTR_FLAG_DEFAULT 0
#define GPIO_INPUT_PIN_SEL 1ULL << GPIO_INPUT_IO_1

int flag = 0; // Flag for signaling from ISR

// Declarations and initializations for ADC readings of thermistor and photosensor
static esp_adc_cal_characteristics_t *adc_chars;
static const adc1_channel_t thermistor_channel = ADC_CHANNEL_5;
static const adc_channel_t channel = ADC_CHANNEL_6; // 34
static const adc_atten_t atten = ADC_ATTEN_DB_11;
static const adc_unit_t unit = ADC_UNIT_1;

// Alarm states
// 0: Not Detected
// 1: Detected
int button_alarm = 0;
int light_alarm_state = 0;
int thermo_alarm_state = 0;

// Global variables for temperature and photosensor voltage readings from the thermistor and photosensor
float temperature = 0;
uint32_t photo_voltage = 0;

// GPIO ISR Handler from Design Pattern -- Interrupts example in skill 12 assignment
static void IRAM_ATTR gpio_isr_handler(void *arg)
{
    // Sets pressure plate detection (button_alarm) to 1 if pressure plate button is pressed
    button_alarm = 1;
    gpio_set_level(GPIO_INPUT_IO_1, 1);
}

void init()
{
    // Initializes LED Pins as outputs
    gpio_reset_pin(LIGHT_ALARM_PIN);
    gpio_set_direction(LIGHT_ALARM_PIN, GPIO_MODE_OUTPUT);

    gpio_reset_pin(THERMO_ALARM_PIN);
    gpio_set_direction(THERMO_ALARM_PIN, GPIO_MODE_OUTPUT);

    gpio_reset_pin(BUTTON_ALARM_PIN);
    gpio_set_direction(BUTTON_ALARM_PIN, GPIO_MODE_OUTPUT);

    gpio_reset_pin(GREEN_PIN);
    gpio_set_direction(GREEN_PIN, GPIO_MODE_OUTPUT);

    // Initializes reset button as an input
    gpio_reset_pin(RESET_BUTTON);
    gpio_set_direction(RESET_BUTTON, GPIO_MODE_INPUT);

    // Configures the hardware interrupt for the pressure plate button
    // This segment of code is from the Design Pattern -- Interrupts example in skill 12 assignment

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

// Check eFuse function from ADC1 example in skill 07 assignment
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

// Print Char Val Type function from ADC1 example in skill 07 assignment
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

// Task that continually samples the adc from the photoresistor, converts that value to voltage, and changes the alarm state triggered by the photoresistor (light_alarm_state) if the voltage passes the threshold
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

        // Sets light_alarm_state to 1 if the photoresistor voltage is greater than or equal to 500mv
        // Sets light_alarm_state to 0 if the photoresistor voltage is less than to 500mv
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

// Task that continually samples the adc from the thermistor, converts that value to degress Celcius, and changes the alarm state triggered by the thermistor (thermo_alarm_state) if the temperature passes the threshold
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
        // Calculates the photoresistor resistance based on the voltage
        float resistance = 10000.0 * voltage / (3.0 - voltage);
        // Calculates the temperature in degrees Celcius using the Steinhart equation
        temperature = (1.0 / 298.15) + ((1.0 / 3435.0) * (log(resistance / 10000.0)));
        temperature = (1.0 / temperature) - 273.1;

        // Sets thermo_alarm_state to 1 if the temperature is greater than 27 degrees Celcius
        // Sets thermo_alarm_state to 0 if the temperature is less than or equal to 27 degrees Celcius
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

// Continuously checks the status of the three alarms and lights the correct LED if the alarm state is equal to 1
// Lights the green LED if all alarm states are equal to 0
void sound_alarm_task()
{
    while (1)
    {
        // Lights the green LED if no alarms are triggered
        if (!light_alarm_state & !thermo_alarm_state & !button_alarm)
        {
            gpio_set_level(GREEN_PIN, 1);
        }
        else
        {
            gpio_set_level(GREEN_PIN, 0);
        }

        // Lights the yellow LED if the light_alarm_state is equal to 1
        gpio_set_level(LIGHT_ALARM_PIN, light_alarm_state);
        // Lights the red LED if the thermo_alarm_state is equal to 1
        gpio_set_level(THERMO_ALARM_PIN, thermo_alarm_state);
        // Lights the blue LED if the button_alarm is equal to 1
        gpio_set_level(BUTTON_ALARM_PIN, button_alarm);

        // Resets the flag after a hardware interrupt from the pressure plate button occurs
        if (flag)
        {

            flag = 0;
        }

        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}

// Prints the values and status of the sensors every 2 seconds
// If an alarm state is 1, it's status will be printed as "Detected"
// If an alarm state is 0, it's status will be printed as "Not Detected"
// Value of button will be: 1 for pressed, 0 for not pressed
// Value of thermistor will be in degrees Celcius
// Value of photoresistor will be in mV
void print_reading_task()
{
    while (1)
    {
        // Get string values for a sensor's status based on its alarm state
        char buttonOutput[13];
        char lightOutput[13];
        char thermoOutput[13];
        if (button_alarm)
        {
            strcpy(buttonOutput, "Detected");
        }
        else
        {
            strcpy(buttonOutput, "Not Detected");
        }
        if (light_alarm_state)
        {
            strcpy(lightOutput, "Detected");
        }
        else
        {
            strcpy(lightOutput, "Not Detected");
        }
        if (thermo_alarm_state)
        {
            strcpy(thermoOutput, "Detected");
        }
        else
        {
            strcpy(thermoOutput, "Not Detected");
        }

        // Formats the print and prints it to the console
        printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
        printf("       Status          Value\n");
        printf("Button: %s             %i \n", buttonOutput, button_alarm);
        printf("Light:  %s             %ld mV\n", lightOutput, photo_voltage);
        printf("Heat:   %s             %f oC\n", thermoOutput, temperature);

        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
}

// Resets the pressure plate button alarm state when the reset button is pressed using the polling design pattern
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
    // Initialize pins and hardware interrupt
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
    // Tasks in priority order (high to low): Sound Alarm, Reset Pressure Plate Alarm, Print Current Readings, Photoresistor Reading, Thermistor Reading
    xTaskCreate(sound_alarm_task, "sound_alarm_task", 1024 * 2, NULL, configMAX_PRIORITIES, NULL);
    xTaskCreate(reset_task, "reset_task", 1024 * 2, NULL, configMAX_PRIORITIES - 1, NULL);
    xTaskCreate(print_reading_task, "print_reading_task", 1024 * 2, NULL, configMAX_PRIORITIES - 2, NULL);
    xTaskCreate(photoresistor_task, "photoresistor_task", 1024 * 2, NULL, configMAX_PRIORITIES - 3, NULL);
    xTaskCreate(thermistor_task, "thermistore_task", 1024 * 2, NULL, configMAX_PRIORITIES - 4, NULL);
}
