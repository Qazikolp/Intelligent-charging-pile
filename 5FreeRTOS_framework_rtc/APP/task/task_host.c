

#include "framework.h"

void myprintf( char* format, ... );



void task_host_entry(void *arg)
{
	myprintf("%s\r\n",__FUNCTION__);
	
	while(1);
	
}

static TaskHandle_t TaskHandle_host;


void create_task_of_host( void )
{
	myprintf("%s\r\n",__FUNCTION__);
	//创建任务
	xTaskCreate( task_host_entry, (const char *)"host", 1024, NULL, 2, &TaskHandle_host );
}






