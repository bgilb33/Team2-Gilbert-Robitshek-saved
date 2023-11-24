#include <stdio.h>
#include <string.h>
#include <stdlib.h>   // Added in 2023..
#include <inttypes.h> // Added in 2023
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h" // Added in 2023
#include "esp_log.h"
#include "esp_system.h"    // Added in 2023
#include "driver/rmt_tx.h" // Modified in 2023
#include "soc/rmt_reg.h"   // Not needed?
#include "driver/uart.h"
#include "driver/gptimer.h"       // Added in 2023
#include "driver/gpio.h"          // Added in 2023
#include "driver/mcpwm_prelude.h" // Added in 2023
#include "driver/ledc.h"          // Added in 2023

// MCPWM defintions -- 2023: modified
#define MCPWM_TIMER_RESOLUTION_HZ 10000000 // 10MHz, 1 tick = 0.1us
#define MCPWM_FREQ_HZ 38000                // 38KHz PWM -- 1/38kHz = 26.3us
#define MCPWM_FREQ_PERIOD 263              // 263 ticks = 263 * 0.1us = 26.3us
#define MCPWM_GPIO_NUM 25

// UART definitions -- 2023: no changes
#define UART_TX_GPIO_NUM 26 // A0
#define UART_RX_GPIO_NUM 34 // A2
#define BUF_SIZE (1024)
#define BUF_SIZE2 (32)

// LED Output pins definitions -- 2023: minor changes
#define YELLOWPIN 14
#define GREENPIN 32
#define REDPIN 15

// Variables for my ID, minVal and status plus string fragments
char start = 0x1B; // START BYTE that UART looks for
int len_out = 3;
int fobID = -1;    // IF FOB ID == -1, no fob connected
int led_state = 0; // 0: NONE, 1: YELLOW, 2: GREEN, 3: RED

// Mutex (for resources), and Queues (for button)
SemaphoreHandle_t mux = NULL;               // 2023: no changes
static QueueHandle_t gpio_evt_queue = NULL; // 2023: Changed
// static xQueueHandle_t timer_queue; -- 2023: removed

// A simple structure to pass "events" to main task -- 2023: modified
typedef struct
{
    uint64_t event_count;
} example_queue_element_t;

// Create a FIFO queue for timer-based events -- Modified
example_queue_element_t ele;
QueueHandle_t timer_queue;

// System tags for diagnostics -- 2023: modified
// static const char *TAG_SYSTEM = "ec444: system";       // For debug logs

// Checksum -- 2023: no changes
char genCheckSum(char *p, int len)
{
    char temp = 0;
    for (int i = 0; i < len; i++)
    {
        temp = temp ^ p[i];
    }
    // printf("%X\n",temp);  // Diagnostic

    return temp;
}
bool checkCheckSum(uint8_t *p, int len)
{
    char temp = (char)0;
    bool isValid;
    for (int i = 0; i < len - 1; i++)
    {
        temp = temp ^ p[i];
    }
    // printf("Check: %02X ", temp); // Diagnostic
    if (temp == p[len - 1])
    {
        isValid = true;
    }
    else
    {
        isValid = false;
    }
    return isValid;
}

// MCPWM Initialize -- 2023: this is to create 38kHz carrier
static void pwm_init()
{

    // Create timer
    mcpwm_timer_handle_t pwm_timer = NULL;
    mcpwm_timer_config_t pwm_timer_config = {
        .group_id = 0,
        .clk_src = MCPWM_TIMER_CLK_SRC_DEFAULT,
        .resolution_hz = MCPWM_TIMER_RESOLUTION_HZ,
        .period_ticks = MCPWM_FREQ_PERIOD,
        .count_mode = MCPWM_TIMER_COUNT_MODE_UP,
    };
    ESP_ERROR_CHECK(mcpwm_new_timer(&pwm_timer_config, &pwm_timer));

    // Create operator
    mcpwm_oper_handle_t oper = NULL;
    mcpwm_operator_config_t operator_config = {
        .group_id = 0, // operator must be in the same group to the timer
    };
    ESP_ERROR_CHECK(mcpwm_new_operator(&operator_config, &oper));

    // Connect timer and operator
    ESP_ERROR_CHECK(mcpwm_operator_connect_timer(oper, pwm_timer));

    // Create comparator from the operator
    mcpwm_cmpr_handle_t comparator = NULL;
    mcpwm_comparator_config_t comparator_config = {
        .flags.update_cmp_on_tez = true,
    };
    ESP_ERROR_CHECK(mcpwm_new_comparator(oper, &comparator_config, &comparator));

    // Create generator from the operator
    mcpwm_gen_handle_t generator = NULL;
    mcpwm_generator_config_t generator_config = {
        .gen_gpio_num = MCPWM_GPIO_NUM,
    };
    ESP_ERROR_CHECK(mcpwm_new_generator(oper, &generator_config, &generator));

    // set the initial compare value, so that the duty cycle is 50%
    ESP_ERROR_CHECK(mcpwm_comparator_set_compare_value(comparator, 132));
    // CANNOT FIGURE OUT HOW MANY TICKS TO COMPARE TO TO GET 50%

    // Set generator action on timer and compare event
    // go high on counter empty
    ESP_ERROR_CHECK(mcpwm_generator_set_action_on_timer_event(generator,
                                                              MCPWM_GEN_TIMER_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP, MCPWM_TIMER_EVENT_EMPTY, MCPWM_GEN_ACTION_HIGH)));
    // go low on compare threshold
    ESP_ERROR_CHECK(mcpwm_generator_set_action_on_compare_event(generator,
                                                                MCPWM_GEN_COMPARE_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP, comparator, MCPWM_GEN_ACTION_LOW)));

    // Enable and start timer
    ESP_ERROR_CHECK(mcpwm_timer_enable(pwm_timer));
    ESP_ERROR_CHECK(mcpwm_timer_start_stop(pwm_timer, MCPWM_TIMER_START_NO_STOP));
}

