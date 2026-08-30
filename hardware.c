#include "hardware.h"
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "hardware/adc.h"
#include "hardware/gpio.h"

void hardware_init(void)
{
    /* Initialize ADC Peripheral & Pins */
    adc_init();
    adc_set_temp_sensor_enabled(true);

    for (uint8_t pin = 26; pin <= 29; pin++) {
        adc_gpio_init(pin);
    }

    /* Initialize standard GPIOs 0 - 22 as inputs by default */
    for (uint8_t pin = 1; pin <= 22; pin++) {
        gpio_init(pin);
        gpio_set_dir(pin, GPIO_IN);
    }
}

int32_t read_rssi(void)
{
    int32_t rssi = 0;
    if (cyw43_wifi_get_rssi(&cyw43_state, &rssi) != 0) {
        return 0;
    }
    return rssi;
}