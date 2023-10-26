#include <stdio.h>
#include <math.h>
#include <time.h>
#include "driver/i2c.h"
#include "./ADXL343.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/adc.h"
#include "esp_adc_cal.h"
#include <time.h>
#include "driver/timer.h"
#include "driver/periph_ctrl.h"
#include "driver/gpio.h"
#include <string.h>
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "lwip/err.h"
#include "lwip/sys.h"
#include "esp_sntp.h"
#include "esp_netif.h"'

//noah code
#include <string.h>
#include <sys/param.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "protocol_examples_common.h"

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include <lwip/netdb.h>


#include <stdio.h>
#include "driver/gpio.h"


// benji code
#include <stdio.h>
#include <string.h>
#include "driver/uart.h"
#include "esp_vfs_dev.h" 
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
////

#define LED_PIN GPIO_NUM_2

#ifdef CONFIG_EXAMPLE_SOCKET_IP_INPUT_STDIN
#include "addr_from_stdin.h"
#endif

#if defined(CONFIG_EXAMPLE_IPV4)
#define HOST_IP_ADDR CONFIG_EXAMPLE_IPV4_ADDR
#elif defined(CONFIG_EXAMPLE_IPV6)
#define HOST_IP_ADDR CONFIG_EXAMPLE_IPV6_ADDR
#else
#define HOST_IP_ADDR ""
#endif

#define PORT CONFIG_EXAMPLE_PORT

static const char *TAG = "example";
static const char *payload = "Hello this is the ESP32 requesting the blink time";


// Wifi Defines
#define EXAMPLE_ESP_WIFI_SSID "Group_2"
#define EXAMPLE_ESP_WIFI_PASS "smartsys"
#define EXAMPLE_ESP_MAXIMUM_RETRY 5
#define ESP_WIFI_SAE_MODE WPA3_SAE_PWE_BOTH
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA2_PSK
#define EXAMPLE_H2E_IDENTIFIER ""
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT BIT1
static EventGroupHandle_t s_wifi_event_group;
static int s_retry_num = 0;
esp_netif_ip_info_t ip_info;

// Master I2C
#define I2C_EXAMPLE_MASTER_SCL_IO 22        // gpio number for i2c clk
#define I2C_EXAMPLE_MASTER_SDA_IO 23        // gpio number for i2c data
#define I2C_EXAMPLE_MASTER_NUM I2C_NUM_0    // i2c port
#define I2C_EXAMPLE_MASTER_TX_BUF_DISABLE 0 // i2c master no buffer needed
#define I2C_EXAMPLE_MASTER_RX_BUF_DISABLE 0 // i2c master no buffer needed
#define I2C_EXAMPLE_MASTER_FREQ_HZ 100000   // i2c master clock freq
#define WRITE_BIT I2C_MASTER_WRITE          // i2c master write
#define READ_BIT I2C_MASTER_READ            // i2c master read
#define ACK_CHECK_EN true                   // i2c master will check ack
#define ACK_CHECK_DIS false                 // i2c master will not check ack
#define ACK_VAL 0x00                        // i2c ack value
#define NACK_VAL 0x1                        // i2c nack value

// ADXL343
#define SLAVE_ADDR ADXL343_ADDRESS // 0x53

double magnitude;
int step_count = 0; // Counter for step count
bool wait;
int stepped = 0;
int waitCount = 0;
int mode = 2;

int timer_state = 0;    // 0: off, 1: on
int activity_ended = 0; // 0: did not just end, 1: just ended

time_t now;
struct tm timeInfo;
int splitTime[2];

// Display Defs
#define SLAVE_ADDR_DISPLAY 0x70      // alphanumeric address
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

uint16_t displaybuffer[3];

// Timer Defines
#define GPIO_INPUT_IO_1 4
#define ESP_INTR_FLAG_DEFAULT 0
#define GPIO_INPUT_PIN_SEL 1ULL << GPIO_INPUT_IO_1
#define GPT_TIMER_GROUP TIMER_GROUP_0
#define GPT_TIMER_INDEX TIMER_0
#define GPT_INTERVAL_MS 1000 // Interval in milliseconds

