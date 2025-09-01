#ifndef MICROPHP_ESP32_HAL_H
#define MICROPHP_ESP32_HAL_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// GPIO modes
#define GPIO_INPUT     0
#define GPIO_OUTPUT    1
#define GPIO_INPUT_PULLUP  2
#define GPIO_INPUT_PULLDOWN 3

// GPIO functions
int esp32_gpio_mode(int pin, int mode);
int esp32_gpio_write(int pin, bool value);
bool esp32_gpio_read(int pin);
int esp32_gpio_set_pull(int pin, int pull);

// PWM functions
typedef struct {
    int channel;
    int pin;
    int freq_hz;
    float duty_cycle;
} esp32_pwm_config_t;

int esp32_pwm_open(const esp32_pwm_config_t *config);
int esp32_pwm_set_duty(int channel, float duty_cycle);
int esp32_pwm_set_freq(int channel, int freq_hz);
int esp32_pwm_close(int channel);

// I2C functions
typedef struct {
    int port;
    int scl_pin;
    int sda_pin;
    int freq_hz;
} esp32_i2c_config_t;

int esp32_i2c_open(const esp32_i2c_config_t *config);
int esp32_i2c_close(int port);
int esp32_i2c_write(int port, uint8_t addr, const uint8_t *data, size_t len);
int esp32_i2c_read(int port, uint8_t addr, uint8_t *data, size_t len);

// SPI functions
typedef struct {
    int host;
    int mosi_pin;
    int miso_pin;
    int sclk_pin;
    int cs_pin;
    int freq_hz;
    int mode;
} esp32_spi_config_t;

int esp32_spi_open(const esp32_spi_config_t *config);
int esp32_spi_close(int host);
int esp32_spi_transfer(int host, const uint8_t *tx_data, uint8_t *rx_data, size_t len);

// UART functions
typedef struct {
    int uart_num;
    int baud_rate;
    int data_bits;
    int parity;
    int stop_bits;
    int tx_pin;
    int rx_pin;
} esp32_uart_config_t;

int esp32_uart_open(const esp32_uart_config_t *config);
int esp32_uart_close(int uart_num);
int esp32_uart_write(int uart_num, const uint8_t *data, size_t len);
int esp32_uart_read(int uart_num, uint8_t *data, size_t len);
int esp32_uart_available(int uart_num);

// Timer functions
typedef struct {
    int timer_num;
    int freq_hz;
    bool auto_reload;
} esp32_timer_config_t;

int esp32_timer_open(const esp32_timer_config_t *config);
int esp32_timer_start(int timer_num);
int esp32_timer_stop(int timer_num);
int esp32_timer_set_freq(int timer_num, int freq_hz);

// ADC functions
int esp32_adc_read(int channel);
int esp32_adc_set_atten(int channel, int atten);

// RTC functions
typedef struct {
    uint32_t seconds;
    uint32_t microseconds;
} esp32_rtc_time_t;

esp32_rtc_time_t esp32_rtc_get_time(void);
int esp32_rtc_set_time(const esp32_rtc_time_t *time);

// Sleep functions
void esp32_sleep_ms(uint32_t ms);
void esp32_sleep_us(uint32_t us);

// System functions
uint32_t esp32_get_free_heap(void);
uint32_t esp32_get_min_free_heap(void);
void esp32_restart(void);

// Wi-Fi functions (if enabled)
#ifdef MICROPHP_NET
int esp32_wifi_init(void);
int esp32_wifi_connect(const char *ssid, const char *password);
int esp32_wifi_disconnect(void);
bool esp32_wifi_is_connected(void);
#endif

// TCP/UDP functions (if enabled)
#ifdef MICROPHP_NET
typedef struct {
    int sock;
    char *remote_ip;
    int remote_port;
} esp32_socket_t;

esp32_socket_t* esp32_tcp_connect(const char *ip, int port);
int esp32_tcp_send(esp32_socket_t *sock, const uint8_t *data, size_t len);
int esp32_tcp_recv(esp32_socket_t *sock, uint8_t *data, size_t len);
int esp32_tcp_close(esp32_socket_t *sock);

esp32_socket_t* esp32_udp_create(int port);
int esp32_udp_send_to(esp32_socket_t *sock, const char *ip, int port, const uint8_t *data, size_t len);
int esp32_udp_recv_from(esp32_socket_t *sock, char *ip, int *port, uint8_t *data, size_t len);
int esp32_udp_close(esp32_socket_t *sock);
#endif

// Key-value store functions (if enabled)
#ifdef MICROPHP_KVSTORE
int esp32_kv_set(const char *key, const void *value, size_t size);
int esp32_kv_get(const char *key, void *value, size_t *size);
int esp32_kv_delete(const char *key);
#endif

#ifdef __cplusplus
}
#endif

#endif // MICROPHP_ESP32_HAL_H
