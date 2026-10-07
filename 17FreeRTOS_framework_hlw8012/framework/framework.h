#ifndef __framework_h__
#define __framework_h__

#include "SWM320.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

#include <stdarg.h>
#include <string.h>


/*----------------------------------------------------------------------------------*/
#define NetRST 	56
#define NetUart _e_uart1

#define IO_HC_DAT 36
#define IO_HC_CLK 35
#define IO_HC_UD	37

#define IO_Fuse		21


#define MaxSock		2		//有2个插座

#define IO_HLW0		22      //计量芯片引脚
#define IO_HLW1		20

#define IO_ELC0		26      //继电器引脚
#define IO_ELC1		3



#include "fk_typedef.h"

extern void sysprintf( char* format, ... );


#endif

