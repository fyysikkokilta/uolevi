#include <stdlib.h>
#include <avr/io.h>

#include "gpio.h"

uint8_t spi_transfer(uint8_t tx_value) {
    while (!(SPI0.INTFLAGS & (1 << 5))); // Wait for empty data buffer
    SPI0.DATA = tx_value;
    while (!(SPI0.INTFLAGS & (1 << 6))); // Wait for transmit
    SPI0.INTFLAGS |= (1 << 6); // Clear transmit flag

    // Read data
    uint8_t rx_value = SPI0.DATA;
    while (SPI0.INTFLAGS & (1 << 7)) {
        rx_value = SPI0.DATA;
    }

    return rx_value;
}

void spi_peripheral(uint8_t peripheral, uint8_t enable) {
    if (peripheral == 0) {
        gpio_write(0, 4, !enable);
    } else if (peripheral == 1) {
        gpio_write(0, 5, !enable);
    }
}

uint8_t spi_init(void) {
    SPI0.CTRLA = (1 << 5) | (1 << 4) | (0x0 << 1); // Set SPI freq. to 5 MHz
    SPI0.CTRLB = (1 << 7) | (1 << 2);  // Enable buffer and disable auto SS
    //SPI0.CTRLB = 1 << 2; // Disable auto SS

    gpio_write(0, 4, 1);
    gpio_write(0, 5, 1);

    SPI0.CTRLA |= 1; // Enable SPI

    // Wake up flash    
    spi_peripheral(0, 1);
    spi_transfer(0xAB);
    spi_peripheral(0, 0);

    return 0;
}

uint8_t spi_shutdown(void) {
    // Power down flash
    spi_peripheral(0, 1);
    spi_transfer(0xB9);
    spi_peripheral(0, 0);

    // Give empty clocks for SD card
    spi_peripheral(0, 0);
    spi_peripheral(1, 0);
    for (int i = 0; i < 8; i++) {
        spi_transfer(0xFF);
    }

    SPI0.CTRLA = 0; // Disable SPI

    return 0;
}
