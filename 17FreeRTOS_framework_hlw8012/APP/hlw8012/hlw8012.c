#include "hlw8012.h"

/*
hlw8012是一个电能计量芯片

这是一个积分芯片，固定周期采样，瞬时电压，瞬时电流,求乘积。
并且对乘积进行累加，加到一定程度(固定值)，则输出一个脉冲。

测量这个固定值的方法：
固定负载功率，测量HLW8012单位时间内输出的脉冲数。

通过对其脉冲计数，就可以知道周期内所消耗的电脑。
而1s钟所消耗的电能就是平均功率。


1,我们要知道插座有没有人插入，需要用到低功率检测
2,我们需要知道插座的平均功率，需要周期采样，求平均
3,我们需要定时1分钟，发出一分钟的电能消耗


冷风 [1]pulse = 00000014
低热 [1]pulse = 00000256

*/
//引脚映射表
static uint8_t pin_map[ MaxSock ]={ IO_HLW1,IO_HLW0 };   //原理图相反

//采样脉冲数
static uint32_t pulse[ MaxSock ]={ 0,0};

//记录最后一次脉冲计量时间
static uint32_t last_tick[ MaxSock ] = {0,0};

typedef struct{
	uint8_t road;		//第几路HLW芯片
	uint8_t pin;		//对应的具体stm IO序号
}dat_t;   //hlw芯片引脚参数结构体


static dat_t ArgDat[ MaxSock ];     //hlw引脚参数结构体变量

static void do_isr( void *args )    //中断服务回调函数
{
	dat_t* pdat = (dat_t*)args;     //结构体指针传参
	
	//判断road 合法性
	if( pdat->road >= MaxSock ) return;
	
	//采样脉冲数++
	pulse[ pdat->road ]++;
	
	//记录最后一次脉冲计量时间
	last_tick[ pdat->road ] = get_sys_tick_irq();  //中断内部可调用的获取滴答函数
	
}

//初始化函数
static void init( void )
{
	
	for( int road=0;road<MaxSock;road++ )
	{
		ArgDat[road].pin = pin_map[road];  //stm IO引脚体填入hlw引脚结构体
		ArgDat[road].road = road;          //1/2路
		
		
		//先设置为输入模式
		gpio->set_mode( pin_map[road],PIN_MODE_INPUT );
		//附加PIN_IRQ_MODE_FALLING下降沿中断
		gpio->attach_irq( pin_map[road],PIN_IRQ_MODE_FALLING,do_isr,&( ArgDat[road] ) );
		//使能中断
		gpio->irq_enable( pin_map[road],PIN_IRQ_ENABLE );

	}
	
}

//工作函数
static void work( void )
{
	static uint32_t otick_cyc =0 ;
	static uint32_t min_pulse[MaxSock] ={0,0};
	static uint32_t min_cntr = 0;
	
	
	uint32_t ntick = get_sys_ticks();    //获取目前系统滴答
	
	
	//检查用电器拔出事件
	for( int road=0;road<MaxSock;road++ )
	{
		//如果当前系统滴答减去上一次获取到脉冲的滴答大于3秒，说明插座拔出，时间尽可能短，避免偷电
		if( ( ntick - last_tick[road] ) >( 3000 ) )
		{//成立，则认为没有插入用电器
			//发送用电器拔出事件
			//TODO
		}
	}
	
	
	//每5s的周期采样脉冲数 
	if( (ntick - otick_cyc) >= 5000 )
	{
		otick_cyc = ntick;
		
		//打印本周期采样的脉冲数
		for( int road=0;road<MaxSock;road++ )
		{
			sysprintf("[%d]pulse = %08d\r\n",road,pulse[road] );  //打印周期脉冲数
			
			min_pulse[road] += pulse[road];		//累加分钟检测
			
			pulse[road] = 0;
		}
		min_cntr++;
	}
	
	//1分钟发送一边电能统计
	if( min_cntr >= 12 )
	{//成立，说明过了1分钟
		min_cntr = 0;
		
	}
	
}


//接口
static hlw_opt_t do_hlw_opt={
	.init = init,
	.work = work,
};

hlw_opt_t* hlw_opt = &do_hlw_opt;

