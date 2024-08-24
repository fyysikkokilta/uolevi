#ifndef GPIO_H
#define	GPIO_H

uint8_t gpio_write(uint8_t port, uint8_t pin, uint8_t value);
void dac_write(uint8_t value);
void dac_enable(uint8_t en);
void gpio_init(void);

#endif	/* GPIO_H */