// Configure UART -- 2023: minor changes
static void uart_init()
{
    // Basic configs
    const uart_config_t uart_config = {
        .baud_rate = 1200, // Slow BAUD rate
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT};
    uart_param_config(UART_NUM_1, &uart_config);

    // Set UART pins using UART0 default pins
    uart_set_pin(UART_NUM_1, UART_TX_GPIO_NUM, UART_RX_GPIO_NUM, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);

    // Reverse receive logic line
    uart_set_line_inverse(UART_NUM_1, UART_SIGNAL_RXD_INV);

    // Install UART driver
    uart_driver_install(UART_NUM_1, BUF_SIZE * 2, 0, 0, NULL, 0);
}

// GPIO init for LEDs -- 2023: modified
static void led_init()
{
    gpio_reset_pin(YELLOWPIN);
    gpio_set_direction(YELLOWPIN, GPIO_MODE_OUTPUT);
    gpio_reset_pin(REDPIN);
    gpio_set_direction(REDPIN, GPIO_MODE_OUTPUT);
    gpio_reset_pin(GREENPIN);
    gpio_set_direction(GREENPIN, GPIO_MODE_OUTPUT);
}

// Receive task -- looks for Start byte then stores received values -- 2023: minor changes
void recv_task()
{
    // Buffer for input data
    // Receiver expects message to be sent multiple times
    uint8_t *data_in = (uint8_t *)malloc(BUF_SIZE2);
    while (1)
    {
        int len_in = uart_read_bytes(UART_NUM_1, data_in, BUF_SIZE2, 100 / portTICK_PERIOD_MS);
        if (len_in > 10)
        {
            int nn = 0;
            // ESP_LOGE(TAG_UART, "Length greater than 10");
            while (data_in[nn] != start)
            {
                nn++;
            }
            uint8_t copied[len_out];
            memcpy(copied, data_in + nn, len_out * sizeof(uint8_t));
            // printf("before checksum");
            // ESP_LOG_BUFFER_HEXDUMP("DATA IN", copied, len_out, ESP_LOG_INFO);
            if (checkCheckSum(copied, len_out))
            {
                // printf("after checksum\n");
                if (copied[1] != fobID)
                {
                    fobID = copied[1];
                    led_state = 1;
                }
                printf("FOB ID: %i\n", fobID);
                uart_flush(UART_NUM_1);
            }
        }
        else
        {
            // printf("Nothing received.\n");
        }
        // vTaskDelay(5 / portTICK_PERIOD_MS);
    }
    free(data_in);
}

void led_task()
{
    while (1)
    {
        // printf("LED STATE: %i\n", led_state);
        if (led_state == 0)
        {
            gpio_set_level(REDPIN, 0);
            gpio_set_level(YELLOWPIN, 0);
            gpio_set_level(GREENPIN, 0);
        }
        else if (led_state == 1)
        {
            gpio_set_level(REDPIN, 0);
            gpio_set_level(YELLOWPIN, 1);
            gpio_set_level(GREENPIN, 0);
        }
        else if (led_state == 2)
        {
            gpio_set_level(REDPIN, 0);
            gpio_set_level(YELLOWPIN, 0);
            gpio_set_level(GREENPIN, 1);
        }
        else if (led_state == 3)
        {
            gpio_set_level(REDPIN, 1);
            gpio_set_level(YELLOWPIN, 0);
            gpio_set_level(GREENPIN, 0);
        }
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}

void app_main()
{

    // Mutex for current values when sending -- no changes
    mux = xSemaphoreCreateMutex();

    // Timer queue initialize -- 2023: modified
    timer_queue = xQueueCreate(10, sizeof(example_queue_element_t));

    // Initialize all the things -- no changes
    // rmt_tx_init();
    uart_init();
    led_init();
    pwm_init();

    // Create tasks for receive, send, set gpio, and button -- 2023 -- no changes

    xTaskCreate(recv_task, "uart_rx_task", 1024 * 4, NULL, configMAX_PRIORITIES, NULL);
    xTaskCreate(led_task, "led_task", 1024 * 4, NULL, configMAX_PRIORITIES, NULL);

    while (1)
    {
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}