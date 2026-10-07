#include "framework.h"

extern drvp_uart_t* drvp_uart;

/*
2.drv层
(1)init,初始化串口
(2)open
(3)close
(4)write,直接把数据发送出去
(5)read，读出环形缓冲区的数据
(6)irq_enable

需要配置环形缓冲区
需要实现串口接收的回调函数，并且安装到drvp_uart
*/


static loopbuf_t * lb_uart_rx[_e_max_uart];   //串口的环形缓冲区数组

//------------------------------------------------------------------------------
//接收中断的回调函数，drvp调用时，需要根据串口号，输入index，用于辨别哪一个是串口
static void  recv_callback(int index, uint8_t dat)
{
    //把数据扔进环形缓冲区
    loopbuf_write(lb_uart_rx[index], &dat, 1);
    
}

//------------------------------------------------------------------------------

static void init(int index, uint32_t baudrate)
{
    int err;
    loopbuf_t* tlb = loopbuf_init(1024);
    lb_uart_rx[index] = tlb;
    //判断索引是否合法
    if(index >= _e_max_uart)
    {   //说明索引不合法
        err = 1;
        goto __deal_err;
    }
    //初始化环形缓冲区
   
    if(!tlb)
    {   //说明构建缓冲区错误
        err = 2;
        goto __deal_err;
    }
    
    //安装接收中断服务函数
    drvp_uart->set_int_rxfunc(index, recv_callback);
    
    //初始化外设
    drvp_uart->init(index, baudrate);
    
    return;
    __deal_err:
    printf("err[%d]:%s(%d)\r\n", err,__func__, index);
    while(1);
}

//------------------------------------------------------------------------------

static void open(int index)
{
    drvp_uart->open(index);
}

//------------------------------------------------------------------------------

static void close(int index)
{
    drvp_uart->close(index);
}

//------------------------------------------------------------------------------

static void write(int index, uint8_t dat)
{
    drvp_uart->write(index, dat);
}

//------------------------------------------------------------------------------

static int read(int index, void * buf, int len)
{
    return loopbuf_read(lb_uart_rx[index], buf, len);
}

//------------------------------------------------------------------------------

static void clr_rxbuf(int index)
{
    loopbuf_reset(lb_uart_rx[index]);
    
}

//------------------------------------------------------------------------------

static void irq_enable(int index, int enable)
{
    drvp_uart->irq_enable(index, enable);
}

//------------------------------------------------------------------------------

static drvp_uart_t do_drvp_uart = {
	.init = init,
	.open = open,
	.close = close,
	.write = write,
	.read = read,
	.clr_rxbuf = clr_rxbuf,
	.irq_enable = irq_enable,    
};

drvp_uart_t* uart = &do_drvp_uart;
