#include "framework.h"

extern drvp_rtc_t* drvp_rtc;

//初始化函数
static void init( rtc_dt_t* dt )
{
	drvp_rtc->init( dt );
}

//启动RTC
static void start(void)
{
	drvp_rtc->start();
}

//停止RTC
static void stop(void)
{
	drvp_rtc->stop();
}									

//获取rtc_dt变量
static rtc_dt_t* get_dt(void)
{
	return drvp_rtc->get_dt();
}	

//从硬件读取rtc_dt
static void read( rtc_dt_t* dt )
{
	drvp_rtc->read( dt );
}

static drv_rtc_t do_drv_rtc={
	.init = init,
	.start = start,
	.stop = stop,
	.get_dt = get_dt,
	.read = read,
};

drv_rtc_t* rtc = &do_drv_rtc;


