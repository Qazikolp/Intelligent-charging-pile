#include "framework.h"

/*

drvp_rtc实现思路
	静态分配一个变量 m_rtc_dt
	里面存储了rtc的日期、时间等信息。
	
	在rtc的秒中断里面，更新这个变量。
	
	drvp_rtc提供该变量的读函数

(1)初始化RTC，要开启秒中断
(2)开始函数
(3)停止函数
(4)中断服务函数
	每次进入秒中断，更新变量m_rtc_dt
(5)分享该变量


*/
/*-------------------------------------------------------------------------*/
static rtc_dt_t m_rtc_dt;    //静态分配一个变量 m_rtc_dt

//初始化函数
//输入参数,年月日时分秒，
static void init( rtc_dt_t* dt )
{
	RTC_InitStructure RTC_initStruct;
	
	RTC_initStruct.Year = dt->Year;
	RTC_initStruct.Month = dt->Month;
	RTC_initStruct.Date = dt->Date;
	RTC_initStruct.Hour = dt->Hour;
	RTC_initStruct.Minute = dt->Minute;
	RTC_initStruct.Second = dt->Second;
	
	RTC_initStruct.SecondIEn = 1;		//秒中断
	RTC_initStruct.MinuteIEn = 0;		//分中断
	
	RTC_Init( RTC, &RTC_initStruct);
	
}

//启动RTC
static void start(void)
{
	RTC_Start( RTC );
}

//停止RTC
static void stop(void)
{
	RTC_Stop( RTC );
}

//中断服务函数                

void RTC_Handler(void)
{

	if( RTC_IntSecondStat(RTC) )          //秒中断
	{//成立，说明触发了秒中断           
		
		RTC_IntSecondClr( RTC );
		
		//更新变量 m_rtc_dt
		RTC_GetDateTime( RTC, &m_rtc_dt );
	}
	
	if( RTC_IntMinuteStat(RTC) )
	{
		RTC_IntMinuteClr( RTC );
		
		//执行分中断的代码
		
	}
	
}

//获取rtc_dt变量
static rtc_dt_t* get_dt(void)
{
	return &m_rtc_dt;
}

//从硬件读取rtc_dt
static void read( rtc_dt_t* dt )
{
	RTC_GetDateTime( RTC, dt );
}

static drvp_rtc_t do_drvp_rtc={
	.init = init,
	.start = start,
	.stop = stop,
	.get_dt = get_dt,
	.read = read,
	
};

drvp_rtc_t* drvp_rtc = &do_drvp_rtc;

