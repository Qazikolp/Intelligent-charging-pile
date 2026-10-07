
#include "work.h"
#include "led_board.h"

/*
保险丝怎么检测

检测保险丝，实际上就是检测单位时间内，光耦输出的脉冲数量，是否合理

我们采用查询方式检测脉冲，(有可能丢脉冲)
单位时间内，脉冲数小于10，我们认为不合理，否则 认为合理

*/

static uint8_t fuse_err = 0;	//=0，说明正常，=1 说明保险丝异常

static void init( void )
{
	gpio->set_mode( IO_Fuse,PIN_MODE_INPUT );
}


//查看保险丝是否异常
static int cat_err( int road )
{
	road = road;
	return fuse_err;
}

static uint32_t otick = 0;
static uint32_t otick_sec = 0;

static uint32_t pulse = 50;	//用于记录单位时间内的脉冲数

static void work(void)
{
	static uint8_t sta = 0;
	
	
	uint32_t ntick = get_sys_ticks();
	
	//开机后数秒之内，不检测
	if( ntick < 2000 ) return;
	
	
	//控制访问的频率，达到滤波的效果
	if( (ntick - otick) <5 ) return;
	otick = ntick;
	
	int val = gpio->read( IO_Fuse );
	switch( sta )
	{
		case 0://等待IO为低电平
			if( val == 0 )
				sta++;
			break;
	
		case 1://等待IO为高电平
			if( val == 1 )
			{
				sta = 0;
				pulse++;
			}
			break;
		
		default:
			sta = 0;
			break;
	}
	
	//控制1秒钟访问一边
	if( ( ntick - otick_sec ) < 1000 ) return;
	otick_sec = ntick;
	
	if( pulse <10 )
	{//认为保险丝异常
		fuse_err = 1;
		
		//熄灭fuse led
		led_board_opt->set_fuse( __OFF );
	
		sysprintf( "fuse_pulse:%d\r\n",pulse );
	}
	else
	{//保险丝正常
		fuse_err = 0;
		//点亮fuse led
		led_board_opt->set_fuse( __ON );
	}
	
	
	
	pulse = 0;
	sta = 0;
	
	
}	

static fuse_t do_fuse={
	.init = init,
	.cat_err = cat_err,
	.work = work,
	
};

fuse_t* fuse = &do_fuse;






