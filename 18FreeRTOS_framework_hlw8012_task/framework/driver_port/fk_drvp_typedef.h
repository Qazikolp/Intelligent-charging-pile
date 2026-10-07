//不需要头文件卫士，只能被fk_typedef.h包含



#define PIN_NONE                (-RT_EEMPTY)

#define PIN_LOW                 0x00 /*!< low level */
#define PIN_HIGH                0x01 /*!< high level */

#define PIN_MODE_OUTPUT         0x00 /*!< output mode */
#define PIN_MODE_INPUT          0x01 /*!< input mode */
#define PIN_MODE_INPUT_PULLUP   0x02 /*!< input mode with pull-up */
#define PIN_MODE_INPUT_PULLDOWN 0x03 /*!< input mode with pull-down */
#define PIN_MODE_OUTPUT_OD      0x04 /*!< output mode with open-drain */


#define PIN_IRQ_MODE_RISING             0x00 /*!< rising edge trigger */
#define PIN_IRQ_MODE_FALLING            0x01 /*!< falling edge trigger */
#define PIN_IRQ_MODE_RISING_FALLING     0x02 /*!< rising and falling edge trigger */
#define PIN_IRQ_MODE_HIGH_LEVEL         0x03 /*!< high level trigger */
#define PIN_IRQ_MODE_LOW_LEVEL          0x04 /*!< low level trigger */

#define PIN_IRQ_DISABLE                 0x00 /*!< disable irq */
#define PIN_IRQ_ENABLE                  0x01 /*!< enable irq */

#define PIN_IRQ_PIN_NONE                PIN_NONE /*!< no pin irq */

typedef void (*pin_callback_t)(void *args);

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
	
	
}drvp_gpio_t;
/*-------------------------------------------------------------------------*/
//串口
enum{
	_e_uart0 = 0,
	_e_uart1,
	_e_uart2,
	_e_uart3,
	_e_max_uart,
};//对于上层驱动，只需要指导串口号，不需要知道具体的寄存器

typedef struct{
	void (*init)( int index,uint32_t baudrate );
	void (*open)( int index );
	void (*close)( int index );
	void (*write)( int index,uint8_t dat );
	void (*set_int_rxfunc)( int index,void* cb );
	void (*irq_enable)( int index, int enable );
	
}drvp_uart_t;

/*-------------------------------------------------------------------------*/
//RTC
/*
typedef struct {
	uint16_t Year;
	uint8_t  Month;
	uint8_t  Date;
	uint8_t  Day;			//RTC_SUN、RTC_MON、RTC_TUE、RTC_WED、RTC_THU、RTC_FRI、RTC_SAT
	uint8_t  Hour;
	uint8_t  Minute;
	uint8_t  Second;
} RTC_DateTime;
*/

//借助官方sdk的数据类型，如果你更换芯片，自行匹配这个数据类型即可。



typedef struct{
	void (*init)( utc_t* dt );				//初始化函数
	void (*start)(void);								//启动RTC
	void (*stop)(void);									//停止RTC
	utc_t* (*get_dt)(void);					//获取rtc_dt变量
	int (*read)( utc_t* dt );				//从硬件读取rtc_dt

}drvp_rtc_t;
/*-------------------------------------------------------------------------*/
//WDT
typedef struct{
	void (*init)( uint32_t tick );
	void (*feed)(void);
	void (*start)(void);
	void (*stop)(void);
}drvp_wdt_t;

/*-------------------------------------------------------------------------*/



