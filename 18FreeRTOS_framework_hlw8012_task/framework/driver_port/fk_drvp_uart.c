#include "framework.h"

/*
1,驱动解耦，实现这些接口
(1)能够初始化串口
(2)能够随时，开关串口这一个外设
(3)往串口里面写数据
(4)从串口里面读数据出来
(5)开关中断

2，从drv层，怎么看
(1)init,根据波特率，初始化串口
(2)open,
(3)close
(4)write:直接把数据发送出去
(5)read	:从环形缓冲区间接读取数据
(6)irq_enable

需要分配环形缓冲区
需要实现串口接收的回调函数，并且安装到drvp_uart

3, 从drvp层，怎么看
(1)init,根据波特率，初始化串口
(2)open,
(3)close
(4)write:直接把数据发送出去
(5)设置串口接受中断的回调函数(用于接收数据到串口)
(6)irq_enable

实现 index 转换为 寄存器


*/
/*-------------------------------------------------------------------------*/

//根据索引值，返回外设寄存器指针
static void* get_uart_by_index( int index,int* err )
{
	void* uart = NULL;
	*err = 0;
	switch( index )
	{
		case 0: uart = UART0;break;
		case 1: uart = UART1;break;
		case 2: uart = UART2;break;
		case 3: uart = UART3;break;
		default:
			*err = 1;
			break;
			
	}
	
	return uart;
}	

/*-------------------------------------------------------------------------*/
//index 索引值, =_e_uart0 ~ _e_uart3

static void init( int index,uint32_t baudrate )
{
	UART_InitStructure UART_initStruct;						//配置变量
	
	int err = 0;
	void* uart = get_uart_by_index( index,&err );
	if( 1 == err )
	{
		err = 1;					//index 不合法
		goto __deal_err;
	}
	
	//根据index，区分串口，初始化具体的引脚
	switch( index )
	{
		case 0:
			//初始化引脚功能
			PORT_Init(PORTA, PIN2, FUNMUX0_UART0_RXD, 1);	//GPIOA.2配置为UART0输入引脚
			PORT_Init(PORTA, PIN3, FUNMUX1_UART0_TXD, 0);	//GPIOA.3配置为UART0输出引脚
		
			//配置串口的参数
			UART_initStruct.Baudrate = baudrate;
			UART_initStruct.DataBits = UART_DATA_8BIT;		//8bit数据位
			UART_initStruct.Parity = UART_PARITY_NONE;		//无校验
			UART_initStruct.StopBits = UART_STOP_1BIT;		//1位停止位
			
			//当RX FIFO中数据个数 >  RXThreshold时触发中断
			UART_initStruct.RXThreshold = 7;							
			UART_initStruct.RXThresholdIEn = 1;						//使用接收中断
			
			//当TX FIFO中数据个数 <= TXThreshold时触发中断
			UART_initStruct.TXThreshold = 0;
			UART_initStruct.TXThresholdIEn = 0;						//不使用发送中断
			
			//超时中断，RX FIFO非空，且超过 TimeoutTime/(Baudrate/10) 秒没有在RX线上接收到数据时触发中断
			UART_initStruct.TimeoutTime = 255;						//10个字符时间内未接收到新的数据则触发超时中断
			UART_initStruct.TimeoutIEn = 1;
			
			break;
			
		case 1:
			//初始化引脚功能
			PORT_Init(PORTA, PIN4, FUNMUX0_UART1_RXD, 1);
			PORT_Init(PORTA, PIN5, FUNMUX1_UART1_TXD, 0);
		
			//配置串口的参数
			UART_initStruct.Baudrate = baudrate;
			UART_initStruct.DataBits = UART_DATA_8BIT;		//8bit数据位
			UART_initStruct.Parity = UART_PARITY_NONE;		//无校验
			UART_initStruct.StopBits = UART_STOP_1BIT;		//1位停止位
			
			//当RX FIFO中数据个数 >  RXThreshold时触发中断
			UART_initStruct.RXThreshold = 7;							
			UART_initStruct.RXThresholdIEn = 1;						//使用接收中断
			
			//当TX FIFO中数据个数 <= TXThreshold时触发中断
			UART_initStruct.TXThreshold = 0;
			UART_initStruct.TXThresholdIEn = 0;						//不使用发送中断
			
			//超时中断，RX FIFO非空，且超过 TimeoutTime/(Baudrate/10) 秒没有在RX线上接收到数据时触发中断
			UART_initStruct.TimeoutTime = 255;						//10个字符时间内未接收到新的数据则触发超时中断
			UART_initStruct.TimeoutIEn = 1;
			break;
		
		case 2:
			
			break;
		
		case 3:
			
			break;
		
		default:
			break;
			
	}
	UART_Init(uart, &UART_initStruct);
	
	return;
__deal_err:
	printf("err[%d]:%s\r\n",err,__func__ );
	while(1);
	
}

