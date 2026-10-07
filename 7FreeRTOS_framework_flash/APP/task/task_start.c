
#include "framework.h"


extern void create_task_of_host( void );
extern void create_task_of_listen( void );
extern void create_task_of_ctrl( void );

void cslock_init(void);
void myprintf( char* format, ... );




void task_start_entry(void *arg)
{
	printf("%s\r\n",__FUNCTION__);
	
	//初始化 sysprintf
	cslock_init();
	//创建其他任务
	create_task_of_host();
	create_task_of_listen();
	create_task_of_ctrl();
	
	
	
	//-----------------------------------------
	//Flash编程
	//SWM320RET7 512KB
	//SECTOR 大小为 4KB		,每次擦除操作，尺寸为4KB
	//FLASH 位宽为 128bit ,每次编程操作，尺寸为16B，必须保证 4 个 WORD（128 位）对齐
	//void FLASH_Erase(uint32_t addr);  擦除
	//int FLASH_Write(uint32_t addr, uint32_t buff[], uint32_t count);
	
	//flash的特性:读/写/擦除
	//读:在swm320里面，直接用指针读取就可以了,按字去读
	//写:称为编程操作，只能写0，不能写1。
	//擦除:擦除一个扇区，会把扇区里面的数据，全部变成0xFF 
	
	//读取 _f_conf_start 的数据4*4 = 16 (对齐到4字节) 
	uint8_t buf[16];           //分配一个16字节缓冲区，在函数内部（栈区）
	
	uint32_t* dst = ( uint32_t* )buf;    //buf缓冲区目标位置指针 32/8 = 4字节
	volatile uint32_t* src = ( uint32_t* )(_f_conf_start);    //数据源头指针
	uint32_t len = sizeof( buf )/4;	    //计算buf中有多少个4字节， 得出复制多少个字
	
	int i = 0;
	for( i=0;i<len;i++ )       //遍历buf， 将从_f_conf_start数据源头指针，开始的数据，复制到buf
	{
		*dst = *(src+i);
		dst++;
	}
	for( i=0;i<sizeof( buf );i++ )    //打印buf缓冲区的内容
	{
		sysprintf("buf[%d] = %c \r\n",i,buf[i] );
	}
	
	//修改数据
	for( i=0;i<sizeof( buf );i++ )
	{
		buf[i] = 'a'+i;
	}
	
	//擦除这一个扇区 4096 B
	FLASH_Erase( _f_conf_start );
	//重新写回flash
	FLASH_Write( _f_conf_start,(uint32_t*)buf,len );
	
	//步骤：读取数据到缓冲区，修改缓冲区中的数据，擦除flash，写flash
	
	while(1)
	{
		
		vTaskDelay( pdMS_TO_TICKS( 10 ) );
		
	}
	
}

static TaskHandle_t TaskHandle_start;


void create_task_of_start( void )
{
	printf("%s\r\n",__FUNCTION__);
	//创建任务
	xTaskCreate( task_start_entry, (const char *)"start", 1024, NULL, 2, &TaskHandle_start );
}


/*----------------------------------------------------------------------------*/


static SemaphoreHandle_t cslock;
static int flag_init = 0;				//描述锁是否已经初始化了
//初始化锁
void cslock_init(void)
{
	if( flag_init == 1 ) return;
	cslock = xSemaphoreCreateMutex();
	if( cslock == NULL )
	{//说明不成功
		printf( "err:%s\r\n",__func__ );
		while(1);
	}
	flag_init = 1;
}

void cslock_get(void)
{
	if( flag_init == 0 ) return;
	
	//获取cslock，最多等待100个tick，超时，则返回错误；成功，返回TRUE
	int ret = xSemaphoreTake( cslock,0xFFFFFFFF );
	if( ret == pdTRUE )
	{//说明，成功获取了
	
	}
	else
	{//失败
		
	}
}

void cslock_free(void)
{
	if( flag_init == 0 ) return;
	xSemaphoreGive( cslock );
}


void sysprintf( char* format, ... )
{
	if( flag_init == 0 ) return;
	
	char buf[256];		//最大可以打印的字符串长度
	
	va_list v_args;
	va_start( v_args,format );
	(void)vsnprintf( buf,sizeof(buf),format,v_args );
	
	va_end( v_args );
	
	//获取锁
	cslock_get();
	//使用互斥资源,因为printf最终调用到uart,指向同一个硬件资源，好几个任务同时使用，是不行的。
	printf("%s",buf);
	//释放锁
	cslock_free();
}

