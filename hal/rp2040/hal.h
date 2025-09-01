#ifndef MICROPHP_RP2040_HAL_H
#define MICROPHP_RP2040_HAL_H

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
int rp2040_gpio_mode(int pin, int mode);
int rp2040_gpio_write(int pin, bool value);
bool rp2040_gpio_read(int pin);
int rp2040_gpio_set_pull(int pin, int pull);

// PWM functions
typedef struct {
    int slice;
    int channel;
    int pin;
    int freq_hz;
    float duty_cycle;
} rp2040_pwm_config_t;

int rp2040_pwm_open(const rp2040_pwm_config_t *config);
int rp2040_pwm_set_duty(int slice, int channel, float duty_cycle);
int rp2040_pwm_set_freq(int slice, int freq_hz);
int rp2040_pwm_close(int slice);

// I2C functions
typedef struct {
    int i2c_num;
    int scl_pin;
    int sda_pin;
    int freq_hz;
} rp2040_i2c_config_t;

int rp2040_i2c_open(const rp2040_i2c_config_t *config);
int rp2040_i2c_close(int i2c_num);
int rp2040_i2c_write(int i2c_num, uint8_t addr, const uint8_t *data, size_t len);
int rp2040_i2c_read(int i2c_num, uint8_t addr, uint8_t *data, size_t len);

// SPI functions
typedef struct {
    int spi_num;
    int mosi_pin;
    int miso_pin;
    int sclk_pin;
    int cs_pin;
    int freq_hz;
    int mode;
} rp2040_spi_config_t;

int rp2040_spi_open(const rp2040_spi_config_t *config);
int rp2040_spi_close(int spi_num);
int rp2040_spi_transfer(int spi_num, const uint8_t *tx_data, uint8_t *rx_data, size_t len);

// UART functions
typedef struct {
    int uart_num;
    int baud_rate;
    int data_bits;
    int parity;
    int stop_bits;
    int tx_pin;
    int rx_pin;
} rp2040_uart_config_t;

int rp2040_uart_open(const rp2040_uart_config_t *config);
int rp2040_uart_close(int uart_num);
int rp2040_uart_write(int uart_num, const uint8_t *data, size_t len);
int rp2040_uart_read(int uart_num, uint8_t *data, size_t len);
int rp2040_uart_available(int uart_num);

// Timer functions
typedef struct {
    int timer_num;
    int freq_hz;
    bool auto_reload;
} rp2040_timer_config_t;

int rp2040_timer_open(const rp2040_timer_config_t *config);
int rp2040_timer_start(int timer_num);
int rp2040_timer_stop(int timer_num);
int rp2040_timer_set_freq(int timer_num, int freq_hz);

// ADC functions
int rp2040_adc_read(int channel);
int rp2040_adc_set_atten(int channel, int atten);

// RTC functions
typedef struct {
    uint32_t seconds;
    uint32_t microseconds;
} rp2040_rtc_time_t;

rp2040_rtc_time_t rp2040_rtc_get_time(void);
int rp2040_rtc_set_time(const rp2040_rtc_time_t *time);

// Sleep functions
void rp2040_sleep_ms(uint32_t ms);
void rp2040_sleep_us(uint32_t us);

// System functions
uint32_t rp2040_get_free_heap(void);
uint32_t rp2040_get_min_free_heap(void);
void rp2040_restart(void);

// Flash functions
int rp2040_flash_read(uint32_t offset, void *data, size_t len);
int rp2040_flash_write(uint32_t offset, const void *data, size_t len);
int rp2040_flash_erase(uint32_t offset, size_t len);

// Key-value store functions (if enabled)
#ifdef MICROPHP_KVSTORE
int rp2040_kv_set(const char *key, const void *value, size_t size);
int rp2040_kv_get(const char *key, void *value, size_t *size);
int rp2040_kv_delete(const char *key);
#endif

#ifdef __cplusplus
}
#endif

#endif // MICROPHP_RP2040_HAL_H
