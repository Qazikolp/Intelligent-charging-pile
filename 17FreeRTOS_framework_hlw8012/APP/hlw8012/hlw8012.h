#ifndef __hlw8012_h__
#define __hlw8012_h__

#include "framework.h"
#include "stdlib.h"


typedef struct{
	void (*init)( void );
	void (*work)( void );
}hlw_opt_t;


extern hlw_opt_t* hlw_opt;

#endif




