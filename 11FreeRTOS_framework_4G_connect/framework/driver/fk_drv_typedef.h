//不需要头文件卫士，只能被fk_typedef.h包含


typedef struct{
	//设置引脚模式
	void (*set_mode)( int pin, int mode );
	//写引脚状态
	void (*write)( int pin, int value);
	//读引脚状态
	int (*read)(  int pin );
	//给一个引脚附加(外部)中断
	int (*attach_irq)( int pin,int mode,pin_callback_t cb,void *args);
	//给一个引脚去除(外部)中断
	int (*detach_irq)(  int pin );
	//引脚外部中断使能
	int (*irq_enable)( int pin,int enabled );
	
	
}drv_gpio_t;

extern drv_gpio_t* gpio;

/*-------------------------------------------------------------------------*/
typedef struct{
	void (*init)( int index,uint32_t baudrate );
	void (*open)( int index );
	void (*close)( int index );
	void (*write)( int index,uint8_t dat );
	int (*read)( int index,void* buf,int len );
	void (*clr_rxbuf)( int index );
	void (*irq_enable)( int index, int enable );
	
}drv_uart_t;

extern drv_uart_t* uart;
/*-------------------------------------------------------------------------*/
//RTC
typedef struct{
	void (*init)( utc_t* dt );				//初始化函数
	void (*start)(void);								//启动RTC
	void (*stop)(void);									//停止RTC
	utc_t* (*get_dt)(void);					//获取rtc_dt变量
	int (*read)( utc_t* dt );				//从硬件读取rtc_dt

}drv_rtc_t;

extern drv_rtc_t* rtc;
/*-------------------------------------------------------------------------*/
//WDT
typedef struct{
	void (*init)( uint32_t tick );
	void (*feed)(void);
	void (*start)(void);
	void (*stop)(void);
}drv_wdt_t;

extern drv_wdt_t* wdt;
/*-------------------------------------------------------------------------*/




