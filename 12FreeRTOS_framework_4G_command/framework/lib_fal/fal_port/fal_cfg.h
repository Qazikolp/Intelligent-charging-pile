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

//#include <rtconfig.h>
//#include <board.h>

#define FAL_DEBUG      1
#define FAL_PART_HAS_TABLE_CFG

#define NOR_FLASH_DEV_NAME             "norflash0"

/* ===================== Flash device Configuration ========================= */
extern const struct fal_flash_dev onchip_flash;

/* flash device table */
#define FAL_FLASH_DEV_TABLE                                          \
{                                                                    \
    &onchip_flash,                                           \
}
/* ====================== Partition Configuration ========================== */

/*----------------------------------------------------------------------------------*/
//flash ¿Õ¼ä°²ÅÅ

//0
//´æ´¢bootloader
#define _f_boot_start	0
#define _f_boot_size	64*1024

//´æ´¢ÅäÖÃÎÄ¼þ
#define _f_conf_start	64*1024
#define _f_conf_size	 4*1024

//´æ´¢app
#define _f_app_start	70*1024
#define _f_app_size		186*1024

//256KB
//´æ´¢Êý¾Ý¿â
#define _f_fdb_start	256*1024
#define _f_fdb_size		256*1024

/*----------------------------------------------------------------------------------*/


#ifdef FAL_PART_HAS_TABLE_CFG
/* partition table */
#define FAL_PART_TABLE                                                               \
{                                                                                    \
    {FAL_PART_MAGIC_WORD, "boot",	"onchip",  _f_boot_start,   _f_boot_size, 0}, \
    {FAL_PART_MAGIC_WORD, "conf", "onchip",  _f_conf_start,   _f_conf_size, 0}, \
    {FAL_PART_MAGIC_WORD, "app", 	"onchip",  _f_app_start,  	_f_app_size, 0}, \
    {FAL_PART_MAGIC_WORD, "fdb", 	"onchip",  _f_fdb_start,  	_f_fdb_size, 0}, \
}
#endif /* FAL_PART_HAS_TABLE_CFG */

#endif /* _FAL_CFG_H_ */
