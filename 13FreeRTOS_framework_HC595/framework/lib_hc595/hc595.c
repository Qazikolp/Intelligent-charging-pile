#include "framework.h"

#define wide_rck 50
#define wide_sck 20

//初始化
static void init( hc595_pin_t* bus )    
{                                                      //四个引脚模式设置
	gpio->set_mode( bus->dat,PIN_MODE_OUTPUT );
	gpio->set_mode( bus->clk,PIN_MODE_OUTPUT );
	gpio->set_mode( bus->ud,PIN_MODE_OUTPUT );
	gpio->set_mode( bus->cs,PIN_MODE_OUTPUT );
}

//写数据函数，输出一个字节
static void write_byte( hc595_pin_t* bus,uint8_t dat )    //软件SPI
{
	volatile uint32_t dealy = 0;     //延时
	
	for( int i=0;i<8;i++ )
	{
		//根据数据进行输出
		if( dat & 0x80 )            //最高位为1，输出高电平
			gpio->write( bus->dat,PIN_HIGH );
		else                        //最高位为0，输出低电平
			gpio->write( bus->dat,PIN_LOW  );
		
		dat<<=1;  //左移位
		
		//产生上升沿   软件SPI
		gpio->write( bus->clk,PIN_LOW  );
		dealy = wide_sck; while( dealy-- );      //延时
		gpio->write( bus->clk,PIN_HIGH );
		dealy = wide_sck; while( dealy-- );      //延时
		//移位寄存器会读出1bit数据，并推出1bit数据
	}
	
}

//输出多个字节
//len = 2 
static void write( hc595_pin_t* bus,uint8_t* buf,uint8_t len )
{
	for( int i=0;i<len;i++ )
	{

		//		write_byte( bus,buf[i] );		//先送首端数据
		write_byte( bus,buf[ len -1 - i ] );	//先送末端数据
	}
}

static void update( hc595_pin_t* bus )
{
	volatile uint32_t dealy = 0;
    
	//UD引脚，产生上升沿时，会从移位寄存器加载数据到锁存器
	gpio->write( bus->ud,PIN_LOW  );
	dealy = wide_sck; while( dealy-- );
	gpio->write( bus->ud,PIN_HIGH );
	dealy = wide_sck; while( dealy-- );
	
}


//函数指针接口
static hc595_opt_t do_hc595_opt={
	.init = init,
	.write = write,
	.update = update,
};

hc595_opt_t* hc595_opt = &do_hc595_opt;


