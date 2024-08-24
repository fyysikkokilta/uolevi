#include <stdlib.h>
#include <avr/cpufunc.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <string.h>
#include <avr/sleep.h>
#include "petitfs/diskio.h"
#include "petitfs/pff.h"

#include "main.h"
#include "gpio.h"
#include "spi.h"
#include "flash.h"

#define BUFFER_SIZE 1024

FATFS file_system;
uint8_t reset = 0;
uint8_t sd_initialized = 0;
uint8_t file_num = 0;

// Initialize main and Timer A clocks

uint8_t clk_init(void) {
    while (!(CLKCTRL.MCLKSTATUS & (1 << 4)));
    *((volatile unsigned char*) (0x0030 + 0x04)) = 0xD8; // Unlock IOREG
    CLKCTRL.MCLKCTRLB = 1; // Set CPU clock to 10 MHz

    // Start Timer A clock at 39.0625 kHz
    TCA0.SINGLE.PER = 0xFFFF;
    TCA0.SINGLE.CTRLA = (0x6 << 1) | 1;

    return 0;
}

// Disable peripherals and power down

void shutdown(void) {
    disable_sd_card();
    spi_shutdown(); // Disable SPI

    DAC0.CTRLA = 0; // Disable DAC

    // Clear GPIO
    PORTA.OUTCLR = 0xFF;
    PORTA.DIRSET = 0;
    PORTB.OUTCLR = 0xFF;
    PORTB.DIRSET = 0;

    SLPCTRL.CTRLA = (0x2 << 1) | 1; // Set sleep mode to power down
    sleep_mode(); // Sleep
}

// Wait for Timer A clocks (25.6 us per clock)

uint8_t delay_clks(uint8_t delay) {
    uint16_t end_val = TCA0.SINGLE.CNT + delay;

    if (end_val < TCA0.SINGLE.CNT) {
        while (!(TCA0.SINGLE.INTFLAGS & 1));
        TCA0.SINGLE.INTFLAGS |= 1;
    }

    while (TCA0.SINGLE.CNT < end_val);

    return 0;
}

// Wait for milliseconds (max. 1680)

uint8_t delay_ms(uint16_t delay) {
    uint16_t end_val = TCA0.SINGLE.CNT + delay * 39;

    if (end_val < TCA0.SINGLE.CNT) {
        while (!(TCA0.SINGLE.INTFLAGS & 1));
        TCA0.SINGLE.INTFLAGS |= 1;
    }

    while (TCA0.SINGLE.CNT < end_val);

    return 0;
}

// Beep for given number of times

void beep(uint8_t beeps) {
    for (uint8_t i = 0; i < beeps; i++) {
        for (uint8_t j = 0; j < 244; j++) {
            dac_write(0);
            delay_clks(32);
            dac_write(0x0F);
            delay_clks(32);
        }
        delay_ms(800);
    }
}

// Initialize microSD card in SPI mode and mount file system

uint8_t init_sd_card(void) {
    spi_peripheral(0, 0);

    FRESULT result;
    DSTATUS status = STA_NOINIT;

    uint8_t spi_presc = (SPI0.CTRLA >> 1) & 0x3;
    SPI0.CTRLA = SPI0.CTRLA & ~(0x3 << 1);
    SPI0.CTRLA |= (0x2 << 1); // Set SPI freq. to 312.5 kHz

    /* Initialize physical drive */
    status = disk_initialize();
    if (status == STA_NODISK) {
        return 1;
    }

    SPI0.CTRLA = SPI0.CTRLA & ~(0x3 << 1);
    SPI0.CTRLA |= (spi_presc << 1); // Reset SPI freq. setting

    /* Mount volume */
    result = pf_mount(&file_system);
    if (result != FR_OK) {
        return 2;
    }

    sd_initialized = 1;

    return 0;
}

uint8_t disable_sd_card(void) {
    if (sd_initialized) {
        spi_peripheral(0, 0);
        disk_idle(&file_system);
        sd_initialized = 0;
    }

    return 0;
}

// Try opening file from microSD card