double start_time = 0.00;
double *current_time = &start_time;
int flag = 0;
int display_mode = 0; // 0: Clock, 1: Timer

// Temp Defines

#define DEFAULT_VREF 1100 // Use adc2_vref_to_gpio() to obtain a better estimate
#define NO_OF_SAMPLES 64  // Multisampling
#define TEMP_ALARM_PIN 13
static esp_adc_cal_characteristics_t *adc_chars;
static const adc1_channel_t thermistor_channel = ADC1_CHANNEL_6;
static const adc_atten_t atten = ADC_ATTEN_DB_11;
static const adc_unit_t unit = ADC_UNIT_1;
float temp;

// Priorities
#define ADXL343_TASK_PRIORITY 5
#define COUNTING_TASK_PRIORITY 4
#define TEMP_TASK_PRIORITY 4
#define PRINT_TASK_PRIORITY 4
#define CHAR_TO_DISPLAY_TASK_PRIORITY 10
#define ALPHA_DISPLAY_TASK_PRIORITY 2
#define BUTTON_TASK_PRIORITY 2

void init()
{
    // Setup based on code from Console I/O Example from the skill assignment
    /* Install UART driver for interrupt-driven reads and writes */
    ESP_ERROR_CHECK(uart_driver_install(UART_NUM_0,
                                        256, 0, 0, NULL, 0));

    /* Tell VFS to use UART driver */
    esp_vfs_dev_uart_use_driver(UART_NUM_0);
    gpio_reset_pin(13);
    gpio_set_direction(13, GPIO_MODE_OUTPUT);
}

// Wifi Event Handler
static void event_handler(void *arg, esp_event_base_t event_base,
                          int32_t event_id, void *event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
    {
        esp_wifi_connect();
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
    {
        if (s_retry_num < EXAMPLE_ESP_MAXIMUM_RETRY)
        {
            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGI(TAG, "retry to connect to the AP");
        }
        else
        {
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
        }
        ESP_LOGI(TAG, "connect to the AP fail");
    }
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
    {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

// Wifi Init

void wifi_init_sta(void)
{
    s_wifi_event_group = xEventGroupCreate();

    ESP_ERROR_CHECK(esp_netif_init());

    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_got_ip));

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = EXAMPLE_ESP_WIFI_SSID,
            .password = EXAMPLE_ESP_WIFI_PASS,
            /* Authmode threshold resets to WPA2 as default if password matches WPA2 standards (pasword len => 8).
             * If you want to connect the device to deprecated WEP/WPA networks, Please set the threshold value
             * to WIFI_AUTH_WEP/WIFI_AUTH_WPA_PSK and set the password with length and format matching to
             * WIFI_AUTH_WEP/WIFI_AUTH_WPA_PSK standards.
             */
            .threshold.authmode = ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD,
            .sae_pwe_h2e = ESP_WIFI_SAE_MODE,
            .sae_h2e_identifier = EXAMPLE_H2E_IDENTIFIER,
        },
    };
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "wifi_init_sta finished.");

    /* Waiting until either the connection is established (WIFI_CONNECTED_BIT) or connection failed for the maximum
     * number of re-tries (WIFI_FAIL_BIT). The bits are set by event_handler() (see above) */
    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
                                           WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
                                           pdFALSE,
                                           pdFALSE,
                                           portMAX_DELAY);

    /* xEventGroupWaitBits() returns the bits before the call returned, hence we can test which event actually
     * happened. */
    if (bits & WIFI_CONNECTED_BIT)
    {
        ESP_LOGI(TAG, "connected to ap SSID:%s password:%s",
                 EXAMPLE_ESP_WIFI_SSID, EXAMPLE_ESP_WIFI_PASS);
    }
    else if (bits & WIFI_FAIL_BIT)
    {
        ESP_LOGI(TAG, "Failed to connect to SSID:%s, password:%s",
                 EXAMPLE_ESP_WIFI_SSID, EXAMPLE_ESP_WIFI_PASS);
    }
    else
    {
        ESP_LOGE(TAG, "UNEXPECTED EVENT");
    }
}

