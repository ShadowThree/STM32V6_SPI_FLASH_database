/*
 * This file is part of the Serial Flash Universal Driver Library.
 *
 * Copyright (c) 2016-2018, Armink, <armink.ztl@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * 'Software'), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED 'AS IS', WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * Function: Portable interface for each platform.
 * Created on: 2016-04-23
 */

#include <sfud.h>
#include <stdarg.h>
#include <string.h>
#include "main.h"
#include "spi.h"
#include "sfud_cfg.h"
#include "dbger.h"

#define SFUD_LOG_EN	0
#if SFUD_LOG_EN
#define SFUD_DBG	LOG_DBG
#else
#define SFUD_DBG(...)
#endif

#define SFUD_ERR	LOG_ERR

static char log_buf[256];

typedef struct {
		#if !defined(SFUD_USING_SFDP)	&& !defined(SFUD_USING_FLASH_INFO_TABLE)
		char* name;
		#endif
    SPI_HandleTypeDef *spix;
    GPIO_TypeDef *cs_gpiox;
    uint16_t cs_gpio_pin;
} flash_port, *flash_port_t;

#if !defined(SFUD_USING_SFDP)	&& !defined(SFUD_USING_FLASH_INFO_TABLE)
static flash_port flashPort[] = {
	{.name = SPI_FLASH_1_NAME, .spix = &hspi3, .cs_gpiox = SPI_FLASH_CS_GPIO_Port, .cs_gpio_pin = SPI_FLASH_CS_Pin }
};
#else
static flash_port flashPort[] = {
	[SFUD_W25Q64JV_DEVICE_INDEX] = { .spix = &hspi3, .cs_gpiox = SPI_FLASH_CS_GPIO_Port, .cs_gpio_pin = SPI_FLASH_CS_Pin }
};
#endif

void sfud_log_debug(const char *file, const long line, const char *format, ...);

/**
 * SPI write data then read data
 */
static sfud_err spi_write_read(const sfud_spi *spi, const uint8_t *write_buf, size_t write_size, uint8_t *read_buf, size_t read_size)
{
	HAL_StatusTypeDef hal_sta;
					
	HAL_GPIO_WritePin(((flash_port_t)(spi->user_data))->cs_gpiox, ((flash_port_t)(spi->user_data))->cs_gpio_pin, GPIO_PIN_RESET);
	
	hal_sta = HAL_SPI_Transmit(((flash_port_t)(spi->user_data))->spix, (uint8_t*)write_buf, write_size, 1000);
	if(hal_sta != HAL_OK) {
		HAL_GPIO_WritePin(((flash_port_t)(spi->user_data))->cs_gpiox, ((flash_port_t)(spi->user_data))->cs_gpio_pin, GPIO_PIN_SET);
			SFUD_ERR("sfud spi write err[%d]", hal_sta);
			return SFUD_ERR_WRITE;
	}
	
	if(read_size) {
		hal_sta = HAL_SPI_Receive(((flash_port_t)(spi->user_data))->spix, read_buf, read_size, 1000);
		if(hal_sta != HAL_OK) {
			HAL_GPIO_WritePin(((flash_port_t)(spi->user_data))->cs_gpiox, ((flash_port_t)(spi->user_data))->cs_gpio_pin, GPIO_PIN_SET);
			SFUD_ERR("sfud spi read err[%d]", hal_sta);
			return SFUD_ERR_READ;
		}
	}

	HAL_GPIO_WritePin(((flash_port_t)(spi->user_data))->cs_gpiox, ((flash_port_t)(spi->user_data))->cs_gpio_pin, GPIO_PIN_SET);

	return SFUD_SUCCESS;
}

#ifdef SFUD_USING_QSPI
/**
 * read flash data by QSPI
 */
static sfud_err qspi_read(const struct __sfud_spi *spi, uint32_t addr, sfud_qspi_read_cmd_format *qspi_read_cmd_format,
        uint8_t *read_buf, size_t read_size) {
    sfud_err result = SFUD_SUCCESS;

    /**
     * add your qspi read flash data code
     */

    return result;
}
#endif /* SFUD_USING_QSPI */

sfud_err sfud_spi_port_init(sfud_flash *flash)
{
    sfud_err result = SFUD_ERR_NOT_FOUND;

    /**
     * add your port spi bus and device object initialize code like this:
     * 1. rcc initialize
     * 2. gpio initialize
     * 3. spi device initialize
     * 4. flash->spi and flash->retry item initialize
     *    flash->spi.wr = spi_write_read; //Required
     *    flash->spi.qspi_read = qspi_read; //Required when QSPI mode enable
     *    flash->spi.lock = spi_lock;
     *    flash->spi.unlock = spi_unlock;
     *    flash->spi.user_data = &spix;
     *    flash->retry.delay = null;
     *    flash->retry.times = 10000; //Required
     */
	#if !defined(SFUD_USING_SFDP)	&& !defined(SFUD_USING_FLASH_INFO_TABLE)
		for(uint8_t i = 0; i < sizeof(flashPort) / sizeof(flashPort[0]); i++) {
			if(strcmp(flash->name, flashPort[i].name) == 0) {
				flash->spi.wr = spi_write_read;
				flash->spi.user_data = &flashPort[i];
				flash->retry.delay = NULL;
				flash->retry.times = 10000;
				result = SFUD_SUCCESS;
			}
		}
	#else
		switch(flash->index) {
			case SFUD_W25Q64JV_DEVICE_INDEX:
				flash->spi.wr = spi_write_read;
				flash->spi.user_data = &flashPort[SFUD_W25Q64JV_DEVICE_INDEX];
				flash->retry.delay = NULL;
				flash->retry.times = 10000;
				result = SFUD_SUCCESS;
				break;
		}
	#endif

    return result;
}

/**
 * This function is print debug info.
 *
 * @param file the file which has call this function
 * @param line the line number which has call this function
 * @param format output format
 * @param ... args
 */
void sfud_log_debug(const char *file, const long line, const char *format, ...) {
    va_list args;

    /* args point to the first variable parameter */
    va_start(args, format);
    SFUD_DBG("[SFUD](%s:%ld) ", file, line);
    /* must use vprintf to print */
    vsnprintf(log_buf, sizeof(log_buf), format, args);
    SFUD_DBG("%s\r\n", log_buf);
    va_end(args);
}

/**
 * This function is print routine info.
 *
 * @param format output format
 * @param ... args
 */
void sfud_log_info(const char *format, ...) {
    va_list args;

    /* args point to the first variable parameter */
    va_start(args, format);
    SFUD_DBG("[SFUD]");
    /* must use vprintf to print */
    vsnprintf(log_buf, sizeof(log_buf), format, args);
    SFUD_DBG("%s\r\n", log_buf);
    va_end(args);
}
