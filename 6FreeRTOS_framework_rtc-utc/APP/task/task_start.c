
#include "framework.h"


extern void create_task_of_host( void );
extern void create_task_of_listen( void );
extern void create_task_of_ctrl( void );

void cslock_init(void);
void myprintf( char* format, ... );




void task_start_entry(void *arg)
{
	printf("%s\r\n",__FUNCTION__);
	
	//初始化 myprintf
	cslock_init();
	//创建其他任务
	create_task_of_host();
	create_task_of_listen();
	create_task_of_ctrl();
	
	//-----------------------------------------
	//初始化RTC 
	utc_t dt;
	dt.year = 2025;
	dt.month = 5;
	dt.day = 9;
	dt.hour = 0;
	dt.minute = 00;
	dt.second = 0;
	
	rtc->init( &dt );  //初始化rtc  设置rtc墙上时间
	rtc->start();
	
	
	utc_t* nowdt = rtc->get_dt();    //读取rtc墙上时间
	
	rtc->read( nowdt );             //读取当前墙上时间
	//-----------------------------------------
	//utc转换为linux时间戳进行运算
	
	utc_t utc1;
	utc_t utc2;
	uint64_t timestamp = 0;
	
	//设置utc1
	utc1.year = 2025;
	utc1.month = 5;
	utc1.day = 9;
	utc1.hour = 0;
	utc1.minute = 0;
	utc1.second = 0;
	
	//将utc1转换为linux时间戳
	timestamp = utc_opt->utc_to_timestamp( &utc1 );
	
	//linux时间戳加上2天1小时1分1秒
	timestamp += ( 2*24*60*60 );			//加2天
	timestamp += (    1*60*60 );			//加1小时
	timestamp += (       1*60 );			//加1分钟
	timestamp += (          1 );			//加1秒
	
	//将修改后的时间戳，转换为utc时间->utc2
	utc_opt->timestamp_to_utc( timestamp,&utc2 );
	
	//原本  2025-05-09 00:00:00
	//后来  2025-05-11 01:01:01
	utc_opt->print_utc_time( &utc2 );
	//-----------------------------------------

	//操作示例,参考 http://www.armsoc.cn/zxx/doc/charge/#8040304
	
	while(1)
	{
		
		
		
		
		vTaskDelay( pdMS_TO_TICKS( 1000 ) );
		
		
		
		
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