static void IRAM_ATTR gpio_isr_handler(void *arg)
{
    flag = 1;
    timer_state = ~timer_state;
}
// Function to initiate i2c -- note the MSB declaration!
static void i2c_master_init()
{
    // Debug
    // printf("\n>> i2c Config\n");
    int err;

    // Port configuration
    int i2c_master_port = I2C_EXAMPLE_MASTER_NUM;

    /// Define I2C configurations
    i2c_config_t conf;
    conf.mode = I2C_MODE_MASTER;                        // Master mode
    conf.sda_io_num = I2C_EXAMPLE_MASTER_SDA_IO;        // Default SDA pin
    conf.sda_pullup_en = GPIO_PULLUP_ENABLE;            // Internal pullup
    conf.scl_io_num = I2C_EXAMPLE_MASTER_SCL_IO;        // Default SCL pin
    conf.scl_pullup_en = GPIO_PULLUP_ENABLE;            // Internal pullup
    conf.master.clk_speed = I2C_EXAMPLE_MASTER_FREQ_HZ; // CLK frequency
    conf.clk_flags = 0;
    err = i2c_param_config(i2c_master_port, &conf); // Configure
    if (err == ESP_OK)
    {
        printf("- parameters: ok\n");
    }

    // Install I2C driver
    err = i2c_driver_install(i2c_master_port, conf.mode,
                             I2C_EXAMPLE_MASTER_RX_BUF_DISABLE,
                             I2C_EXAMPLE_MASTER_TX_BUF_DISABLE, 0);
    if (err == ESP_OK)
    {
        // printf("- initialized: yes\n");
    }

    // Data in MSB mode
    i2c_set_data_mode(i2c_master_port, I2C_DATA_MODE_MSB_FIRST, I2C_DATA_MODE_MSB_FIRST);
}

////////DISPLAY FUNCTIONS///////////////////////////////////////////

int alpha_oscillator()
{
    int ret;
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (SLAVE_ADDR_DISPLAY << 1) | WRITE_BIT, ACK_CHECK_EN);
    i2c_master_write_byte(cmd, OSC, ACK_CHECK_EN);
    i2c_master_stop(cmd);
    ret = i2c_master_cmd_begin(I2C_EXAMPLE_MASTER_NUM, cmd, 1000 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);
    vTaskDelay(200 / portTICK_PERIOD_MS);
    return ret;
}

int no_blink()
{
    int ret;
    i2c_cmd_handle_t cmd2 = i2c_cmd_link_create();
    i2c_master_start(cmd2);
    i2c_master_write_byte(cmd2, (SLAVE_ADDR_DISPLAY << 1) | WRITE_BIT, ACK_CHECK_EN);
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
    i2c_master_write_byte(cmd3, (SLAVE_ADDR_DISPLAY << 1) | WRITE_BIT, ACK_CHECK_EN);
    i2c_master_write_byte(cmd3, HT16K33_CMD_BRIGHTNESS | val, ACK_CHECK_EN);
    i2c_master_stop(cmd3);
    ret = i2c_master_cmd_begin(I2C_EXAMPLE_MASTER_NUM, cmd3, 1000 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd3);
    vTaskDelay(200 / portTICK_PERIOD_MS);
    return ret;
}

