
#include "work.h"
#include "network.h"


extern void create_task_of_host( void );
extern void create_task_of_listen( void );
extern void create_task_of_ctrl( void );

void cslock_init(void);
void myprintf( char* format, ... );




void task_start_entry(void *arg)
{
	printf("%s\r\n",__FUNCTION__);
	
	
	//初始化uart0
	uart->close( _e_uart0 );
	uart->init( _e_uart0,115200 );
	uart->open( _e_uart0 );
	
	//延时1s，避免后续代码发生异常后，芯片无法使用DAP-Link烧录代码
	//如果存在溢出清空，或者有异常访问，导致芯片死机，无法使用DAP-Link烧写程序。
	//需要按下主板的BOOT键+Reset键，先松开Reset键，再松开BOOT键。进入ISP模式。
	//使用官方的ISP工具，串口(CH340)清除flash，可以救机。
	vTaskDelay( pdMS_TO_TICKS( 1000 ) );
	
	//初始化 sysprintf
	cslock_init();
	
	work_data_load();
	
	hc595_pin_t LED_BUS = { IO_HC_DAT,IO_HC_CLK,IO_HC_UD,0 };
	hc595_opt->init( &LED_BUS );
	
	uint8_t led[2];
	led[0] = 0xFF;
	led[1] = 0XFF;
	hc595_opt->write( &LED_BUS,led,sizeof( led ) );
	hc595_opt->update( &LED_BUS );
	
	
	
	
	//创建其他任务
	create_task_of_host();
	create_task_of_listen();
	create_task_of_ctrl();
	
	
	

	while(1)
	{
//		network_entry();
		
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



	
/*
AT

OK
------
AT+RESET
OK
^boot.rom.'v....'!\n
RDY

^MODE: 17,17

+E_UTRAN Service

+CGEV: ME PDN ACT 1

------
读取模块厂家信息
AT+CGMI

+CGMI: "AirM2M"

OK

------
读取模块的固件信息
AT+CGMR

+CGMR: "AirM2M_780EP_V1010_LTE_AT"

OK
------
查询物联网卡/SIM卡有没有插好
AT+CPIN?

+CPIN: READY

OK
------
查询信号质量
AT+CSQ

+CSQ: 22,0

OK
------
查询网络的注册状态
AT+CREG?

+CREG: 0,1

OK
------
查询附着GPRS网络
AT+CGATT?

+CGATT: 1

OK

------
设置透明传输
AT+CIPMODE=1


OK
------
设置模块为单链接
AT+CIPMUX=0

OK
------
自动获取APN

AT+CSTT="","",""

OK
------
激活移动场景,激活之后，可以去获取IP
AT+CIICR

OK
------
查询IP
AT+CIFSR

10.191.209.135
------
连接TCP服务器
AT+CIPSTART="TCP","www.armsoc.cn","9002"

AT+CIPSTART="TCP","www.armsoc.cn","9002"

OK

CONNECT




*/