uint8_t open_file(uint8_t f_num) {
    if (f_num > 10)
        return 1;

    if (!sd_initialized) {
        init_sd_card();
    }

    // Give empty clocks for SD card
    spi_peripheral(0, 0);
    spi_peripheral(1, 0);
    for (int i = 0; i < 8; i++) {
        spi_transfer(0xFF);
    }

    FRESULT result;
    char filename[6] = {'0' + (f_num - 1), '.', 'U', 'L', 'V', '\0'};
    result = pf_open(filename);
    if (result != FR_OK) {
        return 2;
    }

    sd_initialized = 2;

    return 0;
}

// Transfer opened file from microSD card to external flash memory

uint8_t read_file(void) {
    spi_peripheral(0, 0);

    if (sd_initialized != 2 && open_file(file_num)) {
        return 1;
    }
    beep(file_num);
    delay_ms(1200);

    gpio_write(1, 2, 0);
    gpio_write(1, 3, 1);

    uint16_t rx_bytes;
    uint8_t rx_buff[BUFFER_SIZE];
    uint8_t highest_byte;

    uint16_t erases = 0;
    uint16_t flash_address = 0;
    uint8_t flash_i = 0;
    while (1) {
        if (reset) {
            spi_peripheral(0, 0);
            return 2;
        }

        pf_read(rx_buff, BUFFER_SIZE, &rx_bytes);


        if (!erases) {
            highest_byte = rx_buff[3];
            uint32_t bytes = 0;
            for (uint8_t i = 0; i < 4; i++) {
                bytes |= (uint32_t) rx_buff[i] << (8 * i);
            }
            bytes += 4;

            erases = (bytes >> 16);
            if (bytes & 0xFFFF) {
                erases++;
            }

            rx_buff[3] = 0xFF; // Erase highest byte count byte until finished

            for (uint16_t i = 0; i < erases; i++) {
                if (reset) {
                    spi_peripheral(0, 0);
                    return 2;
                }

                PORTB.OUTTGL = 1 << 2;
                // Erase 64kB block
                flash_write_enable(); // Enable writing
                spi_peripheral(0, 1);
                spi_transfer(0xD8);
                spi_transfer(i);
                spi_transfer(0x00);
                spi_transfer(0x00);
                spi_peripheral(0, 0);

                // Wait for erase
                flash_wait();
            }
            gpio_write(1, 2, 0);
            gpio_write(1, 3, 1);
        }

        PORTB.OUTTGL = 1 << 3 | 1 << 2;
        flash_i = 0;
        for (uint16_t i = 0; i < rx_bytes; i++) {
            if (reset) {
                spi_peripheral(0, 0);
                return 2;
            }

            if (!flash_i) {

                spi_peripheral(0, 0);

                // Wait until not busy
                flash_wait();

                // Write
                flash_write_enable(); // Enable writing
                spi_peripheral(0, 1);
                spi_transfer(0x02);
                spi_transfer((flash_address >> 8) & 0xFF);
                spi_transfer(flash_address & 0xFF);
                spi_transfer(0x00);

                flash_address++;
            }
            spi_transfer(rx_buff[i]);
            flash_i++;
        }
        spi_peripheral(0, 0);

        if (rx_bytes != BUFFER_SIZE) {
            break;
        }
    }
    // Wait until not busy
    flash_wait();

    // Write first byte to indicate finished read
    flash_write_enable(); // Enable writing

    spi_peripheral(0, 1);
    spi_transfer(0x02);
    spi_transfer(0x00);
    spi_transfer(0x00);
    spi_transfer(0x03);
    spi_transfer(highest_byte);
    spi_peripheral(0, 0);

    gpio_write(1, 3, 0);

    disable_sd_card();

    return 0;
}

// Play song from external flash memory