void char_to_display_task()
{

    int digits[3];
    int integerPart;
    int fractionalPart;
    while (1)
    {
        // From GPT
        if (timer_state)
        {
            integerPart = (int)*current_time % 100;
            fractionalPart = (int)((*current_time - integerPart) * 100);
        }
        else
        {
            integerPart = splitTime[0];
            fractionalPart = splitTime[1];
        }
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
    // if (ret == ESP_OK)
    // {
    //     printf("- oscillator: ok \n");
    // }
    // Set display blink off
    ret = no_blink();
    // if (ret == ESP_OK)
    // {
    //     printf("- blink: off \n");
    // }
    ret = set_brightness_max(0xF);
    // if (ret == ESP_OK)
    // {
    //     printf("- brightness: max \n");
    // }
    // Continually writes the same command

    while (1)
    {

        for (int i = 0; i < 100; i++)
        {
            // Send commands characters to display over I2C
            i2c_cmd_handle_t cmd4 = i2c_cmd_link_create();
            i2c_master_start(cmd4);
            i2c_master_write_byte(cmd4, (SLAVE_ADDR_DISPLAY << 1) | WRITE_BIT, ACK_CHECK_EN);
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

////////////////////////////////TIMER FUNCTIONS////////////////////////////////

void timer_initial()
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
    // Attach the interrupt handler to the GPIO pin
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
    while (1)
    {
        if (!timer_state)
        {
            timer_pause(GPT_TIMER_GROUP, GPT_TIMER_INDEX);
            *current_time = 0.00;
            if (!activity_ended)
            {
                // printf("Activity Ended\n");
                printf("STATUS: " IPSTR ",Activity Ended\n", IP2STR(&ip_info.ip));
                activity_ended = 1;
            }
        }
        else if (flag)
        {
            activity_ended = 0;
            step_count = 0;
            printf("STATUS: " IPSTR ",In Activity\n", IP2STR(&ip_info.ip));
            current_time = &start_time;
            timer_pause(GPT_TIMER_GROUP, GPT_TIMER_INDEX);
            timer_set_counter_value(GPT_TIMER_GROUP, GPT_TIMER_INDEX, 0);
            timer_start(GPT_TIMER_GROUP, GPT_TIMER_INDEX);
            flag = 0; // Reset the flag
        }

        timer_get_counter_time_sec(GPT_TIMER_GROUP, GPT_TIMER_INDEX, current_time);
        *current_time = roundf(*current_time * 100) / 100; // Set to two decimal place

        vTaskDelay(10 / portTICK_PERIOD_MS); // Delay for 1 second
    }
}

void updateTime_task()
{
    setenv("TZ", "America/New_York", 1);
    tzset(); // Update the timezone information
    while (1)
    {
        // GPT
        time(&now);
        localtime_r(&now, &timeInfo);
        if (timeInfo.tm_hour >= 0 && timeInfo.tm_hour < 4)
        {
            splitTime[0] = timeInfo.tm_hour + 20;
        }
        else
        {
            splitTime[0] = timeInfo.tm_hour - 4;
        }
        splitTime[1] = timeInfo.tm_min;
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

/////////////////////////TEMP FUNCTIONS/////////////////////////////
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

void temp_init()
{
    gpio_reset_pin(TEMP_ALARM_PIN);
    gpio_set_direction(TEMP_ALARM_PIN, GPIO_MODE_OUTPUT);
    check_efuse();
    if (unit == ADC_UNIT_1)
    {
        adc1_config_width(ADC_WIDTH_BIT_12);
        adc1_config_channel_atten(thermistor_channel, atten);
    }
    else
    {
        adc2_config_channel_atten((adc2_channel_t)thermistor_channel, atten);
    }
    adc_chars = calloc(1, sizeof(esp_adc_cal_characteristics_t));
    esp_adc_cal_value_t val_type = esp_adc_cal_characterize(unit, atten, ADC_WIDTH_BIT_12, DEFAULT_VREF, adc_chars);
    print_char_val_type(val_type);
}

void temp_task()
{
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
        temp = (1.0 / 298.15) + ((1.0 / 3435.0) * (log(resistance / 10000.0)));
        temp = (1.0 / temp) - 271.1;

        if (temp >= 27.00 && timer_state)
        {
            gpio_set_level(TEMP_ALARM_PIN, 1);
        }
        else
        {
            gpio_set_level(TEMP_ALARM_PIN, 0);
        }

        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}

// I2C Utility  Functions //////////////////////////////////////////////////////////

// Utility function to test for I2C device address -- not used in deploy
int testConnection(uint8_t devAddr, int32_t timeout)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (devAddr << 1) | I2C_MASTER_WRITE, ACK_CHECK_EN);
    i2c_master_stop(cmd);
    int err = i2c_master_cmd_begin(I2C_EXAMPLE_MASTER_NUM, cmd, 1000 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);
    return err;
}

// Utility function to scan for i2c device
static void i2c_scanner()
{
    int32_t scanTimeout = 1000;
    // printf("\n>> I2C scanning ..."
    //        "\n");
    uint8_t count = 0;
    for (uint8_t i = 1; i < 127; i++)
    {
        // printf("0x%X%s",i,"\n");
        if (testConnection(i, scanTimeout) == ESP_OK)
        {
            // printf("- Device found at address: 0x%X%s", i, "\n");
            count++;
        }
    }
    if (count == 0)
    {
        // printf("- No I2C devices found!"
        //        "\n");
    }
}

// ADXL343 Functions ///////////////////////////////////////////////////////////

// Get Device ID
int getDeviceID(uint8_t *data)
{
    int ret;
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (SLAVE_ADDR << 1) | WRITE_BIT, ACK_CHECK_EN);
    i2c_master_write_byte(cmd, ADXL343_REG_DEVID, ACK_CHECK_EN);
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (SLAVE_ADDR << 1) | READ_BIT, ACK_CHECK_EN);
    i2c_master_read_byte(cmd, data, ACK_CHECK_DIS);
    i2c_master_stop(cmd);
    ret = i2c_master_cmd_begin(I2C_EXAMPLE_MASTER_NUM, cmd, 1000 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);
    return ret;
}

int writeRegister(uint8_t reg, uint8_t data)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    // Indicate write before sending register
    i2c_master_write_byte(cmd, (SLAVE_ADDR << 1) | WRITE_BIT, ACK_CHECK_EN);
    // Send register address to the device
    i2c_master_write_byte(cmd, reg, ACK_CHECK_EN);
    // Write data
    i2c_master_write_byte(cmd, data, ACK_CHECK_EN);
    i2c_master_stop(cmd);
    int ret = i2c_master_cmd_begin(I2C_EXAMPLE_MASTER_NUM, cmd, 1000 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);
    return ret;
}