/*-------------------------------------------------------------------------*/
static void open( int index )
{
	int err = 0;
	void* uart = get_uart_by_index( index,&err );
	if( 1 == err )
	{
		return;
	}
	
	UART_Open( uart );
}
/*-------------------------------------------------------------------------*/
static void close( int index )
{
	int err = 0;
	void* uart = get_uart_by_index( index,&err );
	if( 1 == err )
	{
		return;
	}
	
	UART_Close( uart );
}
/*-------------------------------------------------------------------------*/
//发送单字节数据
static void write( int index,uint8_t dat )
{
	int err = 0;
	void* uart = get_uart_by_index( index,&err );
	if( 1 == err )
	{
		return;
	}

	//卡住，判忙
	
	while(UART_IsTXFIFOFull(uart) );

	//发送	
	UART_WriteByte( uart, dat );
	
}
/*-------------------------------------------------------------------------*/
typedef void (*uart_callback_t)( int index,uint8_t dat );


//设置串口的接收中断回调函数
static void uart_callback_none( int index,uint8_t dat )
{
	//执行到这里，说明没有安装回调函数
}


//接收中断服务 回调函数
static uart_callback_t uart_rx_callback[_e_max_uart ]={
	uart_callback_none,
	uart_callback_none,
	uart_callback_none,
	uart_callback_none

};

//设置接收中断 回调函数
static void set_int_rxfunc( int index,void* cb )
{
	if( index >= _e_max_uart ) return;
	
	uart_rx_callback[index] = (uart_callback_t)cb;
	
}
/*-------------------------------------------------------------------------*/
//开关中断
static void irq_enable( int index, int enable )
{
	
	if( index >= _e_max_uart ) return;
	
	if( enable ) NVIC_EnableIRQ ( (IRQn_Type)(UART0_IRQn + index) );
	else				 NVIC_DisableIRQ( (IRQn_Type)(UART0_IRQn + index) );
	
}

/*-------------------------------------------------------------------------*/
void UART0_Handler(void)
{
	
	//--------------------------------------------------------
	//设置串口号
	UART_TypeDef * uart = UART0;		//串口寄存器
	int index = 0;									//串口索引号
	//--------------------------------------------------------
	uint32_t chr;										//用于接收数据
	
	//如果触发了FIFO溢出中断/接收超时中断
	if( UART_INTStat( uart, UART_IT_RX_THR | UART_IT_RX_TOUT ) )
	{
		while( UART_IsRXFIFOEmpty( uart ) == 0)
		{
			if(UART_ReadByte( uart, &chr) == 0)
			{
				uart_rx_callback[ index ]( index,chr ); 
			}
		}
	}
	
}
/*-------------------------------------------------------------------------*/
void UART1_Handler(void)
{
	
	//-------------------------------------------------
	
	UART_TypeDef* uart = UART1;
	int index = 1;
	//-------------------------------------------------
	uint32_t chr;
	//判断串口接收FIFO高位中断/接收超时中断
	int ret = UART_INTStat( uart , UART_IT_RX_THR | UART_IT_RX_TOUT );
	if( 1 == ret  )
	{
		//读取数据
		while( UART_IsRXFIFOEmpty( uart ) == 0)
		{
			if(UART_ReadByte( uart, &chr) == 0)
			{
				uart_rx_callback[ index ]( index, chr );
			}
		}
	}
	
}
/*-------------------------------------------------------------------------*/
void UART2_Handler(void)
{
	
	//-------------------------------------------------
	
	UART_TypeDef* uart = UART2;
	int index = 2;
	//-------------------------------------------------
	uint32_t chr;
	//判断串口接收FIFO高位中断/接收超时中断
	int ret = UART_INTStat( uart , UART_IT_RX_THR | UART_IT_RX_TOUT );
	if( 1 == ret  )
	{
		//读取数据
		while( UART_IsRXFIFOEmpty( uart ) == 0)
		{
			if(UART_ReadByte( uart, &chr) == 0)
			{
				uart_rx_callback[ index ]( index, chr );
			}
		}
	}
	
}

/*-------------------------------------------------------------------------*/
void UART3_Handler(void)
{
	
	//-------------------------------------------------
	
	UART_TypeDef* uart = UART3;
	int index = 3;
	//-------------------------------------------------
	uint32_t chr;
	//判断串口接收FIFO高位中断/接收超时中断
	int ret = UART_INTStat( uart , UART_IT_RX_THR | UART_IT_RX_TOUT );
	if( 1 == ret  )
	{
		//读取数据
		while( UART_IsRXFIFOEmpty( uart ) == 0)
		{
			if(UART_ReadByte( uart, &chr) == 0)
			{
				uart_rx_callback[ index ]( index, chr );
			}
		}
	}
	
}
/*-------------------------------------------------------------------------*/

static drvp_uart_t do_drvp_uart={

	.init = init,
	.open = open,
	.close = close,
	.write = write,
	.set_int_rxfunc = set_int_rxfunc,
	.irq_enable = irq_enable,
	
};


drvp_uart_t* drvp_uart = &do_drvp_uart;






