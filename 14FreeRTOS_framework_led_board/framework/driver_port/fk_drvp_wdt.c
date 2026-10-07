#include "framework.h"

/*
WDT，看门狗
我们设置看门狗的饥饿时间为n秒，
如果n秒都没有去喂狗，就会出发异常，
触发异常之后，有2种结果，
	一种是响应异常，进入中断服务函数。
	一种是直接系统复位。
	
	比如，我写的代码，有漏洞，执行到这个地方，死机了(长时间不喂狗)，看门狗会引发异常，系统重启。
	
	
*/

//初始化看门狗
static void init( uint32_t tick )
{
	//判断tick的合法性
	if( tick >= 4294967295 )
	{
		tick = 4294967295;
		sysprintf(" tick is to big\r\n");
	}
	//初始化WDT
	WDT_Init( WDT, tick, WDT_MODE_RESET );
	
}
//喂狗
static void feed(void)
{
	WDT_Feed( WDT );
}
//看门狗启动
static void start(void)
{
	WDT_Start( WDT );
}
//看门狗停止
static void stop(void)
{
	WDT_Stop( WDT );
}

static drvp_wdt_t do_drvp_wdt = {
	.init = init,
	.feed = feed,
	.start = start,
	.stop = stop,
};

drvp_wdt_t* drvp_wdt = &do_drvp_wdt;





