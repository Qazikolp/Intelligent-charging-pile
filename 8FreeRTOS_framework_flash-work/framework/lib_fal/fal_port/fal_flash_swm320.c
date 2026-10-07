/*
 * Copyright (c) 2024, Synwit, <linzh@synwit.cn>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

//#include <fal.h>
//#include <string.h>
//#include "SWM341.h"

#include "framework.h"

/* note: align 必须是 2 的指数幂 */
/* 对齐校验、向上/向下对齐工具宏，对齐值必须是2的整数次幂 */

/**
 * @brief  判断数值是否按照指定字节数对齐
 * @param  val: 待校验数值
 * @param  align: 对齐字节数(2^n)
 * @retval 1-对齐；0-不对齐
 */
#define IS_ALIGN(val, align)        ( ( !( (val) & ( (align) - 1 ) ) ) ) //&& (val) )

/**
 * @brief 向上对齐，往大的方向补齐到align的整数倍
 * @param  val:原始数值
 * @param  align:对齐字节数
 */
#define ALIGN_UP(val, align)        ( ( (val) + (align) - 1 ) & ~( (align) - 1 ) )

/**
 * @brief 向下对齐，往小的方向取align的整数倍
 * @param  val:原始数值
 * @param  align:对齐字节数
 */
#define ALIGN_DOWN(val, align)      ( (val) & ~( (align) - 1 ) )


/**
 * @brief  Flash设备初始化接口(FAL回调函数)
 * @retval 0成功
 */
 //初始化函数（片内flash不需要，走个形式）
static int init(void)
{
    /* 片内Flash不需要额外初始化，空实现即可 */
    return 0;
}

/**
 * @brief  FAL底层-Flash读接口
 * @param  offset: 相对于Flash起始基地址的偏移(字节)
 * @param  buf: 读出数据存放缓冲区
 * @param  size: 需要读取的字节长度
 * @retval 成功返回读取字节数；失败返回-1
 */
//读函数
static int read(long offset, uint8_t *buf, size_t size)
{
    /* 计算出真实的物理访问地址 = Flash基地址 + 偏移 */
    long addr = onchip_flash.addr + offset;

    /* 逐字节读取Flash数据 */
    for (size_t i = 0; i < size; i++, buf++, addr++)
    {
        *buf = *(uint8_t *)addr;
    }

    return size;
}

/**
 * @brief  FAL底层-Flash编程(写)接口
 * @param  offset: Flash内部偏移地址
 * @param  buf: 待写入数据缓冲区
 * @param  size: 写入字节长度
 * @retval 成功返回写入字节数；失败-1
 * @note  此处硬件要求：写入地址、写入长度必须16字节对齐
 */

//写函数
static int write(long offset, const uint8_t *buf, size_t size)
{
    if (size == 0)
        return 0;
    
    /* 换算成单片机内存的真实物理地址 */
    long addr = onchip_flash.addr + offset;
    
    /* 校验：写入地址、写入长度必须16字节对齐，否则报错退出 */
    if (!IS_ALIGN(addr, 16) || !IS_ALIGN(size, 16))
    {
        printf("[FAL] [ERROR]: addr = 0x[%lx], size = 0x[%x]\r\n", addr, size);
        return -1;
    }
    
    /* 如果源缓冲区buf是4字节对齐，可以直接传给Flash写函数 */
    if (IS_ALIGN((uint32_t)buf, 4))
    {
        if (FLASH_RES_OK != FLASH_Write(addr, (uint32_t *)buf, size >> 2))
        {
            printf("[FAL] [ERROR]: addr = 0x[%lx], size = 0x[%x], buf = 0x[%p]\r\n", addr, size, buf);
            return -1;
        }
    }
    else 
    {
        /* 缓冲区未4字节对齐，使用临时对齐缓存中转，每次写入16字节 */
        uint32_t write_gran_buf[4] = {0};
        for (size_t i = 0; i < size / sizeof(write_gran_buf); )
        {
            /* 拷贝到对齐缓存 */
            memcpy(write_gran_buf, buf, sizeof(write_gran_buf));
            if (FLASH_RES_OK == FLASH_Write(addr, write_gran_buf, sizeof(write_gran_buf) >> 2))
            {
                addr += sizeof(write_gran_buf);
                buf += sizeof(write_gran_buf);
                ++i;
            }
        }
    }
    
    return size;
}

/**
 * @brief  FAL底层-Flash扇区擦除接口
 * @param  offset: Flash内部偏移
 * @param  size: 需要擦除字节长度
 * @retval 实际擦除字节数
 * @note  该芯片擦除粒度4KB，擦除起始地址向下4K对齐；擦除长度向上补齐4K整数倍
 */

//擦除函数，根据偏移地址和尺寸进行擦除
static int erase(long offset, size_t size)
{
    /* 擦除起始地址向下4KB对齐 */
    long addr = ALIGN_DOWN(onchip_flash.addr + offset, 4096);
    /* 擦除总长度向上补齐为4KB的整数倍 */
    size = ALIGN_UP(size, 4096);
    
    /* 循环逐4KB块擦除 */
    for (size_t i = 0; i < size; i += 4096, addr += 4096)
    {
        FLASH_Erase(addr);
    }
    
    return size;
}

/**
 * @brief  向FAL注册片内Flash设备实例
 * .name      : Flash设备名称，分区表内需要和该名称匹配
 * .addr      : Flash基地址偏移(相对于MCU总线地址)，此处0代表后续偏移自动加上片内Flash基址0x08000000
 * .len       : Flash总容量 512KB
 * .blk_size  : Flash擦除块大小，4KB
 * .ops       : 底层操作函数集 {init,read,write,erase}，第一个位置对应init传NULL就会调用上面init函数
 * .write_gran: Flash最小编程粒度128字节(FAL内部参数，FlashDB会参考这个值)
 */
//构建一个flash设备
const struct fal_flash_dev onchip_flash =
{
    .name       = "onchip",
    .addr       = 0,
    .len        = 512 * 1024,
    .blk_size   = 4 * 1024,
    .ops        = {NULL, read, write, erase},
    .write_gran = 128
};
