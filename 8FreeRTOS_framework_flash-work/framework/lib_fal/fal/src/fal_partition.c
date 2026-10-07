#include "../inc/fal.h"
#include <string.h>
#include <stdlib.h>

/* 分区表魔术字，用于识别Flash中存储的分区表，固定标识 0x45503130 */
#define FAL_PART_MAGIC_WORD         0x45503130
#define FAL_PART_MAGIC_WORD_H       0x4550L
#define FAL_PART_MAGIC_WORD_L       0x3130L

/**
 * @brief 分区与Flash设备的缓存信息结构体
 */
struct part_flash_info
{
    const struct fal_flash_dev *flash_dev;  // 指向该分区所属的Flash设备对象
};

/**
 * @brief FAL分区表配置说明
 * 如果定义 FAL_PART_HAS_TABLE_CFG：使用代码里写死的分区表(fal_cfg.h中的FAL_PART_TABLE)
 * 如果关闭该宏：FAL会从Flash物理空间里动态读取分区表
 */
#ifdef FAL_PART_HAS_TABLE_CFG
/* 检查是否在 fal_cfg.h 定义了 FAL_PART_TABLE 分区表 */
#if !defined(FAL_PART_TABLE)
#error "You must defined FAL_PART_TABLE on 'fal_cfg.h'"
#endif

/* 不同编译器的段属性宏定义 */
#ifdef __CC_ARM                        /* ARM?Keil 编译器 */
    #define SECTION(x)                 __attribute__((section(x)))
    #define USED                       __attribute__((used))
#elif defined (__IAR_SYSTEMS_ICC__)    /* IAR编译器 */
    #define SECTION(x)                 @ x
    #define USED                       __root
#elif defined (__GNUC__)               /* GCC编译器 */
    #define SECTION(x)                 __attribute__((section(x)))
    #define USED                       __attribute__((used))
#else
    #error not supported tool chain
#endif /* __CC_ARM */

//USED static const struct fal_partition partition_table_def[] SECTION("FalPartTable") = FAL_PART_TABLE;
/* 编译期直接从宏展开生成静态分区表，来自fal_cfg.h */
static const struct fal_partition partition_table_def[] = FAL_PART_TABLE;
static const struct fal_partition *partition_table = NULL;

/* 分区?Flash设备缓存表，保存每个分区对应的flash_dev指针，避免频繁查找 */
static struct part_flash_info part_flash_cache[sizeof(partition_table_def) / sizeof(partition_table_def[0])] = { 0 };

#else /* FAL_PART_HAS_TABLE_CFG 关闭，从Flash介质读取分区表 */

#if !defined(FAL_PART_TABLE_FLASH_DEV_NAME)
#error "You must defined FAL_PART_TABLE_FLASH_DEV_NAME on 'fal_cfg.h'"
#endif
#if !defined(FAL_PART_TABLE_END_OFFSET)
#error "You must defined FAL_PART_TABLE_END_OFFSET on 'fal_cfg.h'"
#endif

static struct fal_partition *partition_table = NULL;
static struct part_flash_info *part_flash_cache = NULL;

#endif /* FAL_PART_HAS_TABLE_CFG */

static uint8_t init_ok = 0;                 // FAL分区模块初始化标记，1=初始化完成
static size_t partition_table_len = 0;       // 分区表条目总数量

/**
 * @brief 打印全部分区表信息，调试用，输出分区名、所属Flash设备、偏移、大小
 */
void fal_show_part_table(void)
{
    char *item1 = "name", *item2 = "flash_dev";
    size_t i, part_name_max = strlen(item1), flash_dev_name_max = strlen(item2);
    const struct fal_partition *part;

    /* 遍历获取名字最大长度，格式化输出对齐用 */
    if (partition_table_len)
    {
        for (i = 0; i < partition_table_len; i++)
        {
            part = &partition_table[i];
            if (strlen(part->name) > part_name_max)
            {
                part_name_max = strlen(part->name);
            }
            if (strlen(part->flash_name) > flash_dev_name_max)
            {
                flash_dev_name_max = strlen(part->flash_name);
            }
        }
    }
    log_i("==================== FAL partition table ====================");
    log_i("| %-*.*s | %-*.*s |   offset   |    length  |", part_name_max, FAL_DEV_NAME_MAX, item1, flash_dev_name_max,
            FAL_DEV_NAME_MAX, item2);
    log_i("-------------------------------------------------------------");
    for (i = 0; i < partition_table_len; i++)
    {
#ifdef FAL_PART_HAS_TABLE_CFG
        part = &partition_table[i];
#else
        part = &partition_table[partition_table_len - i - 1];
#endif
        log_i("| %-*.*s | %-*.*s | 0x%08lx | 0x%08x |", part_name_max, FAL_DEV_NAME_MAX, part->name, flash_dev_name_max,
                FAL_DEV_NAME_MAX, part->flash_name, part->offset, part->len);
    }
    log_i("=============================================================");
}