// Read register
uint8_t readRegister(uint8_t reg)
{
    uint8_t data;

    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);

    // Read First Byte
    // Indicate write before sending register
    i2c_master_write_byte(cmd, (SLAVE_ADDR << 1) | WRITE_BIT, ACK_CHECK_EN);
    // Send register address to device
    i2c_master_write_byte(cmd, reg, ACK_CHECK_EN);
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (SLAVE_ADDR << 1) | READ_BIT, ACK_CHECK_EN);
    i2c_master_read_byte(cmd, &data, I2C_MASTER_ACK);
    i2c_master_stop(cmd);
    int check = i2c_master_cmd_begin(I2C_EXAMPLE_MASTER_NUM, cmd, 1000 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);

    if (check == ESP_OK)
    {
        return data;
    }
    else
    {
        return 0;
    }
}

// read 16 bits (2 bytes)
int16_t read16(uint8_t reg)
{
    uint8_t data_low, data_high;

    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    // Indicate write before sending register
    i2c_master_write_byte(cmd, (SLAVE_ADDR << 1) | WRITE_BIT, ACK_CHECK_EN);
    // Send register address to device
    i2c_master_write_byte(cmd, reg, ACK_CHECK_EN);
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (SLAVE_ADDR << 1) | READ_BIT, ACK_CHECK_EN);
    i2c_master_read_byte(cmd, &data_low, ACK_VAL);
    i2c_master_read_byte(cmd, &data_high, NACK_VAL);
    i2c_master_stop(cmd);
    int check = i2c_master_cmd_begin(I2C_EXAMPLE_MASTER_NUM, cmd, 1000 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);

    if (check == ESP_OK)
    {
        int16_t data = (int16_t)((data_high << 8) | data_low);
        return data;
    }
    else
    {
        return 0;
    }
}

