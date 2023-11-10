#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/mcpwm_prelude.h"
#include "driver/uart.h"
#include "esp_vfs_dev.h"
#include "driver/adc.h"
#include "esp_timer.h"
#include "driver/gpio.h"

// Lidar Includes
#include <stdio.h>
#include "esp_log.h"
#include "driver/i2c.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
// Lidar Includes

// UDP Includes
#include <string.h>
#include <sys/param.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "protocol_examples_common.h"

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include <lwip/netdb.h>

#define PORT CONFIG_EXAMPLE_PORT
// UDP Includes

// Timer Includes
#include <stdio.h>
#include "driver/i2c.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/periph_ctrl.h"
#include "driver/timer.h"
#include "driver/gpio.h"
#include <math.h>
// Timer Includes End

int count = 0;
int previousColor = 0;

float averageDistance = 0;

static const char *TAG = "example";
char control[] = " ";

float distance_1;
float distance_2;
int16_t frontDistance = 0;

int autoBreak = 0;

bool remoteControl = 0;

bool encounter = 0;

bool largeBreak = 1; // large break after encountering obsticle

// Please consult the datasheet of your servo before changing the following parameters
#define SERVO_MIN_PULSEWIDTH_US 700  // Minimum pulse width in microsecond
#define SERVO_MAX_PULSEWIDTH_US 2300 // Maximum pulse width in microsecond
#define SERVO_MIN_DEGREE -60         // Minimum angle
#define SERVO_MAX_DEGREE 60          // Maximum angle

#define SERVO_PULSE_GPIO 33                        // GPIO connects to the PWM signal line
#define SERVO_TIMEBASE_RESOLUTION_HZ 1000000       // 1MHz, 1us per tick
#define STEER_SERVO_TIMEBASE_RESOLUTION_HZ 1000000 // 1MHz, 1us per tick
#define SERVO_TIMEBASE_PERIOD 20000                // 20000 ticks, 20ms

#define MOTOR_PULSE_GPIO 32 // GPIO connects to the PWM signal line
// #define SERVO_TIMEBASE_RESOLUTION_HZ 1000000 // 1MHz, 1us per tick
// #define SERVO_TIMEBASE_PERIOD 20000          // 20000 ticks, 20ms

#define DEFAULT_VREF 1100   // Default reference voltage (in mV) for ADC
#define ADC_UNIT ADC_UNIT_1 // ADC unit
#define ADC_CHANNEL ADC1_CHANNEL_0
#define ADC_WIDTH ADC_WIDTH_BIT_12 // ADC bit width
#define ADC_ATTEN ADC_ATTEN_DB_11  // ADC attenuation

#define TRIGGER_PIN 17
#define ECHO_PIN 16
#define TRIGGER_PIN_2 13
#define ECHO_PIN_2 12

// Lidar Defines
#define I2C_EXAMPLE_MASTER_SCL_IO          22   // gpio number for i2c clk
#define I2C_EXAMPLE_MASTER_SDA_IO          23   // gpio number for i2c data
#define I2C_EXAMPLE_MASTER_NUM             I2C_NUM_0  // i2c port
#define I2C_EXAMPLE_MASTER_TX_BUF_DISABLE  0    // i2c master no buffer needed
#define I2C_EXAMPLE_MASTER_RX_BUF_DISABLE  0    // i2c master no buffer needed
#define I2C_EXAMPLE_MASTER_FREQ_HZ         100000     // i2c master clock freq
#define WRITE_BIT                          I2C_MASTER_WRITE // i2c master write
#define READ_BIT                           I2C_MASTER_READ  // i2c master read
#define ACK_CHECK_EN                       true // i2c master will check ack
#define ACK_CHECK_DIS                      false// i2c master will not check ack
#define ACK_VAL                            0x00 // i2c ack value
#define NACK_VAL                           0x1 // i2c nack value

#define MPU9250_SENSOR_ADDR                 0x62        /*!< Slave address of the MPU9250 sensor */
// Lidar Define End

// Timer Define
#define SLAVE_ADDR 0x70              // alphanumeric address
#define OSC 0x21                     // oscillator cmd
#define HT16K33_BLINK_DISPLAYON 0x01 // Display on cmd
#define HT16K33_BLINK_OFF 0          // Blink off cmd
#define HT16K33_BLINK_CMD 0x80       // Blink cmd
#define HT16K33_CMD_BRIGHTNESS 0xE0  // Brightness cmd

#define ZERO 0b00111111  // 0
#define ONE 0b00000110   // 1
#define TWO 0b01011011   // 2
#define THREE 0b01001111 // 3
#define FOUR 0b01100110  // 4
#define FIVE 0b01101101  // 5
#define SIX 0b01111101   // 6
#define SEVEN 0b00000111 // 7
#define EIGHT 0b01111111 // 8
#define NINE 0b01101111  // 9
#define COLON 0b00001001 // :

