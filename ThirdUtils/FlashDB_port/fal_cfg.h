/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-05-17     armink       the first version
 */

#ifndef _FAL_CFG_H_
#define _FAL_CFG_H_

#include "main.h"

#define FAL_DEBUG 1
#if FAL_DEBUG
#include "dbger.h"
#define FAL_PRINTF		LOG_DBG
#else
#define FAL_PRINTF
#endif

#define FAL_PART_HAS_TABLE_CFG
#define FAL_USING_SFUD_PORT

#define FAL_USING_NOR_FLASH_DEV_NAME		SPI_FLASH_1_NAME

/* ===================== Flash device Configuration ========================= */
extern struct fal_flash_dev fal_flash_1;

/* flash device table */
#define FAL_FLASH_DEV_TABLE                                          \
{                                                                    \
    &fal_flash_1,                                                     \
}
/* ====================== Partition Configuration ========================== */
#ifdef FAL_PART_HAS_TABLE_CFG
/* partition table */
#define FAL_PART_TABLE                                                               \
{                                                                                    \
    {FAL_PART_MAGIC_WORD, "fdb_tsdb1", FAL_USING_NOR_FLASH_DEV_NAME,         0, 1024*1024, 0}, \
    {FAL_PART_MAGIC_WORD, "fdb_kvdb1", FAL_USING_NOR_FLASH_DEV_NAME, 1024*1024, 1024*1024, 0}, \
}
#endif /* FAL_PART_HAS_TABLE_CFG */

#endif /* _FAL_CFG_H_ */
