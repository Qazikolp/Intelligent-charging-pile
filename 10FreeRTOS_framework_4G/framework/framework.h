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


#include "fk_typedef.h"

extern void sysprintf( char* format, ... );


#endif