#define GPIO_INPUT_IO_1 4
#define ESP_INTR_FLAG_DEFAULT 0
#define GPIO_INPUT_PIN_SEL 1ULL << GPIO_INPUT_IO_1

#define GPT_TIMER_GROUP TIMER_GROUP_0
#define GPT_TIMER_INDEX TIMER_0
#define GPT_INTERVAL_MS 1000 // Interval in milliseconds

// Initialize start time, current time, and array to display
double start_time = 0.00;
double *current_time = &start_time;
uint16_t displaybuffer[3];

int wheelSpeed = 0;

bool sideSensor = 1;


int flag = 0;
// Timer Define End

//////
// Timer Tasks
/////
static void IRAM_ATTR gpio_isr_handler(void *arg)
{
    flag = 1;
}

// Initialize Hardware Interrupt and timer
void init()
{
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

    timer_config_t config = {
        .divider = 16, // 16 prescaler
        .counter_dir = TIMER_COUNT_UP,
        .counter_en = TIMER_PAUSE,
        .alarm_en = TIMER_ALARM_EN,
        .auto_reload = true,
        .intr_type = TIMER_INTR_LEVEL,
    };

    timer_init(GPT_TIMER_GROUP, GPT_TIMER_INDEX, &config);
    timer_set_counter_value(GPT_TIMER_GROUP, GPT_TIMER_INDEX, 0);
    timer_set_alarm_value(GPT_TIMER_GROUP, GPT_TIMER_INDEX, GPT_INTERVAL_MS * 1000); // microseconds
    timer_enable_intr(GPT_TIMER_GROUP, GPT_TIMER_INDEX);
    timer_start(GPT_TIMER_GROUP, GPT_TIMER_INDEX);
}

void button_task()
{
    current_time = &start_time;
    timer_pause(GPT_TIMER_GROUP, GPT_TIMER_INDEX);
    timer_set_counter_value(GPT_TIMER_GROUP, GPT_TIMER_INDEX, 0);
    timer_start(GPT_TIMER_GROUP, GPT_TIMER_INDEX);
    while (1)
    {

        timer_get_counter_time_sec(GPT_TIMER_GROUP, GPT_TIMER_INDEX, current_time);
        *current_time = roundf(*current_time * 100) / 100; // Set to two decimal places

        if (*current_time >= 99.00)
        {
            current_time = &start_time;
            timer_pause(GPT_TIMER_GROUP, GPT_TIMER_INDEX);
            timer_set_counter_value(GPT_TIMER_GROUP, GPT_TIMER_INDEX, 0);
            timer_start(GPT_TIMER_GROUP, GPT_TIMER_INDEX);
        }

        vTaskDelay(10 / portTICK_PERIOD_MS); // Delay for 1 second
    }
}

// ISR Handler

// Turn on oscillator for alpha display
int alpha_oscillator()
{
    int ret;
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (SLAVE_ADDR << 1) | WRITE_BIT, ACK_CHECK_EN);
    i2c_master_write_byte(cmd, OSC, ACK_CHECK_EN);
    i2c_master_stop(cmd);
    ret = i2c_master_cmd_begin(I2C_EXAMPLE_MASTER_NUM, cmd, 1000 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);
    vTaskDelay(200 / portTICK_PERIOD_MS);
    return ret;
}

// Set blink rate to off
int no_blink()
{
    int ret;
    i2c_cmd_handle_t cmd2 = i2c_cmd_link_create();
    i2c_master_start(cmd2);
    i2c_master_write_byte(cmd2, (SLAVE_ADDR << 1) | WRITE_BIT, ACK_CHECK_EN);
    i2c_master_write_byte(cmd2, HT16K33_BLINK_CMD | HT16K33_BLINK_DISPLAYON | (HT16K33_BLINK_OFF << 1), ACK_CHECK_EN);
    i2c_master_stop(cmd2);
    ret = i2c_master_cmd_begin(I2C_EXAMPLE_MASTER_NUM, cmd2, 1000 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd2);
    vTaskDelay(200 / portTICK_PERIOD_MS);
    return ret;
}

// Set Brightness
int set_brightness_max(uint8_t val)
{
    int ret;
    i2c_cmd_handle_t cmd3 = i2c_cmd_link_create();
    i2c_master_start(cmd3);
    i2c_master_write_byte(cmd3, (SLAVE_ADDR << 1) | WRITE_BIT, ACK_CHECK_EN);
    i2c_master_write_byte(cmd3, HT16K33_CMD_BRIGHTNESS | val, ACK_CHECK_EN);
    i2c_master_stop(cmd3);
    ret = i2c_master_cmd_begin(I2C_EXAMPLE_MASTER_NUM, cmd3, 1000 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd3);
    vTaskDelay(200 / portTICK_PERIOD_MS);
    return ret;
}

