#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/mcpwm_prelude.h"
#include "driver/uart.h"
#include "esp_vfs_dev.h"
#include "driver/adc.h"

static const char *TAG = "example";
char control[] = " ";

int count = 0;
int previousColor = 0;

#define DEFAULT_VREF 1100   // Default reference voltage (in mV) for ADC
#define ADC_UNIT ADC_UNIT_1 // ADC unit
#define ADC_CHANNEL ADC1_CHANNEL_6
#define ADC_WIDTH ADC_WIDTH_BIT_12 // ADC bit width
#define ADC_ATTEN ADC_ATTEN_DB_11  // ADC attenuation

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

static inline uint32_t example_angle_to_compare(int angle)
{
    return (angle - SERVO_MIN_DEGREE) * (SERVO_MAX_PULSEWIDTH_US - SERVO_MIN_PULSEWIDTH_US) / (SERVO_MAX_DEGREE - SERVO_MIN_DEGREE) + SERVO_MIN_PULSEWIDTH_US;
}

// void calibrateESC()
// {
//     mcpwm_set_duty_in_us(0, 0, MCPWM_OPR_B, 1500); // NEUTRAL signal in microseconds
//     vTaskDelay(pdMS_TO_TICKS(3100));               // Do for at least 3s, and leave in neutral state
// }

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
            angle = 20;
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
            angle = -20;
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

    int angle = 0;
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
            angle = 10;
        }
        else if (control[0] == 'b')
        {
            firstControl = 0;
            // inReverse = 0;
            angle = 0;
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
            angle = -10;
        }
        ESP_ERROR_CHECK(mcpwm_comparator_set_compare_value(comparator, example_angle_to_compare(angle)));
        // Add delay, since it takes time for servo to rotate, usually 200ms/60degree rotation under 5V power supply
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

void read_control_task()
{
    char input;
    while (1)
    {
        printf("Control: ");
        if (scanf(" %c", &input) == 1)
        {
            control[0] = input;
            printf("%c\n", control[0]);
        }
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

int returnColor(uint16_t adc_value)
{
    int color;
    if (adc_value < 300)
    {
        color = 1;
    }
    else
    {
        color = 0;
    }

    if (previousColor == 0 && color)
    {
        count++;
    }
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

        float rotationPerSec = nedEdges * (1.0 / 6.0);

        float inchesPerSecond = rotationPerSec * (8);

        printf("The inches per sec is: %f\n", inchesPerSecond);
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

void app_main(void)
{
    gpio_reset_pin(34);
    gpio_set_direction(34, GPIO_MODE_INPUT);

    ESP_ERROR_CHECK(uart_driver_install(UART_NUM_0,
                                        256, 0, 0, NULL, 0));
    esp_vfs_dev_uart_use_driver(UART_NUM_0);

    xTaskCreate(rotate_wheels_task, "rotate_wheels_task", 4096, NULL, 5, NULL);
    xTaskCreate(run_motor_task, "run_motor_task", 4096, NULL, 5, NULL);
    xTaskCreate(read_control_task, "read_control_task", 4096, NULL, 5, NULL);
    xTaskCreate(adc_task, "adc_task", 2048, NULL, 5, NULL);
    xTaskCreate(timerTask, "timmerTask", 2048, NULL, 1, NULL);
}