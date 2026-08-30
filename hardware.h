#ifndef HARDWARE_H
#define HARDWARE_H

#include <stdint.h>

/**
 * @brief Initializes ADC and GPIO peripherals.
 */
void hardware_init(void);

/**
 * @brief Retrieves the current Wi-Fi RSSI value.
 * @return RSSI in dBm, or 0 on failure.
 */
int32_t read_rssi(void);

#endif /* HARDWARE_H */