uint8_t play(void) {
    // Wait until not busy
    flash_wait();

    // Start read
    spi_peripheral(0, 1);
    spi_transfer(0x03);
    spi_transfer(0x00);
    spi_transfer(0x00);
    spi_transfer(0x00);

    // Read number of data bytes
    uint32_t bytes = 0;
    for (int i = 0; i < 4; i++) {
        bytes |= (uint32_t) spi_transfer(0xFF) << (8 * i);
    }

    if (!(bytes >> 24)) {
        uint8_t mech_byte = 0;
        uint16_t j = 0;
        for (uint32_t i = 0; i < bytes; i++) {
            if (reset)
                return 1;

            if (j == 0) { // Play next mech sample
                if (!mech_byte) {
                    mech_byte = spi_transfer(0xFF); // Read next byte
                    i++;

                    // Play mech sample
                    gpio_write(1, 0, mech_byte & 1);
                    gpio_write(1, 1, mech_byte & (1 << 1));
                    gpio_write(1, 2, mech_byte & (1 << 2));
                    gpio_write(1, 3, mech_byte & (1 << 3));

                    mech_byte = mech_byte | 1;

                    for (uint8_t k = 0; k < 6; k++)
                        __asm__ __volatile__("nop "); // Adjust timing

                    __asm__ __volatile__("nop "); // Adjust timing
                    __asm__ __volatile__("nop "); // Adjust timing
                    __asm__ __volatile__("nop "); // Adjust timing
                } else {
                    gpio_write(1, 0, mech_byte & (1 << 4));
                    gpio_write(1, 1, mech_byte & (1 << 5));
                    gpio_write(1, 2, mech_byte & (1 << 6));
                    gpio_write(1, 3, mech_byte & (1 << 7));

                    mech_byte = 0;

                    for (uint8_t k = 0; k < 20; k++)
                        __asm__ __volatile__("nop "); // Adjust timing

                    __asm__ __volatile__("nop "); // Adjust timing
                    __asm__ __volatile__("nop "); // Adjust timing
                    __asm__ __volatile__("nop "); // Adjust timing
                }
                j = 746;
            } else {
                for (uint8_t k = 0; k < 62; k++)
                    __asm__ __volatile__("nop "); // Adjust timing

                __asm__ __volatile__("nop "); // Adjust timing
                __asm__ __volatile__("nop "); // Adjust timing
                __asm__ __volatile__("nop "); // Adjust timing
            }
            j--;
            dac_write(spi_transfer(0xFF)); // Play next audio sample
        }
    } else {
        spi_peripheral(0, 0);
        return 1;
    }
    spi_peripheral(0, 0);
    return 0;
}

// Mode button interrupt

ISR(PORTA_PORT_vect) {
    PORTA.INTFLAGS |= 1 << 7;

    gpio_write(1, 0, 0);
    gpio_write(1, 1, 0);
    gpio_write(1, 2, 0);
    gpio_write(1, 3, 0);

    uint8_t wait_time = 0;
    while (!(PORTA.IN & (1 << 7)) && wait_time < 50) {
        delay_ms(10);
        wait_time++;

        if (wait_time >= 50) {
            break;
        }
    }
    if (wait_time < 50)
        return;

    gpio_write(1, 2, 1);
    gpio_write(1, 3, 1);

    wait_time = 0;
    while (!(PORTA.IN & (1 << 7))) {
        delay_ms(10);
        wait_time++;

        if (wait_time >= 150) {
            gpio_write(1, 2, 0);
            gpio_write(1, 3, 0);
            while (!(PORTA.IN & (1 << 7)));
            break;
        }
    }
    if (wait_time < 150) {
        cli(); // Block interrupts
        shutdown();
        sei(); // Unblock interrupts
    }

    file_num++;
    if (file_num != 1 && open_file(file_num)) {
        file_num = 1;
    }

    if (!open_file(file_num)) {
        reset = 1;
    }
}

// Loop function

uint8_t loop(void) {
    if (!reset && play()) {
        if (!reset) {
            file_num = 1;
            if (!read_file()) {
                play();
            }
        }
    }
    if (reset) {
        reset = 0;
        read_file();
        return 1;
    }

    return 0;
}

// Enter function

int main(void) {
    cli(); // Block interrupts
    clk_init();
    gpio_init();
    
    // Slowly move DAC to center value
    for(uint8_t i = 0; i < 128; i++) {
        dac_write(0xFF - i);
        delay_clks(1);
    }
    
    spi_init();
    sei(); // Unblock interrupts
    
    while (loop());
    shutdown();

    return 0;
}
