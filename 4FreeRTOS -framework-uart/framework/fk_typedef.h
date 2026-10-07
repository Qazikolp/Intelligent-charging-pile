//不用头文件卫士

//设计上，driver是依赖于driver_port，所以先包含fk_drvp_typedef.h

//内存管理
#define mem_get_free()             xPortGetFreeHeapSize()            //例如 int vfree = mem_get_free();
#define mem_alloc(size_bytes)     pvPortMalloc(size_bytes)        //eg. u8* buf = (u8*)mem_alloc(100);
#define mem_free(pv)             vPortFree(pv)                    //eg. mem_free(buf);


#include "./driver_port/fk_drvp_typedef.h"

#include "./driver/fk_drv_typedef.h"

#include "./ilb_loopbuf/loopbuf.h"

