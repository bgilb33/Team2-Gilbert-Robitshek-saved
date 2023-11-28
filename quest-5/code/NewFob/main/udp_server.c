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
// #include "driver/periph_ctrl.h"
#include "driver/gptimer.h" // Added in 2023
// #include "clk_tree.h"             // Added in 2023
#include "driver/gpio.h"          // Added in 2023
#include "driver/mcpwm_prelude.h" // Added in 2023
#include "driver/ledc.h"          // Added in 2023

// UDP Start
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

static const char *TAG = "example";

// UDP End

// MCPWM defintions -- 2023: modified
#define MCPWM_TIMER_RESOLUTION_HZ 10000000 // 10MHz, 1 tick = 0.1us
#define MCPWM_FREQ_HZ 38000                // 38KHz PWM -- 1/38kHz = 26.3us
#define MCPWM_FREQ_PERIOD 263              // 263 ticks = 263 * 0.1us = 26.3us
#define MCPWM_GPIO_NUM 25

// LEDC definitions -- 2023: modified
// NOT USED / altnernative to MCPWM above
#define LEDC_TIMER LEDC_TIMER_0
#define LEDC_MODE LEDC_LOW_SPEED_MODE
#define LEDC_OUTPUT_IO 25
#define LEDC_CHANNEL LEDC_CHANNEL_0
#define LEDC_DUTY_RES 6      // Set duty resolution to 6 bits
#define LEDC_DUTY 32         // Set duty to 50%. ((2^6) - 1) * 50% = 32
#define LEDC_FREQUENCY 38000 // Frequency in Hertz. 38kHz

// UART definitions -- 2023: no changes
#define UART_TX_GPIO_NUM 26 // A0
#define UART_RX_GPIO_NUM 34 // A2
#define BUF_SIZE (1024)
#define BUF_SIZE2 (32)

// Hardware interrupt definitions -- 2023: no changes
#define GPIO_INPUT_IO_1 4
#define ESP_INTR_FLAG_DEFAULT 0
#define GPIO_INPUT_PIN_SEL 1ULL << GPIO_INPUT_IO_1

// LED Output pins definitions -- 2023: minor changes
#define YELLOWPIN 14
#define GREENPIN 32
#define REDPIN 15

// Default ID/color
#define ID 3
#define COLOR 'R'

// Variables for my ID, minVal and status plus string fragments
char start = 0x1B; // START BYTE that UART looks for
char myID = (char)ID;
int len_out = 3;
int sending = 0;

int state = 0;

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
static const char *TAG_TIMER = "ec444: timer"; // For timer logs

// Button interrupt handler -- add to queue -- 2023: no changes
static void IRAM_ATTR gpio_isr_handler(void *arg)
{
    uint32_t gpio_num = (uint32_t)arg;
    xQueueSendFromISR(gpio_evt_queue, &gpio_num, NULL);
}

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
    gpio_reset_pin(REDPIN);
    gpio_set_direction(REDPIN, GPIO_MODE_OUTPUT);

    gpio_reset_pin(GREENPIN);
    gpio_set_direction(GREENPIN, GPIO_MODE_OUTPUT);

    gpio_reset_pin(YELLOWPIN);
    gpio_set_direction(YELLOWPIN, GPIO_MODE_OUTPUT);
}

// Button interrupt init -- 2023: minor changes
static void button_init()
{
    // zero-initialize the config structure.
    gpio_config_t io_conf = {};
    // interrupt of rising edge
    io_conf.intr_type = GPIO_INTR_POSEDGE;
    // bit mask of the pins, use GPIO4 here
    io_conf.pin_bit_mask = GPIO_INPUT_PIN_SEL;
    // set as input mode
    io_conf.mode = GPIO_MODE_INPUT;
    // enable pull-up mode
    io_conf.pull_up_en = 1;
    gpio_config(&io_conf);

    gpio_intr_enable(GPIO_INPUT_IO_1); // 2023: retained

    // install gpio isr service
    gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);
    // hook isr handler for specific gpio pin
    gpio_isr_handler_add(GPIO_INPUT_IO_1, gpio_isr_handler, (void *)GPIO_INPUT_IO_1);

    // create a queue to handle gpio event from isr
    gpio_evt_queue = xQueueCreate(10, sizeof(uint32_t));
}

