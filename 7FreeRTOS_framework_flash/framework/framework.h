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
//flash ¿Õ¼ä°²ÅÅ

//0
//´æ´¢bootloader
#define _f_boot_start	0
#define _f_boot_size	64*1024

//´æ´¢ÅäÖÃÎÄ¼þ
#define _f_conf_start	64*1024
#define _f_conf_size	 4*1024

//´æ´¢app
#define _f_app_start	70*1024
#define _f_app_size		186*1024

//256KB
//´æ´¢Êý¾Ý¿â
#define _f_fdb_start	256*1024
#define _f_fdb_size		256*1024


/*----------------------------------------------------------------------------------*/




#include "fk_typedef.h"

extern void sysprintf( char* format, ... );


#endif

