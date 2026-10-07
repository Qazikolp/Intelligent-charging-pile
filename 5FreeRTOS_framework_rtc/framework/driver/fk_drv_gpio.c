#include "framework.h"

extern drvp_gpio_t* drvp_gpio;

//设置引脚模式
static void set_mode( int pin, int mode )
{
	drvp_gpio->set_mode( pin,mode );
}
	//写引脚状态
static void write( int pin, int value )
{
	drvp_gpio->write( pin,value );

}
	//读引脚状态
static int read(  int pin )
{
	return drvp_gpio->read( pin );

}
	//给一个引脚附加(外部)中断
static int attach_irq( int pin,int mode,pin_callback_t cb,void *args)
{
	return drvp_gpio->attach_irq( pin,mode,cb,args );

}
	//给一个引脚去除(外部)中断
static int detach_irq(  int pin )
{
	return drvp_gpio->detach_irq( pin );

}
	//引脚外部中断使能
static int irq_enable( int pin,int enabled )
{
	return drvp_gpio->irq_enable( pin,enabled );
}

static drv_gpio_t do_drv_gpio={
	.set_mode = set_mode,
	.write = write,
	.read = read,
	.attach_irq = attach_irq,
	.detach_irq = detach_irq,
	.irq_enable = irq_enable,
	
};

drv_gpio_t* gpio = &do_drv_gpio;