/**
 * @brief 校验分区表，填充part_flash_cache缓存
 * @param table 分区表数组
 * @param len 分区条目数量
 * @retval 0成功，?1分区越界，?2内存分配失败
 */
static int check_and_update_part_cache(const struct fal_partition *table, size_t len)
{
    const struct fal_flash_dev *flash_dev = NULL;
    size_t i;

#ifndef FAL_PART_HAS_TABLE_CFG
    /* 如果是动态从Flash读取分区表，释放旧缓存，重新申请内存 */
    if (part_flash_cache)
    {
        FAL_FREE(part_flash_cache);
    }
    part_flash_cache = FAL_MALLOC(len * sizeof(struct part_flash_info));
    if (part_flash_cache == NULL)
    {
        log_e("Initialize failed! No memory for partition table cache");
        return -2;
    }
#endif

    /* 循环校验每一条分区配置 */
    for (i = 0; i < len; i++)
    {
        /* 根据分区内flash_name查找已经注册的Flash设备 */
        flash_dev = fal_flash_device_find(table[i].flash_name);
        if (flash_dev == NULL)
        {
            log_d("Warning: Do NOT found the flash device(%s).", table[i].flash_name);
            continue;
        }
        /* 判断分区偏移是否超出Flash物理总长度，防止越界访问 */
        if (table[i].offset >= (long)flash_dev->len)
        {
            log_e("Initialize failed! Partition(%s) offset address(%ld) out of flash bound(<%d).",
                    table[i].name, table[i].offset, flash_dev->len);
            partition_table_len = 0;
            return -1;
        }
        /* 缓存该分区对应的Flash设备指针 */
        part_flash_cache[i].flash_dev = flash_dev;
    }
    return 0;
}

/**
 * @brief FAL分区模块初始化
 * @return 成功返回分区总条目数；失败返回0
 * @note 如果FAL_PART_HAS_TABLE_CFG开启：直接使用代码静态分区表
 *       如果关闭：从Flash介质搜索魔术字，动态加载分区表
 */
