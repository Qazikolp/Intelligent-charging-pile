

//这个类型用于定义引脚
typedef struct{
	uint8_t dat;  //数据引脚
	uint8_t clk;
	uint8_t ud;	//update
	uint8_t cs;	//使能引脚，在一些售货机场合，就需要用到使能引脚。

}hc595_pin_t;

//这个类型用于操作总线
typedef struct{
	void (*init)( hc595_pin_t* bus );     //初始化
	void (*write)( hc595_pin_t* bus,uint8_t* buf,uint8_t len );    //写
	void (*update)( hc595_pin_t* bus );   //更新

}hc595_opt_t;

extern hc595_opt_t* hc595_opt;       //灯板操作接口
