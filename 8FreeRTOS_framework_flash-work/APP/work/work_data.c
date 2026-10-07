#include "work.h"
static  storage_t m_storage;

storage_t * storage = &m_storage;

int write_flag = 0;  //写入操作执行flag

void work_data_load(void)
{
    //fal初始化
    fal_init();
    //获取想要操作的fal分区
    const struct fal_partition *ptt_conf = fal_partition_find("conf");
    if(ptt_conf == NULL)
    {   //说明获取失败
        sysprintf("err:%s,%d\r\n", __func__,__LINE__);
        while(1);
    }
    //从分区里面读数据
    int len = fal_partition_read(ptt_conf, 0, (uint8_t *)(&m_storage), sizeof(storage_t));
    if(len != sizeof(storage_t))
    {    //说明读取失败
        sysprintf("err:%s,%d\r\n", __func__,__LINE__);
        while(1);
    }
    //判断是否为第一次开机
    
    if(storage -> bootcode != BootCode)
    { //成立，说明是第一次开机
      //初始化一些参数  
        sysprintf("sys is frist boot\r\n");
        storage -> bootcode = BootCode;
        
        write_flag++;
    }
    else
    {  //否则，不是第一次开机
        sysprintf("sys is not frist boot\r\n");
    }
    //如果有数据变化，执行写入操作
    if(write_flag != 0)
    {
        //擦除
        fal_partition_erase(ptt_conf, 0, sizeof(storage_t));
        //写入
        fal_partition_write(ptt_conf, 0, (uint8_t *)(&m_storage), sizeof(storage_t));
        sysprintf("write data to flash\r\n");
    }
}


