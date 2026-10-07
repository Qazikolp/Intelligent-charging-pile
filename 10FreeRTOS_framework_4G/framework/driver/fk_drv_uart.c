#include "framework.h"


extern drvp_uart_t* drvp_uart;


/*-------------------------------------------------------------------------*/

//环形缓冲区
static loopbuf_t* lb_uart_rx[_e_max_uart];
/*-------------------------------------------------------------------------*/
//接收中断回调函数,drvp 调用时，需要根据串口号，输入index,用于辨别是哪一个串口
static void recv_callback( int index,uint8_t dat )
{
	//把数据往环形缓冲区里面扔
	loopbuf_write( lb_uart_rx[index], &dat,1 );
}
/*-------------------------------------------------------------------------*/



static void init( int index,uint32_t baudrate )
{
	int err = 0;				//存储错误号
	loopbuf_t* tlb = NULL;
	
	//判断index是否合法
	if( index >= _e_max_uart ) 
	{
		err = 1;			//说明是 index 不合法
		goto _deal_err;
	}
	
	//初始化环形缓冲区，用于串口的数据接收
	tlb = loopbuf_init( 1024 );
	if( !tlb )
	{//成立，说明错误
		err = 2;			//说明是 缓冲区分配错误
		goto _deal_err;
	}
	lb_uart_rx[ index ] = tlb;
	
	//安装接收中断服务函数
	drvp_uart->set_int_rxfunc( index,recv_callback );
	
	//初始化外设
	drvp_uart->init( index,baudrate );
	
	return;
_deal_err:	
	printf("err[%d]:%s(%d)\r\n",err,__func__,index );
	while(1);
	
}
/*-------------------------------------------------------------------------*/

static void open( int index )
{
	drvp_uart->open( index );
	
}
/*-------------------------------------------------------------------------*/


static void close( int index )
{
	drvp_uart->close( index );
	
}
/*-------------------------------------------------------------------------*/

static void write( int index,uint8_t dat )
{
	drvp_uart->write( index,dat );
}
/*-------------------------------------------------------------------------*/
//index 索引号
//buf 用于保存数据的缓冲区
//len 本次想要读取多少个数据
//返回值 返回真实读取到的数据
static int read( int index,void* buf,int len )
{
	
	return loopbuf_read( lb_uart_rx[index],buf,len );
	
}

static void clr_rxbuf( int index )
{
	loopbuf_reset( lb_uart_rx[index] );
}


/*-------------------------------------------------------------------------*/

//开关中断
static void irq_enable( int index, int enable )
{
	drvp_uart->irq_enable( index,enable );
}
/*-------------------------------------------------------------------------*/


static drv_uart_t do_drv_uart= {
	.init = init,
	.open = open,
	.close = close,
	.write = write,
	.read = read,
	.clr_rxbuf = clr_rxbuf,
	.irq_enable = irq_enable,
	
};

drv_uart_t* uart = &do_drv_uart;





