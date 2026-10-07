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

//#include <rtconfig.h>   // RT-Thread操作系统配置头文件，裸机环境注释掉
//#include <board.h>      // RT-Thread板级头文件，裸机环境注释掉

#include "framework.h"

#define FAL_DEBUG      1                // 开启FAL调试打印输出，0关闭，1开启
#define FAL_PART_HAS_TABLE_CFG          // 使用代码内分区表配置；不定义则从Flash中读取分区表

#define NOR_FLASH_DEV_NAME             "norflash0"   // 外部SPI-Flash设备名称宏定义


/*----------------------------------------------------------------------------------*/
//flash 空间安排

//0
//存储bootloader
#define _f_boot_start	0
#define _f_boot_size	64*1024

//存储配置文件
#define _f_conf_start	64*1024
#define _f_conf_size	 4*1024

//存储app
#define _f_app_start	70*1024
#define _f_app_size		186*1024

//256KB
//存储数据库
#define _f_fdb_start	256*1024
#define _f_fdb_size		256*1024


/*----------------------------------------------------------------------------------*/

/* ===================== Flash设备配置 ========================= */
// 声明两个Flash设备的外部对象
extern const struct fal_flash_dev onchip_flash;     // 单片机片内Flash设备对象
extern const struct fal_flash_dev nor_flash0;       // 外接SPI-NorFlash(W25Q)设备对象

/* flash设备表：把注册好的Flash设备添加到此表，FAL便可识别管理 */
#define FAL_FLASH_DEV_TABLE                                          \
{                                                                    \
    &onchip_flash,                                                   \
   /* &nor_flash0,          W25Q64 外接 SPI Flash(不要这个)*/                 \
}

/* ====================== 分区配置 ========================== */
#ifdef FAL_PART_HAS_TABLE_CFG
/*
 * 分区表：在物理Flash上划分出多个逻辑分区
 * 分区结构体参数：
 * {魔术字, 分区名字, 所属Flash设备名, 分区在设备内偏移地址, 分区大小, 保留参数}
 *
 * 注意事项：
 * 1.偏移地址、大小单位：字节
 * 2.分区起始地址必须扇区对齐
 * 3.分区之间地址不能重叠
 */
#define FAL_PART_TABLE                                                               \
{                                                                                    \
    /* 分区1：片内Flash-boot分区，偏移0，大小64KB， 存储bootloader */                        \
    {FAL_PART_MAGIC_WORD, "boot", "onchip", _f_boot_start, _f_boot_size, 0}, \
    /* 分区2：片内Flash-conf分区，偏移64KB，大小4KB， 存储配置文件 */           \
    {FAL_PART_MAGIC_WORD, "conf","onchip",_f_conf_start,_f_conf_size, 0}, \
    /* 分区3：Flash-app分区，偏移70KB，大小186KB，存放应用程序 */       \
    {FAL_PART_MAGIC_WORD, "app", "onchip", _f_app_start, _f_app_size, 0}, \
    /* 分区4：Flash-fdb分区，偏移256KB，大小256KB，存储数据库 */   \
    {FAL_PART_MAGIC_WORD, "fdb", "onchip", _f_fdb_start, _f_fdb_size, 0}, \
}
#endif /* FAL_PART_HAS_TABLE_CFG */

#endif /* _FAL_CFG_H_ */