// Transform current time into 7-segment display binary
void char_to_display_task()
{

    int digits[3];
    int integerPart;
    int fractionalPart;
    while (1)
    {
        // From GPT
        // Prompt: How can I extract each digit from a double number that has two integer digits and two fractional digits?
        integerPart = (int)*current_time;
        fractionalPart = (int)((*current_time - integerPart) * 100);
        // Extract individual digits
        digits[0] = integerPart / 10;
        digits[1] = integerPart % 10;
        digits[2] = fractionalPart / 10;
        digits[3] = fractionalPart % 10;
        // End GPT

        for (int i = 0; i < 4; i++)
        {
            switch (digits[i])
            {
            case 0:
                displaybuffer[i] = ZERO;
                break;
            case 1:
                displaybuffer[i] = ONE;
                break;
            case 2:
                displaybuffer[i] = TWO;
                break;
            case 3:
                displaybuffer[i] = THREE;
                break;
            case 4:
                displaybuffer[i] = FOUR;
                break;
            case 5:
                displaybuffer[i] = FIVE;
                break;
            case 6:
                displaybuffer[i] = SIX;
                break;
            case 7:
                displaybuffer[i] = SEVEN;
                break;
            case 8:
                displaybuffer[i] = EIGHT;
                break;
            case 9:
                displaybuffer[i] = NINE;
                break;
            }
        }
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}

// Print to I2C Display
static void test_alpha_display()
{
    // Debug
    int ret;
    printf(">> Test Alphanumeric Display: \n");

    // Set up routines
    // Turn on alpha oscillator
    ret = alpha_oscillator();
    if (ret == ESP_OK)
    {
        printf("- oscillator: ok \n");
    }
    // Set display blink off
    ret = no_blink();
    if (ret == ESP_OK)
    {
        printf("- blink: off \n");
    }
    ret = set_brightness_max(0xF);
    if (ret == ESP_OK)
    {
        printf("- brightness: max \n");
    }
    // Continually writes the same command

    while (1)
    {

        for (int i = 0; i < 100; i++)
        {
            // Send commands characters to display over I2C
            i2c_cmd_handle_t cmd4 = i2c_cmd_link_create();
            i2c_master_start(cmd4);
            i2c_master_write_byte(cmd4, (SLAVE_ADDR << 1) | WRITE_BIT, ACK_CHECK_EN);
            i2c_master_write_byte(cmd4, (uint8_t)0x00, ACK_CHECK_EN);
            for (uint8_t i = 0; i < 4; i++)
            {
                i2c_master_write_byte(cmd4, displaybuffer[i] & 0xFF, ACK_CHECK_EN);
                i2c_master_write_byte(cmd4, displaybuffer[i] >> 8, ACK_CHECK_EN);
            }
            i2c_master_stop(cmd4);
            ret = i2c_master_cmd_begin(I2C_EXAMPLE_MASTER_NUM, cmd4, 1000 / portTICK_PERIOD_MS);
            i2c_cmd_link_delete(cmd4);

            vTaskDelay(10 / portTICK_PERIOD_MS);
        }
    }
}
/////
// Timer Ends
////


void ultrasonic_setup()
{
    gpio_config_t io_conf_1 = {
        .pin_bit_mask = (1ULL << TRIGGER_PIN) | (1ULL << ECHO_PIN),
        .mode = GPIO_MODE_OUTPUT,
        .intr_type = GPIO_INTR_DISABLE,
        .pull_down_en = 0,
        .pull_up_en = 0,
    };
    gpio_config(&io_conf_1);

    // Configure the ECHO_PIN as input
    io_conf_1.pin_bit_mask = (1ULL << ECHO_PIN);
    io_conf_1.mode = GPIO_MODE_INPUT;
    gpio_config(&io_conf_1);

    gpio_config_t io_conf_2 = {
        .pin_bit_mask = (1ULL << TRIGGER_PIN_2) | (1ULL << ECHO_PIN_2),
        .mode = GPIO_MODE_OUTPUT,
        .intr_type = GPIO_INTR_DISABLE,
        .pull_down_en = 0,
        .pull_up_en = 0,
    };
    gpio_config(&io_conf_2);

    // Configure the ECHO_PIN_2 as input
    io_conf_2.pin_bit_mask = (1ULL << ECHO_PIN_2);
    io_conf_2.mode = GPIO_MODE_INPUT;
    gpio_config(&io_conf_2);
}

static inline uint32_t example_angle_to_compare(int angle)
{
    return (angle - SERVO_MIN_DEGREE) * (SERVO_MAX_PULSEWIDTH_US - SERVO_MIN_PULSEWIDTH_US) / (SERVO_MAX_DEGREE - SERVO_MIN_DEGREE) + SERVO_MIN_PULSEWIDTH_US;
}

void rotate_wheels_task()
{
    ESP_LOGI(TAG, "Create timer and operator");
    mcpwm_timer_handle_t timer_servo = NULL;
    mcpwm_timer_config_t timer_config = {
        .group_id = 0,
        .clk_src = MCPWM_TIMER_CLK_SRC_DEFAULT,
        .resolution_hz = STEER_SERVO_TIMEBASE_RESOLUTION_HZ,
        .period_ticks = SERVO_TIMEBASE_PERIOD,
        .count_mode = MCPWM_TIMER_COUNT_MODE_UP,
    };
    ESP_ERROR_CHECK(mcpwm_new_timer(&timer_config, &timer_servo));

    mcpwm_oper_handle_t oper = NULL;
    mcpwm_operator_config_t operator_config = {
        .group_id = 0, // operator must be in the same group to the timer
    };
    ESP_ERROR_CHECK(mcpwm_new_operator(&operator_config, &oper));

    ESP_LOGI(TAG, "Connect timer and operator");
    ESP_ERROR_CHECK(mcpwm_operator_connect_timer(oper, timer_servo));

    ESP_LOGI(TAG, "Create comparator and generator from the operator");
    mcpwm_cmpr_handle_t comparator = NULL;
    mcpwm_comparator_config_t comparator_config = {
        .flags.update_cmp_on_tez = true,
    };
    ESP_ERROR_CHECK(mcpwm_new_comparator(oper, &comparator_config, &comparator));

    mcpwm_gen_handle_t generator = NULL;
    mcpwm_generator_config_t generator_config = {
        .gen_gpio_num = SERVO_PULSE_GPIO,
    };
    ESP_ERROR_CHECK(mcpwm_new_generator(oper, &generator_config, &generator));

    // set the initial compare value, so that the servo will spin to the center position
    ESP_ERROR_CHECK(mcpwm_comparator_set_compare_value(comparator, example_angle_to_compare(0)));

    ESP_LOGI(TAG, "Set generator action on timer and compare event");
    // go high on counter empty
    ESP_ERROR_CHECK(mcpwm_generator_set_action_on_timer_event(generator,
                                                              MCPWM_GEN_TIMER_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP, MCPWM_TIMER_EVENT_EMPTY, MCPWM_GEN_ACTION_HIGH)));
    // go low on compare threshold
    ESP_ERROR_CHECK(mcpwm_generator_set_action_on_compare_event(generator,
                                                                MCPWM_GEN_COMPARE_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP, comparator, MCPWM_GEN_ACTION_LOW)));

    ESP_LOGI(TAG, "Enable and start timer");
    ESP_ERROR_CHECK(mcpwm_timer_enable(timer_servo));
    ESP_ERROR_CHECK(mcpwm_timer_start_stop(timer_servo, MCPWM_TIMER_START_NO_STOP));

    int angle = 0;
    ESP_ERROR_CHECK(mcpwm_comparator_set_compare_value(comparator, example_angle_to_compare(0)));
    vTaskDelay(pdMS_TO_TICKS(3100));
    while (1)
    {
        if (control[0] == 'a')
        {
            if (angle < 0)
            {
                ESP_ERROR_CHECK(mcpwm_comparator_set_compare_value(comparator, example_angle_to_compare(0)));
                vTaskDelay(pdMS_TO_TICKS(500));
            }
            angle = 10;
        }
        else if (control[0] == 's')
        {
            angle = 0;
        }
        else if (control[0] == 'd')
        {
            if (angle > 0)
            {
                ESP_ERROR_CHECK(mcpwm_comparator_set_compare_value(comparator, example_angle_to_compare(0)));
                vTaskDelay(pdMS_TO_TICKS(500));
            }
            angle = -10;
        }
        ESP_ERROR_CHECK(mcpwm_comparator_set_compare_value(comparator, example_angle_to_compare(angle)));
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

void run_motor_task()
{
    ESP_LOGI(TAG, "Create timer and operator");
    mcpwm_timer_handle_t timer = NULL;
    mcpwm_timer_config_t timer_config = {
        .group_id = 0,
        .clk_src = MCPWM_TIMER_CLK_SRC_DEFAULT,
        .resolution_hz = SERVO_TIMEBASE_RESOLUTION_HZ,
        .period_ticks = SERVO_TIMEBASE_PERIOD,
        .count_mode = MCPWM_TIMER_COUNT_MODE_UP,
    };
    ESP_ERROR_CHECK(mcpwm_new_timer(&timer_config, &timer));

    mcpwm_oper_handle_t oper = NULL;
    mcpwm_operator_config_t operator_config = {
        .group_id = 0, // operator must be in the same group to the timer
    };
    ESP_ERROR_CHECK(mcpwm_new_operator(&operator_config, &oper));

    ESP_LOGI(TAG, "Connect timer and operator");
    ESP_ERROR_CHECK(mcpwm_operator_connect_timer(oper, timer));

    ESP_LOGI(TAG, "Create comparator and generator from the operator");
    mcpwm_cmpr_handle_t comparator = NULL;
    mcpwm_comparator_config_t comparator_config = {
        .flags.update_cmp_on_tez = true,
    };
    ESP_ERROR_CHECK(mcpwm_new_comparator(oper, &comparator_config, &comparator));

    mcpwm_gen_handle_t generator = NULL;
    mcpwm_generator_config_t generator_config = {
        .gen_gpio_num = MOTOR_PULSE_GPIO,
    };
    ESP_ERROR_CHECK(mcpwm_new_generator(oper, &generator_config, &generator));

    // set the initial compare value, so that the servo will spin to the center position
    ESP_ERROR_CHECK(mcpwm_comparator_set_compare_value(comparator, example_angle_to_compare(0)));

    ESP_LOGI(TAG, "Set generator action on timer and compare event");
    // go high on counter empty
    ESP_ERROR_CHECK(mcpwm_generator_set_action_on_timer_event(generator,
                                                              MCPWM_GEN_TIMER_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP, MCPWM_TIMER_EVENT_EMPTY, MCPWM_GEN_ACTION_HIGH)));
    // go low on compare threshold
    ESP_ERROR_CHECK(mcpwm_generator_set_action_on_compare_event(generator,
                                                                MCPWM_GEN_COMPARE_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP, comparator, MCPWM_GEN_ACTION_LOW)));

    ESP_LOGI(TAG, "Enable and start timer");
    ESP_ERROR_CHECK(mcpwm_timer_enable(timer));
    ESP_ERROR_CHECK(mcpwm_timer_start_stop(timer, MCPWM_TIMER_START_NO_STOP));

    int inReverse = 0;
    int firstControl = 1;
    ESP_ERROR_CHECK(mcpwm_comparator_set_compare_value(comparator, example_angle_to_compare(0)));
    vTaskDelay(pdMS_TO_TICKS(3100));
    while (1)
    {
        if (control[0] == 'f')
        {
            firstControl = 0;
            inReverse = 0;
            wheelSpeed = 8;
        }
        else if (control[0] == 'b')
            {
            firstControl = 0;
            // inReverse = 0;
            wheelSpeed = 0;
        }
        else if (control[0] == 'r')
        {
            if (!inReverse && !firstControl)
            {
                ESP_ERROR_CHECK(mcpwm_comparator_set_compare_value(comparator, example_angle_to_compare(-10)));
                vTaskDelay(pdMS_TO_TICKS(500));
                ESP_ERROR_CHECK(mcpwm_comparator_set_compare_value(comparator, example_angle_to_compare(0)));
                vTaskDelay(pdMS_TO_TICKS(500));
                inReverse = 1;
            }
            wheelSpeed = -10;
        }
        
        ESP_ERROR_CHECK(mcpwm_comparator_set_compare_value(comparator, example_angle_to_compare(wheelSpeed)));
        // Add delay, since it takes time for servo to rotate, usually 200ms/60degree rotation under 5V power supply
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

void ultrasonic_task()
{
    while (1)
    {
        // Trigger pulse for the first sensor
        gpio_set_level(TRIGGER_PIN, 1);
        vTaskDelay(1 / portTICK_PERIOD_MS);
        gpio_set_level(TRIGGER_PIN, 0);

        // Wait for the echo signal from the first sensor
        while (gpio_get_level(ECHO_PIN) == 0)
        {
            taskYIELD();
        }

        // Measure the duration of the echo signal from the first sensor
        uint64_t start_time_1 = esp_timer_get_time();
        while (gpio_get_level(ECHO_PIN) == 1)
        {
            taskYIELD();
        }
        uint64_t end_time_1 = esp_timer_get_time();

        // Calculate distance based on the speed of sound for the first sensor
        distance_1 = ((end_time_1 - start_time_1) * 0.0343) / 2.0;
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

void ultrasonic_task2()
{
    while (1)
    {
        gpio_set_level(TRIGGER_PIN_2, 1);
        vTaskDelay(1 / portTICK_PERIOD_MS);
        gpio_set_level(TRIGGER_PIN_2, 0);

        // Wait for the echo signal from the first sensor
        while (gpio_get_level(ECHO_PIN_2) == 0)
        {
            taskYIELD();
        }

        // Measure the duration of the echo signal from the first sensor
        uint64_t start_time_2 = esp_timer_get_time();
        while (gpio_get_level(ECHO_PIN_2) == 1)
        {
            taskYIELD();
        }
        uint64_t end_time_2 = esp_timer_get_time();

        // Calculate distance based on the speed of sound for the first sensor
        distance_2 = ((end_time_2 - start_time_2) * 0.0343) / 2.0;
        vTaskDelay(650 / portTICK_PERIOD_MS);
    }
}

int returnColor(uint16_t adc_value)
{
    int color;
    if (adc_value < 300){color = 1;}
    else{color = 0;}

    if (previousColor == 0 && color){count++;}
    previousColor = color;
    return color;
}

void timerTask(void *pvParameters)
{
    int startCount = 0; // Initialize startCount

    while (1)
    {
        startCount = count;
        vTaskDelay(pdMS_TO_TICKS(1000)); // Delay the task for 1000 milliseconds

        // Calculate the number of negative edges in 1 second
        float difference = count - startCount;

        // negative edges / second
        float nedEdges = difference / 1;
        float rotationPerSec = nedEdges * (1.0 / 4.0);
        float inchesPerSecond = rotationPerSec * (7);

        // printf("The inches per sec is: %f\n", inchesPerSecond);
        vTaskDelay(pdMS_TO_TICKS(10)); // Delay the task for 1000 milliseconds
    }
}

static void adc_task(void *arg)
{
    uint16_t adc_value;
    uint16_t color;

    adc1_config_width(ADC_WIDTH);
    adc1_config_channel_atten(ADC_CHANNEL, ADC_ATTEN);

    while (1)
    {
        adc_value = adc1_get_raw(ADC_CHANNEL);
        color = returnColor(adc_value);

        // printf("The count value is: %i\n", count);
        vTaskDelay(pdMS_TO_TICKS(10)); // Read and print ADC value every second
    }
}

//////////////////////////
// Lidar Functions Start
////////////////////////
static esp_err_t i2c_master_init(void)
{
    int i2c_master_port = I2C_EXAMPLE_MASTER_NUM;

    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_EXAMPLE_MASTER_SDA_IO,
        .scl_io_num = I2C_EXAMPLE_MASTER_SCL_IO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_EXAMPLE_MASTER_FREQ_HZ,
    };

    i2c_param_config(i2c_master_port, &conf);

    return i2c_driver_install(i2c_master_port, conf.mode, I2C_EXAMPLE_MASTER_RX_BUF_DISABLE, I2C_EXAMPLE_MASTER_TX_BUF_DISABLE, 0);
}

// Write one byte to register
int writeRegister(uint8_t reg, uint8_t data) {
    i2c_cmd_handle_t cmd = i2c_cmd_link_create(); 
    // Create an I2C command
    // Start the I2C communication, write the register address, and data
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (MPU9250_SENSOR_ADDR << 1) | WRITE_BIT, ACK_CHECK_EN);
    i2c_master_write_byte(cmd, reg, ACK_CHECK_EN);
    i2c_master_write_byte(cmd, data, ACK_CHECK_EN);
    i2c_master_stop(cmd);
    // Execute the I2C command
    int err = i2c_master_cmd_begin(I2C_EXAMPLE_MASTER_NUM, cmd, 1000 / portTICK_PERIOD_MS);
        // if (err == ESP_OK) {
        //     ESP_LOGI(TAG, "Write operation was successful");
        // } else {
        //     ESP_LOGE(TAG, "Write operation failed with error code 0x%X", err);
        // }
    i2c_cmd_link_delete(cmd);
    return err; // Return success code or an error code
}

int testConnection(uint8_t devAddr, int32_t timeout) {
  i2c_cmd_handle_t cmd = i2c_cmd_link_create();
  i2c_master_start(cmd);
  i2c_master_write_byte(cmd, (devAddr << 1) | I2C_MASTER_WRITE, ACK_CHECK_EN);
  i2c_master_stop(cmd);
  int err = i2c_master_cmd_begin(I2C_EXAMPLE_MASTER_NUM, cmd, 1000 / portTICK_PERIOD_MS);
  i2c_cmd_link_delete(cmd);
  return err;
}

// Utility function to scan for i2c device
static void i2c_scanner() {
  int32_t scanTimeout = 1000;
  printf("\n>> I2C scanning ..."  "\n");
  uint8_t count = 0;
  for (uint8_t i = 1; i < 127; i++) {
    if (testConnection(i, scanTimeout) == ESP_OK) {
      printf( "- Device found at address: 0x%X%s", i, "\n");
      count++;
    }
  }
  if (count == 0) {printf("- No I2C devices found!" "\n");}
}

// Read registere
uint8_t readRegister(uint8_t reg) {
    uint8_t data;
    i2c_cmd_handle_t cmd = i2c_cmd_link_create(); // Create an I2C command
    // Start the I2C communication, write the register address
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (MPU9250_SENSOR_ADDR << 1) | WRITE_BIT, ACK_CHECK_EN);
    i2c_master_write_byte(cmd, reg, ACK_CHECK_EN);

    // Restart and read data
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (MPU9250_SENSOR_ADDR << 1) | READ_BIT, ACK_CHECK_EN);
    i2c_master_read_byte(cmd, &data, I2C_MASTER_ACK); // Read the data with ACK
    i2c_master_stop(cmd);
    // Execute the I2C command
    int err = i2c_master_cmd_begin(I2C_EXAMPLE_MASTER_NUM, cmd, 1000 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);
    if (err == ESP_OK) {
    return data; // Return the read data
    } else {
    return 0; // Return an error value or handle the error as needed
    }
}

