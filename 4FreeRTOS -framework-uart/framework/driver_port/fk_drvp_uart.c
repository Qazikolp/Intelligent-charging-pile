#include "framework.h"

/*
1.驱动解耦，实现这些接口
(1)能够初始化串口
(2)能够随时开关串口这个外设
(3)往串口里写数据
(4)从串口读数据出来
(5)开关中断

2.drv层
(1)init,初始化串口
(2)open
(3)close
(4)write,直接把数据发送出去
(5)read，读出环形缓冲区的数据
(6)irq_enable

需要配置环形缓冲区
需要实现串口接收的回调函数，并且安装到drvp_uart

3.drvp层
(1)init,初始化串口
(2)open
(3)close
(4)write,直接把数据发送出去
(5)设置串口接收中断的回调函数（用于接收数据到环形缓冲区）
(6)irq_enable
(7)

设置 index 转换为 寄存器
*/

static void* get_uart_index(int index, int* err)
{
    void *uart = NULL;
    *err = 0;
    switch(index)
    {
        case 0:
            uart = UART0;
            break;
        
        case 1:
            uart = UART1;
            break;
        
        case 2:
            uart = UART2;
            break;
        
        case 3:
            uart = UART3;
            break;
        
        default:
            *err = 1;
            break;
    }
    return uart;
}
//-------------------------------------------------------------------------------
//index 索引值 =  _e_uart0 ~ _e_uart3
static void init(int index, uint32_t baudrate)
{
    UART_InitStructure UART_initStruct = {0};
    int err = 0; 
    //判断index是否合法
   void * uart = get_uart_index(index, &err);
   if(err == 1)
   {
        err = 1;
       goto __deal_err;
   }
   //根据 index 区分串口，初始化具体的引脚
   switch(index)
   {
       case 0:
            PORT_Init(PORTA, PIN2, FUNMUX0_UART0_RXD, 1);  //GPIOA，2配置为UART0输入引脚
            PORT_Init(PORTA, PIN3, FUNMUX1_UART0_TXD, 0);  //GPIOA，3配置为UART0输入引脚
            //配置串口的参数
            UART_initStruct.Baudrate = baudrate;
            UART_initStruct.DataBits = UART_DATA_8BIT;
            UART_initStruct.Parity = UART_PARITY_NONE;
            UART_initStruct.StopBits = UART_STOP_1BIT;
            UART_initStruct.RXThreshold = 7;
            UART_initStruct.RXThresholdIEn = 1;
            UART_initStruct.TXThreshold = 0;
            UART_initStruct.TXThresholdIEn = 0;
            UART_initStruct.TimeoutTime = 255;
            UART_initStruct.TimeoutIEn = 1;
            UART_Init(UART0, &UART_initStruct);
            UART_Open(UART0);

            break;
       case 1:
//            PORT_Init(PORTA, PIN2, FUNMUX0_UART0_RXD, 1);  //GPIOA，2配置为UART1输入引脚
//            PORT_Init(PORTA, PIN3, FUNMUX1_UART0_TXD, 0);  //GPIOA，3配置为UART1输入引脚
           break;
       
       case 2:
//            PORT_Init(PORTA, PIN2, FUNMUX0_UART0_RXD, 1);  //GPIOA，2配置为UART2输入引脚
//            PORT_Init(PORTA, PIN3, FUNMUX1_UART0_TXD, 0);  //GPIOA，3配置为UART2输入引脚
           break;
       
       case 3:
//            PORT_Init(PORTA, PIN2, FUNMUX0_UART0_RXD, 1);  //GPIOA，2配置为UART3输入引脚
//            PORT_Init(PORTA, PIN3, FUNMUX1_UART0_TXD, 0);  //GPIOA，3配置为UART3输入引脚
           break;
       
       default:
            break;
   }
   UART_Init(uart, &UART_initStruct);
   return;
   __deal_err:   //错误报告
   printf("err[%d]:%s\r\n", err,__func__);
   while(1);
}

static void open(int index)
{
    int err = 0; 
    //判断index是否合法
   void * uart = get_uart_index(index, &err);
   if(err == 1)
   {
       return;
   }
   UART_Open(uart);
}

static void close(int index)
{
    int err = 0; 
    //判断index是否合法
   void * uart = get_uart_index(index, &err);
   if(err == 1)
   {
       return;
   }
   UART_Close(uart);
}

