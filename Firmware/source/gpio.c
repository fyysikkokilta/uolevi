#include <avr/io.h>

#include "gpio.h"


uint8_t gpio_write(uint8_t port, uint8_t pin, uint8_t value) {
    if (port == 0) {
        if(value) {
            PORTA.OUTSET = (1 << pin);
        }
        else {
            PORTA.OUTCLR = (1 << pin);
        }
    }
    else if (port == 1) {
        if(value) {
            PORTB.OUTSET = (1 << pin);
        }
        else {
            PORTB.OUTCLR = (1 << pin);
        }
    }
    else {
        return 1;
    }
    return 0;
}

void dac_write(uint8_t value) {
    DAC0.DATA = value;
}

void dac_enable(uint8_t en) {
    DAC0.CTRLA = (DAC0.CTRLA & ~(1 << 6)) | (en << 6);
}

void gpio_init(void) {
    PORTA.DIRSET = (1 << 5) | (1 << 4) | (1 << 3) | (1 << 1);
    PORTB.DIRSET = (1 << 3) | (1 << 2) | (1 << 1) | 1;
    
    //PORTA.PIN2CTRL = (1 << 3);    // Enable MISO pull-up resistor
    PORTA.PIN7CTRL = (1 << 3) | 0x3;    // Enable mode switch pull-up resistor and falling-edge interrupt
    
    CPUINT.STATUS |= 1 << 7;
    
    VREF.CTRLA |= 0x4;
    
    DAC0.DATA = 0xFF;
    DAC0.CTRLA |= 1;
    dac_enable(1);
}
