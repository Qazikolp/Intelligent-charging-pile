#include "hlw8012.h"

/*
hlw8012是一个电能计量芯片

这是一个积分芯片，固定周期采样，瞬时电压，瞬时电流,求乘积。
并且对乘积进行累加，加到一定程度(固定值)，则输出一个脉冲。

测量这个固定值的方法：
固定负载功率，策略HLW8012单位时间内输出的脉冲数。

通过对其脉冲计数，就可以知道周期内所消耗的电脑。
而1s钟所消耗的电能就是平均功率。


1,我们要知道插座有没有人插入，需要用到低功率检测
2,我们需要知道插座的平均功率，需要周期采样，求平均
3,我们需要定时1分钟，发出一分钟的电能消耗


冷风 [1]pulse = 00000014
低热 [1]pulse = 00000256

*/
//引脚映射表
static uint8_t pin_map[ MaxSock ]={ IO_HLW1,IO_HLW0 };

//采样脉冲数
static uint32_t pulse[ MaxSock ]={ 0,0};

//记录最后一次脉冲计量时间
static uint32_t last_tick[ MaxSock ] = {0,0};

typedef struct{
	uint8_t road;		//第几路HLW芯片
	uint8_t pin;		//对应的具体IO序号
}dat_t;


static dat_t ArgDat[ MaxSock ];

static void do_isr( void *args )
{
	dat_t* pdat = (dat_t*)args;
	
	//判断road 合法性
	if( pdat->road >= MaxSock ) return;
	
	//采样脉冲数++
	pulse[ pdat->road ]++;
	
	//记录最后一次脉冲计量时间
	last_tick[ pdat->road ] = get_sys_tick_irq();
	
}

//队列
/*
configSUPPORT_DYNAMIC_ALLOCATION 默认已经在FreeRTOS.h 设置为1
作用是，是否允许使用堆的内存，动态分配对象。
*/
static thread_mq_t mq_event[ MaxSock ];





static void init( void )
{

	for( int road=0;road<MaxSock;road++ )
	{
		
		mq_event[road] = thread_mq_init( "mq_hlw8012",sizeof(hlw_pul_t),5 );
		
		ArgDat[road].pin = pin_map[road];
		ArgDat[road].road = road;
		
		
		//先设置为输入模式
		gpio->set_mode( pin_map[road],PIN_MODE_INPUT );
		//附加中断
		gpio->attach_irq( pin_map[road],PIN_IRQ_MODE_FALLING,do_isr,&( ArgDat[road] ) );
		//使能中断
		gpio->irq_enable( pin_map[road],PIN_IRQ_ENABLE );

	}
	
}

/*----------------------------------------------------------------------------------*/
//发送一条消息
static void put( int road,uint32_t type,uint32_t cycle,uint32_t val )
{
	hlw_pul_t dat;
	
	if( road >=MaxSock ) return;

	dat.type = type;
	dat.cycle = cycle;
	dat.val = val;
	
	int ret = thread_mq_send( mq_event[road],&dat,sizeof(hlw_pul_t),0x01 );
	if( ret == pdTRUE )
	{//说明发送成功了，意思时消息已经入队。
		//相当于我们把数据写入环形缓冲区，但是此时，还没有消耗这一个数据
		
	}
	else
	{//说明发送超时了
		sysprintf("warning:%s(%d)\r\n",__func__,road );
	}
	
	
}

//接收一条消息
static int get( int road,void* data )
{
	hlw_pul_t* dat = (hlw_pul_t*)data;
	if( road >=MaxSock ) return-1;
	
	int ret = thread_mq_recv(  mq_event[road],dat,sizeof(hlw_pul_t),0 );
	if( ret == pdTRUE )
	{
		return 0;
	}
	else
	{
		return -1;
	}
	
	
}
	
	


/*----------------------------------------------------------------------------------*/

static void work( void )
{
	static uint32_t otick_cyc =0 ;
	static uint32_t min_pulse[MaxSock] ={0,0};
	static uint32_t min_cntr = 0;
	
	
	uint32_t ntick = get_sys_ticks();
	
	
	//检查用电器拔出事件
	for( int road=0;road<MaxSock;road++ )
	{
		if( ( ntick - last_tick[road] ) >( 3000 ) )
		{//成立，则认为没有插入用电器
			last_tick[road] = ntick;
			//发送用电器拔出事件
			//TODO
			put( road, _type_hlw_sock_out, 3000, 0 );
		}
	}
	
	
	//周期采样脉冲数
	if( (ntick - otick_cyc) >= 5000 )
	{
		otick_cyc = ntick;
		
		//打印本周期采样的脉冲数
		for( int road=0;road<MaxSock;road++ )
		{
//			sysprintf("[%d]pulse = %08d\r\n",road,pulse[road] );
			put( road, _type_hlw_cyc_pul, 5000, pulse[road] );
			
			min_pulse[road] += pulse[road];		//累加分钟检测
			
			pulse[road] = 0;
		}
		min_cntr++;
	}
	
	//1分钟发送一遍，用于电能统计
	if( min_cntr >= 12 )
	{//成立，说明过了1分钟
		for( int road=0;road<MaxSock;road++ )
		{
			put( road, _type_hlw_min_pul,60*1000, min_pulse[road] );
			min_pulse[road] = 0;
		}
		min_cntr = 0;
		
	}
	
}

static hlw_opt_t do_hlw_opt={
	.init = init,
	.get = get,
	.work = work,
};

hlw_opt_t* hlw_opt = &do_hlw_opt;