//发送单字节数据
static void write(int index, uint8_t dat)
{
    int err = 0; 
    //判断index是否合法
   void * uart = get_uart_index(index, &err);
   if(err == 1)
   {
       return;
   }
   //卡住，判忙
   while(UART_IsTXFIFOFull(uart));

   //发送
   UART_WriteByte(uart, dat); 
}


//设置串口接收中断的回调函数
typedef void (*uart_callback_t)(int index, uint8_t dat);

static void uart_callback_none(int index, uint8_t dat)
{
    //执行到这，说明没有安装回调函数
}

//接收中断服务 回调函数
static uart_callback_t uart_rx_callback[_e_max_uart] = 
{
    uart_callback_none,
    uart_callback_none,
    uart_callback_none,
    uart_callback_none,
};

//设置接收中断 回调函数
static void set_int_rxfunc(int index, void *cb)
{
    //判断index是否合法
    if(index >= _e_max_uart)
        return;
    
    uart_rx_callback[index] = (uart_callback_t)cb;

}


//开关中断
static void irq_enable(int index, int enable)
{
    //判断index是否合法
    if(index >= _e_max_uart)
        return;
    
    if(enable)
         NVIC_EnableIRQ((IRQn_Type)(UART0_IRQn + index));
    else
        NVIC_DisableIRQ((IRQn_Type)(UART0_IRQn + index));
}

void UART0_Handler(void)
{
    uint32_t chr;
    UART_TypeDef *uart = UART0;
    int index = 0; 
    //判断串口接收fifo高位中断/接收超时中断
    int ret = UART_INTStat(uart, UART_IT_RX_THR | UART_IT_RX_TOUT);
    if(ret == 1)
    {
        //中断发生-》读取数据
        while(UART_IsRXFIFOEmpty(uart) == 0)
		{
			if(UART_ReadByte(uart, &chr) == 0)
			{
				//在中断中使用printf，仅作为延时，实际项目尽可能不要使用
               uart_rx_callback[index](index, chr);
			}
		}
    }
}



void UART1_Handler(void)
{
    uint32_t chr;
    UART_TypeDef *uart = UART0;
    int index = 1; 
    //判断串口接收fifo高位中断/接收超时中断
    int ret = UART_INTStat(uart, UART_IT_RX_THR | UART_IT_RX_TOUT);
    if(ret == 1)
    {
        //中断发生-》读取数据
        while(UART_IsRXFIFOEmpty(uart) == 0)
		{
			if(UART_ReadByte(uart, &chr) == 0)
			{
				//在中断中使用printf，仅作为延时，实际项目尽可能不要使用
               uart_rx_callback[index](index, chr);
			}
		}
    }
}



void UART2_Handler(void)
{
    uint32_t chr;
    UART_TypeDef *uart = UART0;
    int index = 2; 
    //判断串口接收fifo高位中断/接收超时中断
    int ret = UART_INTStat(uart, UART_IT_RX_THR | UART_IT_RX_TOUT);
    if(ret == 1)
    {
        //中断发生-》读取数据
        while(UART_IsRXFIFOEmpty(uart) == 0)
		{
			if(UART_ReadByte(uart, &chr) == 0)
			{
				//在中断中使用printf，仅作为延时，实际项目尽可能不要使用
               uart_rx_callback[index](index, chr);
			}
		}
    }
}



void UART3_Handler(void)
{
    uint32_t chr;
    UART_TypeDef *uart = UART0;
    int index = 3; 
    //判断串口接收fifo高位中断/接收超时中断
    int ret = UART_INTStat(uart, UART_IT_RX_THR | UART_IT_RX_TOUT);
    if(ret == 1)
    {
        //中断发生-》读取数据
        while(UART_IsRXFIFOEmpty(uart) == 0)
		{
			if(UART_ReadByte(uart, &chr) == 0)
			{
				//在中断中使用printf，仅作为延时，实际项目尽可能不要使用
               uart_rx_callback[index](index, chr);
			}
		}
    }
}

static drvp_uart_t do_drvp_uart = {
	.init = init,
	.open = open,
	.close = close,
	.write = write,
	.set_int_rxfunc = set_int_rxfunc,
	.irq_enable = irq_enable,    
};

drvp_uart_t* drvp_uart = &do_drvp_uart;