void setRange(range_t range)
{
    /* Red the data format register to preserve bits */
    uint8_t format = readRegister(ADXL343_REG_DATA_FORMAT);

    /* Update the data rate */
    format &= ~0x0F;
    format |= range;

    /* Make sure that the FULL-RES bit is enabled for range scaling */
    format |= 0x08;

    /* Write the register back to the IC */
    writeRegister(ADXL343_REG_DATA_FORMAT, format);
}

range_t getRange(void)
{
    /* Red the data format register to preserve bits */
    return (range_t)(readRegister(ADXL343_REG_DATA_FORMAT) & 0x03);
}

dataRate_t getDataRate(void)
{
    return (dataRate_t)(readRegister(ADXL343_REG_BW_RATE) & 0x0F);
}

// function to get acceleration
void getAccel(float *xp, float *yp, float *zp)
{
    *xp = read16(ADXL343_REG_DATAX0) * ADXL343_MG2G_MULTIPLIER * SENSORS_GRAVITY_STANDARD;
    *yp = read16(ADXL343_REG_DATAY0) * ADXL343_MG2G_MULTIPLIER * SENSORS_GRAVITY_STANDARD;
    *zp = read16(ADXL343_REG_DATAZ0) * ADXL343_MG2G_MULTIPLIER * SENSORS_GRAVITY_STANDARD;
    // printf("X: %.2f \t Y: %.2f \t Z: %.2f\n", *xp, *yp, *zp);
}

// function to print roll and pitch
void calcRP(float x, float y, float z)
{
    float roll, pitch;
    roll = atan2(y, z) * 57.3;
    pitch = atan2((-x), sqrt(y * y + z * z)) * 57.3;

    // printf("roll: %.2f \t pitch: %.2f \n", roll, pitch);
}

double calcMagnitude(float x, float y, float z)
{
    double magnitudeSquared = (x * x) + (y * y) + (z * z);
    magnitude = sqrt(magnitudeSquared);
    // printf("%.2f\n", magnitude);
    return magnitude;
}