int fal_partition_init(void)
{
    if (init_ok)
    {
        return partition_table_len;
    }

#ifdef FAL_PART_HAS_TABLE_CFG
    /* 使用代码编译时定义的静态分区表 */
    partition_table = &partition_table_def[0];
    partition_table_len = sizeof(partition_table_def) / sizeof(partition_table_def[0]);
#else
    /* 关闭静态分区表：从Flash里面搜索并加载分区表 */
    long part_table_offset = FAL_PART_TABLE_END_OFFSET;
    size_t table_num = 0, table_item_size = 0;
    uint8_t part_table_find_ok = 0;
    uint32_t read_magic_word;
    fal_partition_t new_part = NULL;
    size_t i;
    const struct fal_flash_dev *flash_dev = NULL;

    /* 获取存放分区表的Flash设备 */
    flash_dev = fal_flash_device_find(FAL_PART_TABLE_FLASH_DEV_NAME);
    if (flash_dev == NULL)
    {
        log_e("Initialize failed! Flash device (%s) NOT found.", FAL_PART_TABLE_FLASH_DEV_NAME);
        goto _exit;
    }

    /* 校验分区表结束偏移不能超出Flash容量 */
    if (part_table_offset < 0 || part_table_offset >= (long) flash_dev->len)
    {
        log_e("Setting partition table end offset address(%ld) out of flash bound(<%d).", part_table_offset, flash_dev->len);
        goto _exit;
    }

    table_item_size = sizeof(struct fal_partition);
    new_part = (fal_partition_t)FAL_MALLOC(table_item_size);
    if (new_part == NULL)
    {
        log_e("Initialize failed! No memory for table buffer.");
        goto _exit;
    }

    /* 从指定位置向前搜索分区表魔术字 */
    {
        uint8_t read_buf[64];
        part_table_offset -= sizeof(read_buf);
        while (part_table_offset >= 0)
        {
            /* 读取一段Flash数据 */
            if (flash_dev->ops.read(part_table_offset, read_buf, sizeof(read_buf)) > 0)
            {
                /* 在读取缓冲区遍历查找分区魔术字 */
                for (i = 0; i < sizeof(read_buf) - sizeof(read_magic_word) + 1; i++)
                {
                    read_magic_word = read_buf[0 + i] + (read_buf[1 + i] << 8) + (read_buf[2 + i] << 16) + (read_buf[3 + i] << 24);
                    if (read_magic_word == ((FAL_PART_MAGIC_WORD_H << 16) + FAL_PART_MAGIC_WORD_L))
                    {
                        part_table_find_ok = 1;
                        part_table_offset += i;
                        log_d("Find the partition table on '%s' offset @0x%08lx.", FAL_PART_TABLE_FLASH_DEV_NAME,
                                part_table_offset);
                        break;
                    }
                }
            }
            else
            {
                /* Flash读失败，退出查找 */
                break;
            }
            if (part_table_find_ok)
            {
                break;
            }
            else
            {
                /* 没有找到魔术字，继续向前移动搜索窗口 */
                if (part_table_offset >= (long)sizeof(read_buf))
                {
                    part_table_offset -= sizeof(read_buf);
                    part_table_offset += (sizeof(read_magic_word) - 1);
                }
                else if (part_table_offset != 0)
                {
                    part_table_offset = 0;
                }
                else
                {
                    /* 已经到Flash头部，仍然没找到 */
                    break;
                }
            }
        }
    }

    /* 根据找到的魔术字位置，逐条读取全部分区条目 */
    while (part_table_find_ok)
    {
        memset(new_part, 0x00, table_num);
        /* 读取一条分区条目 */
        if (flash_dev->ops.read(part_table_offset - table_item_size * (table_num), (uint8_t *) new_part,
                table_item_size) < 0)
        {
            log_e("Initialize failed! Flash device (%s) read error!", flash_dev->name);
            table_num = 0;
            break;
        }
        /* 魔术字不匹配代表分区表读取完毕 */
        if (new_part->magic_word != ((FAL_PART_MAGIC_WORD_H << 16) + FAL_PART_MAGIC_WORD_L))
        {
            break;
        }
        /* realloc扩容分区表内存 */
        partition_table = (fal_partition_t) FAL_REALLOC(partition_table, table_item_size * (table_num + 1));
        if (partition_table == NULL)
        {
            log_e("Initialize failed! No memory for partition table");
            table_num = 0;
            break;
        }
        memcpy(partition_table + table_num, new_part, table_item_size);
        table_num++;
    };

    if (table_num == 0)
    {
        log_e("Partition table NOT found on flash: %s (len: %d) from offset: 0x%08x.", FAL_PART_TABLE_FLASH_DEV_NAME,
                FAL_DEV_NAME_MAX, FAL_PART_TABLE_END_OFFSET);
        goto _exit;
    }
    else
    {
        partition_table_len = table_num;
    }
#endif /* FAL_PART_HAS_TABLE_CFG */

    /* 校验分区，建立分区?Flash设备缓存 */
    if (check_and_update_part_cache(partition_table, partition_table_len) != 0)
    {
        goto _exit;
    }

    init_ok = 1;

_exit:
#if FAL_DEBUG
    fal_show_part_table();
#endif

#ifndef FAL_PART_HAS_TABLE_CFG
    if (new_part)
    {
        FAL_FREE(new_part);
    }
#endif /* !FAL_PART_HAS_TABLE_CFG */

    return partition_table_len;
}

/**
 * @brief 根据分区名字查找分区对象
 * @param name 分区名字字符串
 * @return 成功返回分区结构体指针；NULL代表未找到
 */
const struct fal_partition *fal_partition_find(const char *name)
{
    assert(init_ok);
    size_t i;
    for (i = 0; i < partition_table_len; i++)
    {
        if (!strcmp(name, partition_table[i].name))
        {
            return &partition_table[i];
        }
    }
    return NULL;
}

/**
 * @brief 通过分区指针快速获取对应的Flash设备（使用缓存，避免字符串查找）
 * @param part 分区指针
 * @return fal_flash_dev指针
 */
static const struct fal_flash_dev *flash_device_find_by_part(const struct fal_partition *part)
{
    assert(part >= partition_table);
    assert(part <= &partition_table[partition_table_len - 1]);
    return part_flash_cache[part - partition_table].flash_dev;
}

