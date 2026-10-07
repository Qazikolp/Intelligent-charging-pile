//不需要头文件卫士，只能被framework.h包含


/*----------------------------------------------------------------------------------*/
//内存管理
#define mem_get_free() 					xPortGetFreeHeapSize()			//例如 int vfree = mem_get_free();
#define mem_alloc(size_bytes) 	pvPortMalloc(size_bytes)		//eg. u8* buf = (u8*)mem_alloc(100);
#define mem_free(pv) 						vPortFree(pv)								//eg. mem_free(buf);


/*----------------------------------------------------------------------------------*/

#include "./lib_utc/utc.h"
#include "./lib_loopbuf/loopbuf.h"

#include "./lib_fal/fal/inc/fal.h"


//因为设计上，driver是依赖于driver_port，所以先包含fk_drvp_typedef.h
#include "./driver_port/fk_drvp_typedef.h"

#include "./driver/fk_drv_typedef.h"






