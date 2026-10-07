#include "framework.h"
#include "work.h"




void task_listen_entry(void *arg)
{
	sysprintf("%s\r\n",__FUNCTION__);
	
	while(1)
	{
		fuse->work();
		
		vTaskDelay( pdMS_TO_TICKS( 5 ) );   //释放cpu资源
	}
	
}

static TaskHandle_t TaskHandle_listen;


void create_task_of_listen( void )
{
	sysprintf("%s\r\n",__FUNCTION__);
	//创建任务
	xTaskCreate( task_listen_entry, (const char *)"listen", 1024, NULL, 3, &TaskHandle_listen );
}