// read 16 bits (2 bytes)
int16_t read16(uint8_t reg) {
    uint8_t data_low, data_high;
    i2c_cmd_handle_t cmd = i2c_cmd_link_create(); // Create an I2C command
    // Start the I2C communication, write the register address, and read data
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (MPU9250_SENSOR_ADDR << 1) | WRITE_BIT, ACK_CHECK_EN);
    i2c_master_write_byte(cmd, reg, ACK_CHECK_EN);
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (MPU9250_SENSOR_ADDR << 1) | READ_BIT, ACK_CHECK_EN);
    i2c_master_read_byte(cmd, &data_low, ACK_VAL);
    i2c_master_read_byte(cmd, &data_high, NACK_VAL); // NACK the last byte
    i2c_master_stop(cmd);
    // Execute the I2C command
    int err = i2c_master_cmd_begin(I2C_EXAMPLE_MASTER_NUM, cmd, 1000 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);
    if (err == ESP_OK) {
    int16_t value = (int16_t)((data_high << 8) | data_low);
    return value; // Return the 16-bit value
    } else {
    return 0; // Return an error value or handle the error as needed
    }
}

void printDistance(void *pvParameters) {
    // Your task code goes here
    
    while (1) {
        printf("///\n");
        printf("The distance is: %i\n", frontDistance);
        printf("Wheel speed = %i cm per second\n", wheelSpeed);
        printf("Distance Sensor 2: %.2f cm\n", distance_2);
        printf("Distance Sensor 1: %.2f cm\n", distance_1);
        printf("Average Distance = %.2f cm per second\n", averageDistance);
        printf("SideSensor = %i \n", sideSensor);


        vTaskDelay(pdMS_TO_TICKS(250)); // Example: Delay for 1 second
    }
}

