

//这个类型用于定义引脚
typedef struct{
	uint8_t dat;
	uint8_t clk;
	uint8_t ud;	//update
	uint8_t cs;	//使能引脚，在一些售货机场合，就需要用到使能引脚。

}hc595_pin_t;

//这个类型用于操作总线
typedef struct{
	void (*init)( hc595_pin_t* bus );
	void (*write)( hc595_pin_t* bus,uint8_t* buf,uint8_t len );
	void (*update)( hc595_pin_t* bus );

}hc595_opt_t;

extern hc595_opt_t* hc595_opt;