/**
 * @brief 获取分区表数组与条目数量
 * @param len 输出参数，返回分区条目个数
 * @return 分区表数组首地址
 */
const struct fal_partition *fal_get_partition_table(size_t *len)
{
    assert(init_ok);
    assert(len);
    *len = partition_table_len;
    return partition_table;
}

/**
 * @brief 临时设置内存中的分区表，仅内存生效，重启丢失
 * @param table 用户传入分区表
 * @param len 分区条目数量
 */
void fal_set_partition_table_temp(struct fal_partition *table, size_t len)
{
    assert(init_ok);
    assert(table);
    check_and_update_part_cache(table, len);
    partition_table_len = len;
    partition_table = table;
}

/**
 * @brief 从逻辑分区读取数据
 * @param part 分区指针
 * @param addr 分区内相对偏移（逻辑地址，不是Flash物理偏移）
 * @param buf 接收数据缓冲区
 * @param size 读取字节数
 * @retval >=0 实际读取字节；?1出错
 */
int fal_partition_read(const struct fal_partition *part, uint32_t addr, uint8_t *buf, size_t size)
{
    int ret = 0;
    const struct fal_flash_dev *flash_dev = NULL;

    assert(part);
    assert(buf);
    /* 判断读取范围是否超出分区总长度，防止越界 */
    if (addr + size > part->len)
    {
        log_e("Partition read error! Partition address out of bound.");
        return -1;
    }

    flash_dev = flash_device_find_by_part(part);
    if (flash_dev == NULL)
    {
        log_e("Partition read error! Don't found flash device(%s) of the partition(%s).", part->flash_name, part->name);
        return -1;
    }
    /* 转换为Flash设备物理偏移 = 分区在Flash上的offset + 分区内部逻辑addr */
    ret = flash_dev->ops.read(part->offset + addr, buf, size);
    if (ret < 0)
    {
        log_e("Partition read error! Flash device(%s) read error!", part->flash_name);
    }
    return ret;
}

/**
 * @brief 向逻辑分区写入数据
 * @param part 分区指针
 * @param addr 分区内相对逻辑偏移
 * @param buf 待写数据源
 * @param size 写入字节数
 * @retval >=0成功写入字节；?1出错
 * @note Flash写之前必须保证对应区域已经擦除为0xFF
 */
int fal_partition_write(const struct fal_partition *part, uint32_t addr, const uint8_t *buf, size_t size)
{
    int ret = 0;
    const struct fal_flash_dev *flash_dev = NULL;

    assert(part);
    assert(buf);
    if (addr + size > part->len)
    {
        log_e("Partition write error!  Don't found flash device(%s) of the partition(%s).", part->flash_name, part->name);
        return -1;
    }

    flash_dev = flash_device_find_by_part(part);
    if (flash_dev == NULL)
    {
        log_e("Partition write error!  Don't found flash device(%s) of the partition(%s).", part->flash_name, part->name);
        return -1;
    }
    ret = flash_dev->ops.write(part->offset + addr, buf, size);
    if (ret < 0)
    {
        log_e("Partition write error! Flash device(%s) write error!", part->flash_name);
    }
    return ret;
}

/**
 * @brief 擦除分区指定区域
 * @param part 分区指针
 * @param addr 分区内逻辑偏移
 * @param size 需要擦除字节
 * @retval >=0实际擦除字节；?1出错
 * @note 底层会按照Flash硬件块粒度向上对齐
 */
int fal_partition_erase(const struct fal_partition *part, uint32_t addr, size_t size)
{
    int ret = 0;
    const struct fal_flash_dev *flash_dev = NULL;

    assert(part);
    if (addr + size > part->len)
    {
        log_e("Partition erase error! Partition address out of bound.");
        return -1;
    }

    flash_dev = flash_device_find_by_part(part);
    if (flash_dev == NULL)
    {
        log_e("Partition erase error! Don't found flash device(%s) of the partition(%s).", part->flash_name, part->name);
        return -1;
    }
    ret = flash_dev->ops.erase(part->offset + addr, size);
    if (ret < 0)
    {
        log_e("Partition erase error! Flash device(%s) erase error!", part->flash_name);
    }
    return ret;
}

/**
 * @brief 擦除整个分区全部内容
 * @param part 分区指针
 * @retval >=0成功擦除字节；?1出错
 */
int fal_partition_erase_all(const struct fal_partition *part)
{
    return fal_partition_erase(part, 0, part->len);
}
