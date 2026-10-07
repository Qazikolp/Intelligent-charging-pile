#include "led_board.h"

/*分配显存	一共有16个LED

1、修改状态 网络状态指示
2、修改状态 保险丝状态指示
3、修改状态 修改其中一个插座的状态
	空闲状态, 全灭
	正在充电, 跑灯

*/

//描述显示状态
typedef struct{
	uint8_t net;
	uint8_t fuse;
	uint8_t sock[2];			//=0，说明空闲 ；=1 说明正在充电
}lsta_t;

static lsta_t lsta={0,0,0,0};

static void set_net( int pwr )
{
	lsta.net = pwr;
}

static void set_fuse( int pwr )
{
	lsta.fuse = pwr;
}

static void set_sock( int road,int sta )
{
	lsta.sock[ road ] = sta;
}






/*----------------------------------------------------------------------------------*/
static uint8_t disp[2];		//显存


//插座状态灯映射表

#define SockLed	7
static uint8_t led_map[ MaxSock ][ SockLed ]={
	{ 8,9,10,11,12,13,14 },
	{ 7,6, 5, 4, 3, 2, 1 }
};

#define NetLED	0			//网络指示灯
#define FuseLED	15			//保险丝指示灯

//HC595总线引脚定义
static hc595_pin_t LED_BUS = { IO_HC_DAT,IO_HC_CLK,IO_HC_UD,0 };
/*----------------------------------------------------------------------------------*/





/*----------------------------------------------------------------------------------*/

static void set_one_bit( int ibit,int pwr )
{ 
	uint8_t index = ibit/8;		//获取数组的下标
	uint8_t offset = ibit%8;	//获取对应下标的bit偏移量
	
	if( pwr == __ON )
	{//对应bit，输出低电平
		 disp[index] &= ~( 1<<offset );
	}
	else
	{//对应bit，输出高电平
		disp[index]  |=  ( 1<<offset );
	}
}


static void show(void)
{
	hc595_opt->write( &LED_BUS,disp,sizeof( disp ) );
	hc595_opt->update( &LED_BUS );
}
/*----------------------------------------------------------------------------------*/

static void init( void )
{
	hc595_opt->init( &LED_BUS );
	
	for( int i=0;i<sizeof( disp );i++ )
		disp[i] = 0xFF;
	
	hc595_opt->write( &LED_BUS,disp,sizeof( disp ) );
	hc595_opt->update( &LED_BUS );
}

static void sock_led( int road,int sta )
{
	static uint8_t counter[ MaxSock ] ={0,0};
	
	//在缓冲区上灭掉所有灯
	for( int i=0;i<SockLed;i++ )
			set_one_bit( led_map[road][i],__OFF );
	
	if( sta == 0 )
	{//空闲状态
		counter[road] = 0;
		
	}
	else
	{//充电状态，跑灯
		counter[road]++;	//每次进来亮多一颗灯
		if( counter[road]> SockLed )
			counter[road] = 0;
		
		for( int i=0;i<counter[road];i++ )
			set_one_bit( led_map[road][i],__ON );
		
	}
	
	
}


//一般tick = 50  单位 ms
static void work( uint32_t tick )
{
	static uint32_t otick = 0;
	
	//访问频率控制
	uint32_t ntick = get_sys_ticks();
	if( (ntick - otick) < tick ) return;
	otick = ntick;
	
	
	//根据状态，需改指示灯的显存
	set_one_bit( NetLED,lsta.net );	
	set_one_bit( FuseLED,lsta.fuse );	
	
	for( int road=0;road<MaxSock;road++ )
		sock_led( road,lsta.sock[road] );
	
	show();
	
}


static led_board_opt_t do_led_board_opt ={
	.init = init,
	.set_net = set_net,
	.set_fuse = set_fuse,
	.set_sock = set_sock,
	.work = work,

};

led_board_opt_t* led_board_opt = &do_led_board_opt;
/*----------------------------------------------------------------------------------*/




