#ifndef __SPI_H
#define __SPI_H

#include <stdint.h>

void spi_init();
uint8_t spi_transfer(uint8_t send);
uint16_t spi_transfer16(uint16_t send);

#endif