// Task to continuously poll acceleration and calculate roll and pitch
static void test_adxl343()
{
    // printf("\n>> Polling ADAXL343\n");
    while (1)
    {
        float xVal, yVal, zVal;
        getAccel(&xVal, &yVal, &zVal);
        calcRP(xVal, yVal, zVal);
        calcMagnitude(xVal, yVal, zVal);
        // printf("The step count is: %d\n", step_count);

        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}

static void counting()
{
    while (1)
    {
        switch (mode)
        {
        case 0:
            waitCount++;
            if (waitCount >= 10)
            {
                waitCount = 0;
                mode = 1;
            }
            break;
        default:
            if (magnitude >= 15)
            {
                step_count = step_count + 2;
                mode = 0;
            }
            break;
        }
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}

void print_task()
{
    while (1)
    {
        if (timer_state && fmod(*current_time, 10.0) == 0.0)
        {
            // printf("%.2f, %f, %d\n", *current_time, temp, step_count);
            printf("INFO: " IPSTR ",%f,%d\n", IP2STR(&ip_info.ip), temp, step_count);
        }
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}

static void udp_client_task(void *pvParameters)
{
    char rx_buffer[128];

    char host_ip[] = HOST_IP_ADDR;
    int addr_family = 0;
    int ip_protocol = 0;

    while (1) {

#if defined(CONFIG_EXAMPLE_IPV4)
        struct sockaddr_in dest_addr;
        dest_addr.sin_addr.s_addr = inet_addr(HOST_IP_ADDR);
        dest_addr.sin_family = AF_INET;
        dest_addr.sin_port = htons(PORT);
        addr_family = AF_INET;
        ip_protocol = IPPROTO_IP;
#elif defined(CONFIG_EXAMPLE_IPV6)
        struct sockaddr_in6 dest_addr = { 0 };
        inet6_aton(HOST_IP_ADDR, &dest_addr.sin6_addr);
        dest_addr.sin6_family = AF_INET6;
        dest_addr.sin6_port = htons(PORT);
        dest_addr.sin6_scope_id = esp_netif_get_netif_impl_index(EXAMPLE_INTERFACE);
        addr_family = AF_INET6;
        ip_protocol = IPPROTO_IPV6;
#elif defined(CONFIG_EXAMPLE_SOCKET_IP_INPUT_STDIN)
        struct sockaddr_storage dest_addr = { 0 };
        ESP_ERROR_CHECK(get_addr_from_stdin(PORT, SOCK_DGRAM, &ip_protocol, &addr_family, &dest_addr));
#endif

        int sock = socket(addr_family, SOCK_DGRAM, ip_protocol);
        if (sock < 0) {
            ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
            break;
        }

        // Set timeout
        struct timeval timeout;
        timeout.tv_sec = 10;
        timeout.tv_usec = 0;
        setsockopt (sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof timeout);

        ESP_LOGI(TAG, "Socket created, sending to %s:%d", HOST_IP_ADDR, PORT);

        if (timer_state && fmod(*current_time, 10.0) == 0.0)
        {
            // payload attachment

            char outputBuffer[100]; // Make sure the buffer is large enough to hold the formatted string
            sprintf(outputBuffer, "INFO: " IPSTR ",%f,%d\n", IP2STR(&ip_info.ip), temp, step_count);


            // This message below is the key message to send data
            int err = sendto(sock, outputBuffer, strlen(payload), 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
            
            if (err < 0) {
                ESP_LOGE(TAG, "Error occurred during sending: errno %d", errno);
                break;
            }
            ESP_LOGI(TAG, "Message sent");

            struct sockaddr_storage source_addr; // Large enough for both IPv4 or IPv6
            socklen_t socklen = sizeof(source_addr);
            int len = recvfrom(sock, rx_buffer, sizeof(rx_buffer) - 1, 0, (struct sockaddr *)&source_addr, &socklen);

            // Error occurred during receiving
            if (len < 0) {
                ESP_LOGE(TAG, "recvfrom failed: errno %d", errno);
                break;
            }

            // Data received
            else {
                rx_buffer[len] = 0; // Null-terminate whatever we received and treat like a string
                ESP_LOGI(TAG, "Received %d bytes from %s:", len, host_ip);
                ESP_LOGI(TAG, "%s", rx_buffer);
                printf("the buffer is: ");
                printf("%s\n", rx_buffer);

                if (strncmp(rx_buffer, "OK: ", 4) == 0) {
                    ESP_LOGI(TAG, "Received expected message, reconnecting");
                    break;
                }
            }

            printf("ok we are done now\n");
            // ESP_LOGE(TAG, "Shutting down socket and restarting...");
            shutdown(sock, 0);
            close(sock);

        }


    }
    vTaskDelete(NULL);
}



void app_main()
{
    init();
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    /* This helper function configures Wi-Fi or Ethernet, as selected in menuconfig.
     * Read "Establishing Wi-Fi or Ethernet Connection" section in
     * examples/protocols/README.md for more information about this function.
     */
    ESP_ERROR_CHECK(example_connect());

    xTaskCreate(udp_client_task, "udp_client", 4096, NULL, 5, NULL);

    // Setting up wifi
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_LOGI(TAG, "ESP_WIFI_MODE_STA");
    wifi_init_sta();

    // GPT: Get time
    esp_event_loop_create_default();

    // Initialize SNTP
    sntp_setoperatingmode(SNTP_OPMODE_POLL);
    sntp_setservername(0, "pool.ntp.org"); // Set the NTP server
    sntp_init();

    // Get IP
    esp_netif_get_ip_info(esp_netif_get_handle_from_ifkey("WIFI_STA_DEF"), &ip_info);
    printf("IP Address: " IPSTR "\n", IP2STR(&ip_info.ip));

    // Routine
    i2c_master_init();
    i2c_scanner();
    temp_init();
    timer_initial();

    // Check for ADXL343
    uint8_t deviceID;
    getDeviceID(&deviceID);
    if (deviceID == 0xE5)
    {
        // printf("\n>> Found ADAXL343\n");
    }

    // // Disable interrupts
    // writeRegister(ADXL343_REG_INT_ENABLE, 0);

    // Set range
    setRange(ADXL343_RANGE_16_G);
    // Display range
    // printf("- Range:         +/- ");
    switch (getRange())
    {
    case ADXL343_RANGE_16_G:
        // printf("16 ");
        break;
    case ADXL343_RANGE_8_G:
        // printf("8 ");
        break;
    case ADXL343_RANGE_4_G:
        // printf("4 ");
        break;
    case ADXL343_RANGE_2_G:
        // printf("2 ");
        break;
    default:
        // printf("?? ");
        break;
    }
    // printf(" g\n");

    // Display data rate
    // printf("- Data Rate:    ");
    switch (getDataRate())
    {
    case ADXL343_DATARATE_3200_HZ:
        // printf("3200 ");
        break;
    case ADXL343_DATARATE_1600_HZ:
        // printf("1600 ");
        break;
    case ADXL343_DATARATE_800_HZ:
        // printf("800 ");
        break;
    case ADXL343_DATARATE_400_HZ:
        // printf("400 ");
        break;
    case ADXL343_DATARATE_200_HZ:
        // printf("200 ");
        break;
    case ADXL343_DATARATE_100_HZ:
        // printf("100 ");
        break;
    case ADXL343_DATARATE_50_HZ:
        // printf("50 ");
        break;
    case ADXL343_DATARATE_25_HZ:
        // printf("25 ");
        break;
    case ADXL343_DATARATE_12_5_HZ:
        // printf("12.5 ");
        break;
    case ADXL343_DATARATE_6_25HZ:
        // printf("6.25 ");
        break;
    case ADXL343_DATARATE_3_13_HZ:
        // printf("3.13 ");
        break;
    case ADXL343_DATARATE_1_56_HZ:
        // printf("1.56 ");
        break;
    case ADXL343_DATARATE_0_78_HZ:
        // printf("0.78 ");
        break;
    case ADXL343_DATARATE_0_39_HZ:
        // printf("0.39 ");
        break;
    case ADXL343_DATARATE_0_20_HZ:
        // printf("0.20 ");
        break;
    case ADXL343_DATARATE_0_10_HZ:
        // printf("0.10 ");
        break;
    default:
        // printf("???? ");
        break;
    }
    // printf(" Hz\n\n");

    // Enable measurements
    writeRegister(ADXL343_REG_POWER_CTL, 0x08);

    // Create task to poll ADXL343
    xTaskCreate(test_adxl343, "test_adxl343", 4096, NULL, ADXL343_TASK_PRIORITY, NULL);
    xTaskCreate(counting, "counting", 4096, NULL, COUNTING_TASK_PRIORITY, NULL);
    xTaskCreate(temp_task, "temp_task", 4096, NULL, TEMP_TASK_PRIORITY, NULL);
    xTaskCreate(print_task, "print_task", 4096, NULL, PRINT_TASK_PRIORITY, NULL);
    xTaskCreate(char_to_display_task, "char_to_display_task", 2048, NULL, CHAR_TO_DISPLAY_TASK_PRIORITY, NULL);
    xTaskCreate(test_alpha_display, "test_alpha_display", 4096, NULL, ALPHA_DISPLAY_TASK_PRIORITY, NULL);
    xTaskCreate(button_task, "button_task", 2048, NULL, BUTTON_TASK_PRIORITY, NULL);
    xTaskCreate(updateTime_task, "updateTime_task", 2048, NULL, BUTTON_TASK_PRIORITY, NULL);
}