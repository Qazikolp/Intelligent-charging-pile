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
static utc_t m_rtc_dt;

//初始化函数
//输入参数,年月日时分秒，
static void init( utc_t* dt )
{
	RTC_InitStructure RTC_initStruct;
	
	RTC_initStruct.Year = dt->year;
	RTC_initStruct.Month = dt->month;
	RTC_initStruct.Date = dt->day;
	RTC_initStruct.Hour = dt->hour;
	RTC_initStruct.Minute = dt->minute;
	RTC_initStruct.Second = dt->second;
	
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

void rtc_time_to_utc( RTC_DateTime* rdt,utc_t* ut )
{
	ut->year = rdt->Year;
	ut->month = rdt->Month;
	ut->day = rdt->Date;
	ut->hour = rdt->Hour;
	ut->minute = rdt->Minute;
	ut->second = rdt->Second;

}


void RTC_Handler(void)
{
	RTC_DateTime dateTime ;
	if( RTC_IntSecondStat(RTC) )
	{//成立，说明触发了秒中断
		
		RTC_IntSecondClr( RTC );
		
		//更新变量 m_rtc_dt
		RTC_GetDateTime( RTC, &dateTime );
		
		//转换为utc时间
		rtc_time_to_utc( &dateTime,&m_rtc_dt );
	}
	
	if( RTC_IntMinuteStat(RTC) )
	{
		RTC_IntMinuteClr( RTC );
		
		//执行分中断的代码
		
	}
	
}

//获取rtc_dt变量
static utc_t* get_dt(void)
{
	return &m_rtc_dt;
}

//从硬件读取rtc_dt
static int read( utc_t* dt )
{
	int day = 0;
	RTC_DateTime dateTime ;
	
	RTC_GetDateTime( RTC, &dateTime );
	//转换为utc时间
	rtc_time_to_utc( &dateTime, dt );
	
	switch( dateTime.Day )
	{
		case RTC_SUN: day = 0; break;
		case RTC_MON: day = 1; break;
		case RTC_TUE: day = 2; break;
		case RTC_WED: day = 3; break;
		case RTC_THU: day = 4; break;
		case RTC_FRI: day = 5; break;
		case RTC_SAT: day = 6; break;
		default:
			break;
	}
	
	return day;
}



static drvp_rtc_t do_drvp_rtc={
	.init = init,
	.start = start,
	.stop = stop,
	.get_dt = get_dt,
	.read = read,
	
};

drvp_rtc_t* drvp_rtc = &do_drvp_rtc;