// Tasks
// Button task -- rotate through myIDs -- 2023: no changes
void button_task()
{
    uint32_t io_num;
    while (1)
    {
        if (xQueueReceive(gpio_evt_queue, &io_num, portMAX_DELAY))
        {
            xSemaphoreTake(mux, portMAX_DELAY);
            sending = ~sending;
            if (sending)
            {
                state = 2;
            }
            else
            {
                state = 1;
            }
            xSemaphoreGive(mux);
            printf("Button pressed.\n");
        }
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}

// Send task -- sends payload | Start | myID | Start | myID -- 2023: no changes
void send_task()
{
    while (1)
    {
        if (sending) // Only send data when sending is true
        {
            char *data_out = (char *)malloc(len_out);
            xSemaphoreTake(mux, portMAX_DELAY);
            data_out[0] = start;
            data_out[1] = (int)myID;
            data_out[2] = genCheckSum(data_out, len_out - 1);
            // ESP_LOG_BUFFER_HEXDUMP(TAG_SYSTEM, data_out, len_out, ESP_LOG_INFO);
            uart_write_bytes(UART_NUM_1, data_out, len_out);
            xSemaphoreGive(mux);
        }

        vTaskDelay(50 / portTICK_PERIOD_MS);
    }
}

void fob_state()
{
    while (1)
    {
        // Parking Open
        if (state == 1)
        {
            gpio_set_level(REDPIN, 0);
            gpio_set_level(YELLOWPIN, 0);
            gpio_set_level(GREENPIN, 1);
        }
        // Loading
        else if (state == 2)
        {
            gpio_set_level(REDPIN, 0);
            gpio_set_level(YELLOWPIN, 1);
            gpio_set_level(GREENPIN, 0);
        }
        // Parking Taken
        else if (state == 3)
        {
            gpio_set_level(REDPIN, 1);
            gpio_set_level(YELLOWPIN, 0);
            gpio_set_level(GREENPIN, 0);
        }
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}

// UDP Server Start
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
                ESP_LOGI(TAG, "%s", rx_buffer);

                // Alert: Park 
                // This will park the car, change the light to red, and change the led status to 3

                // Alert: Timeout
                // This will unpark the care, change the light to green and change the led status to 2
                if (strncmp(rx_buffer, "Alert: ", strlen("Alert: ")) == 0) {
                    // Move the pointer to the position after "Alert: "
                    const char *afterAlert = rx_buffer + strlen("Alert: ");

                    // Check if the characters after "Alert: " are "Park Car"
                    if (strncmp(afterAlert, "Park", strlen("Park")) == 0) {
                        printf("Alert is for Parking the Car.\n");
                        state = 3;
                    } else if (strncmp(afterAlert, "Timeout", strlen("Timeout")) == 0) {
                        printf("Alert is for Timeout.\n");
                        state = 1;
                    } else if (strncmp(afterAlert, "Taken", strlen("Taken")) == 0) {
                        printf("Alert is for Taken Spot.\n");
                        state = 1;
                    } else {
                        printf("Alert is for something else.\n");
                    }
                } else {
                    printf("String does not start with 'Alert: '.\n");
                }

                int err = sendto(sock, "Fob 1 data recieved", 20, 0, (struct sockaddr *)&source_addr, sizeof(source_addr));
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
// UDP Server End


void app_main()
{
    // UDP Init Functions
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    ESP_ERROR_CHECK(example_connect());
    // UDP End Functions

    // UDP Function
    xTaskCreate(udp_server_task, "udp_server", 4096, (void*)AF_INET6, 5, NULL);
    // UDP Function End

    // Mutex for current values when sending -- no changes
    mux = xSemaphoreCreateMutex();

    // Timer queue initialize -- 2023: modified
    timer_queue = xQueueCreate(10, sizeof(example_queue_element_t));
    if (!timer_queue)
    {
        ESP_LOGE(TAG_TIMER, "Creating queue failed");
        return;
    }

    // Create task to handle timer-based events -- no changes
    // xTaskCreate(timer_evt_task, "timer_evt_task", 2048, NULL, 5, NULL);

    // Initialize all the things -- no changes
    // rmt_tx_init();
    uart_init();
    led_init();
    button_init();
    pwm_init();

    // Create tasks for receive, send, set gpio, and button -- 2023 -- no changes

    // *****Create one receiver and one transmitter (but not both)
    // xTaskCreate(recv_task, "uart_rx_task", 1024 * 4, NULL, configMAX_PRIORITIES, NULL);
    // *****Create one receiver and one transmitter (but not both)
    xTaskCreate(send_task, "uart_tx_task", 1024 * 2, NULL, configMAX_PRIORITIES, NULL);
    xTaskCreate(button_task, "button_task", 1024 * 2, NULL, configMAX_PRIORITIES, NULL);
    xTaskCreate(fob_state, "fob_state", 1024 * 2, NULL, configMAX_PRIORITIES, NULL);

    while (1)
    {
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}