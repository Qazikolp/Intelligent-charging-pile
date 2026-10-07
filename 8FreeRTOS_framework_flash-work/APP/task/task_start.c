
#include "work.h"


extern void create_task_of_host( void );
extern void create_task_of_listen( void );
extern void create_task_of_ctrl( void );
extern int write_flag;
void cslock_init(void);
void myprintf( char* format, ... );

extern static  storage_t m_storage;

storage_t * storage = &m_storage;


void task_start_entry(void *arg)
{
	printf("%s\r\n",__FUNCTION__);
	
	//初始化 sysprintf
	cslock_init();
	//创建其他任务
	create_task_of_host();
	create_task_of_listen();
	create_task_of_ctrl();
	
	storage -> bootcode = 0;
	write_flag = 0;
	work_data_load();
	
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

