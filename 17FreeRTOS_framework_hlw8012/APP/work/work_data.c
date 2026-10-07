#include "work.h"


static storage_t m_storage;

storage_t* storage = &m_storage;


void work_data_load(void)
{
	
	int write_flag = 0;	//描述是否需要执行写入操作
	
	//fal初始化
	fal_init();
	
	//获取想要操作的fal分区
	const struct fal_partition * ptt_conf = fal_partition_find( "conf" );
	if( NULL == ptt_conf )
	{//成立，说明获取失败
		sysprintf("err:%s,%d\r\n",__func__,__LINE__);
		while(1);
	}
	
	//从这一个分区里面读取数据
	int len = fal_partition_read( ptt_conf, 0, (uint8_t*)(&m_storage), sizeof( storage_t ) );
	if( len != sizeof( storage_t ) )
	{//说明读取出错
		sysprintf("err:%s,%d\r\n",__func__,__LINE__);
		while(1);
	}
	
		
	//判断是否为第一次开机
	if( storage->bootcode != BootCode )
	{//成立，说明是第一次开机
		//初始化一些参数
		sysprintf("Sys is first boot\r\n");
		storage->bootcode = BootCode;
		
		
		write_flag ++;
	}
	else
	{//否则，说明不是第一次开机
		sysprintf("Sys is not first boot\r\n");
	}
	
	//如果有数据变化，则执行写入操作
	if( write_flag != 0 )
	{
		//擦除
		fal_partition_erase( ptt_conf, 0,  sizeof( storage_t ) );
		//写入
		fal_partition_write( ptt_conf, 0, (uint8_t*)storage, sizeof( storage_t ) );
		sysprintf("write data to flash \r\n");
	}

}





