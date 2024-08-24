#include <stdlib.h>
#include <avr/io.h>

#include "flash.h"
#include "spi.h"

// Enable writing to external flash
uint8_t flash_write_enable(void) {
    spi_peripheral(0, 1);
    spi_transfer(0x06);
    spi_peripheral(0, 0);

    return 0;
}

// Wait until external flash is not busy
uint8_t flash_wait(void) {
    spi_peripheral(0, 1);
    spi_transfer(0x05);
    spi_transfer(0xFF);
    spi_peripheral(0, 0);
    
    uint8_t rx_val = 1;
    while (rx_val & 1) { // Wait for empty data buffer
        spi_peripheral(0, 1);
        spi_transfer(0x05);
        rx_val = spi_transfer(0xFF);
        spi_peripheral(0, 0);
    }

    return 0;
}
