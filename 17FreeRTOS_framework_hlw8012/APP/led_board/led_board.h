#ifndef __led_board_h__
#define __led_board_h__

#include "framework.h"
#include "stdlib.h"


typedef struct{
	void (*init)( void );
	
	//外部修改灯的显示，调用以下3个函数，修改状态，work函数，会刷新显示
	void (*set_net)( int pwr );
	void (*set_fuse)( int pwr );
	void (*set_sock)( int road,int sta );
	void (*work)( uint32_t tick );
	
	
}led_board_opt_t;


extern led_board_opt_t* led_board_opt;
#endif