void calcDistance(void *pvParameters) {
    // Your task code goes here
    
    while(1){
        uint8_t regValue = 0x04;
        writeRegister(0x00, regValue);

        uint8_t LSB = 1;
        while (LSB) {
            regValue = readRegister(0x01);
            LSB = regValue & 1;
        }
        frontDistance = read16(0x10);
        vTaskDelay(50 / portTICK_PERIOD_MS);


    }
}
////////////////////////
// Lidar Functions End
////////////////////////

/// UDP Start
static void udp_server_task(void *pvParameters)
{
    char rx_buffer[128];
    char addr_str[128];
    int addr_family = (int)pvParameters;
    int ip_protocol = 0;
    struct sockaddr_in6 dest_addr;

    while (1) {

        if (addr_family == AF_INET) {
            struct sockaddr_in *dest_addr_ip4 = (struct sockaddr_in *)&dest_addr;
            dest_addr_ip4->sin_addr.s_addr = htonl(INADDR_ANY);
            dest_addr_ip4->sin_family = AF_INET;
            dest_addr_ip4->sin_port = htons(PORT);
            ip_protocol = IPPROTO_IP;
        } else if (addr_family == AF_INET6) {
            bzero(&dest_addr.sin6_addr.un, sizeof(dest_addr.sin6_addr.un));
            dest_addr.sin6_family = AF_INET6;
            dest_addr.sin6_port = htons(PORT);
            ip_protocol = IPPROTO_IPV6;
        }

        int sock = socket(addr_family, SOCK_DGRAM, ip_protocol);
        if (sock < 0) {
            ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
            break;
        }
        ESP_LOGI(TAG, "Socket created");

#if defined(CONFIG_LWIP_NETBUF_RECVINFO) && !defined(CONFIG_EXAMPLE_IPV6)
        int enable = 1;
        lwip_setsockopt(sock, IPPROTO_IP, IP_PKTINFO, &enable, sizeof(enable));
#endif

#if defined(CONFIG_EXAMPLE_IPV4) && defined(CONFIG_EXAMPLE_IPV6)
        if (addr_family == AF_INET6) {
            // Note that by default IPV6 binds to both protocols, it is must be disabled
            // if both protocols used at the same time (used in CI)
            int opt = 1;
            setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
            setsockopt(sock, IPPROTO_IPV6, IPV6_V6ONLY, &opt, sizeof(opt));
        }
#endif
        // Set timeout
        struct timeval timeout;
        timeout.tv_sec = 10;
        timeout.tv_usec = 0;
        setsockopt (sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof timeout);

        int err = bind(sock, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
        if (err < 0) {
            ESP_LOGE(TAG, "Socket unable to bind: errno %d", errno);
        }
        ESP_LOGI(TAG, "Socket bound, port %d", PORT);

        struct sockaddr_storage source_addr; // Large enough for both IPv4 or IPv6
        socklen_t socklen = sizeof(source_addr);

#if defined(CONFIG_LWIP_NETBUF_RECVINFO) && !defined(CONFIG_EXAMPLE_IPV6)
        struct iovec iov;
        struct msghdr msg;
        struct cmsghdr *cmsgtmp;
        u8_t cmsg_buf[CMSG_SPACE(sizeof(struct in_pktinfo))];

        iov.iov_base = rx_buffer;
        iov.iov_len = sizeof(rx_buffer);
        msg.msg_control = cmsg_buf;
        msg.msg_controllen = sizeof(cmsg_buf);
        msg.msg_flags = 0;
        msg.msg_iov = &iov;
        msg.msg_iovlen = 1;
        msg.msg_name = (struct sockaddr *)&source_addr;
        msg.msg_namelen = socklen;
#endif

        while (1) {
            ESP_LOGI(TAG, "Waiting for data");
#if defined(CONFIG_LWIP_NETBUF_RECVINFO) && !defined(CONFIG_EXAMPLE_IPV6)
            int len = recvmsg(sock, &msg, 0);
#else
            int len = recvfrom(sock, rx_buffer, sizeof(rx_buffer) - 1, 0, (struct sockaddr *)&source_addr, &socklen);
#endif
            // Error occurred during receiving
            if (len < 0) {
                ESP_LOGE(TAG, "recvfrom failed: errno %d", errno);
                break;
            }
            // Data received
            else {
                // Get the sender's ip address as string
                if (source_addr.ss_family == PF_INET) {
                    inet_ntoa_r(((struct sockaddr_in *)&source_addr)->sin_addr, addr_str, sizeof(addr_str) - 1);
#if defined(CONFIG_LWIP_NETBUF_RECVINFO) && !defined(CONFIG_EXAMPLE_IPV6)
                    for ( cmsgtmp = CMSG_FIRSTHDR(&msg); cmsgtmp != NULL; cmsgtmp = CMSG_NXTHDR(&msg, cmsgtmp) ) {
                        if ( cmsgtmp->cmsg_level == IPPROTO_IP && cmsgtmp->cmsg_type == IP_PKTINFO ) {
                            struct in_pktinfo *pktinfo;
                            pktinfo = (struct in_pktinfo*)CMSG_DATA(cmsgtmp);
                            ESP_LOGI(TAG, "dest ip: %s", inet_ntoa(pktinfo->ipi_addr));
                        }
                    }
#endif
                } else if (source_addr.ss_family == PF_INET6) {
                    inet6_ntoa_r(((struct sockaddr_in6 *)&source_addr)->sin6_addr, addr_str, sizeof(addr_str) - 1);
                }

                rx_buffer[len] = 0; // Null-terminate whatever we received and treat like a string...
                ESP_LOGI(TAG, "Received %d bytes from %s:", len, addr_str);
                control[0] = *rx_buffer;



                ESP_LOGI(TAG, "%s", rx_buffer);

                int err = sendto(sock, rx_buffer, len, 0, (struct sockaddr *)&source_addr, sizeof(source_addr));
                if (err < 0) {
                    ESP_LOGE(TAG, "Error occurred during sending: errno %d", errno);
                    break;
                }
            }
        }

        if (sock != -1) {
            ESP_LOGE(TAG, "Shutting down socket and restarting...");
            shutdown(sock, 0);
            close(sock);
        }
    }
    vTaskDelete(NULL);
}
/// UDP End

void app_main(void)
{
    ultrasonic_setup();

    xTaskCreate(rotate_wheels_task, "rotate_wheels_task", 4096, NULL, 5, NULL);
    xTaskCreate(run_motor_task, "run_motor_task", 4096, NULL, 5, NULL);
    xTaskCreate(adc_task, "adc_task", 2048, NULL, 5, NULL);
    xTaskCreate(timerTask, "timmerTask", 2048, NULL, 1, NULL);
    xTaskCreate(ultrasonic_task, "ultrasonic_task", 2048, NULL, 1, NULL);
    xTaskCreate(ultrasonic_task2, "ultrasonic_task2", 2048, NULL, 1, NULL);


    // Lidar
    i2c_master_init();
    xTaskCreate(printDistance, "printDistance", 4096, NULL, 5, NULL);
    xTaskCreate(calcDistance, "calcDistance", 4096, NULL, 5, NULL);
    i2c_scanner();
    // Lidar

    // UDP
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    ESP_ERROR_CHECK(example_connect());

    #ifdef CONFIG_EXAMPLE_IPV4
        xTaskCreate(udp_server_task, "udp_server", 4096, (void*)AF_INET, 5, NULL);
    #endif
    #ifdef CONFIG_EXAMPLE_IPV6
        xTaskCreate(udp_server_task, "udp_server", 4096, (void*)AF_INET6, 5, NULL);
    #endif
    // UDP

    // Timer
    init(); // Initialize the timer and GPIO
    xTaskCreate(char_to_display_task, "char_to_display_task", 2048, NULL, 10, NULL);
    xTaskCreate(test_alpha_display, "test_alpha_display", 4096, NULL, 5, NULL);
    xTaskCreate(button_task, "button_task", 2048, NULL, 10, NULL);

    // Timer End
}