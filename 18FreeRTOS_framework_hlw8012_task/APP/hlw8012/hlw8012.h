#ifndef __hlw8012_h__
#define __hlw8012_h__

#include "framework.h"
#include "stdlib.h"

enum{
	_type_hlw_sock_out = 0,		//插座拔出事件
	_type_hlw_cyc_pul,				//周期脉冲
	_type_hlw_min_pul,				//一分钟脉冲
};

typedef struct{
	uint32_t type;	//本次事件的类型
	uint32_t cycle;	//本次事件所用周期
	uint32_t val;		//本次事件的脉冲数
	
}hlw_pul_t;

typedef struct{
	void (*init)( void );
	
	int (*get)( int road,void* data );
	
	void (*work)( void );
}hlw_opt_t;


extern hlw_opt_t* hlw_opt;

#endif




