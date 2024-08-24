#ifndef SPI_H
#define	SPI_H

uint8_t spi_transfer(uint8_t tx_value);
void spi_peripheral(uint8_t peripheral, uint8_t enable);
uint8_t spi_init(void);
uint8_t spi_shutdown(void);

#endif	/* SPI_H */

