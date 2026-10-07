#include "framework.h"



void task_ctrl_entry(void *arg)
{
	sysprintf("%s\r\n",__FUNCTION__);
	
	while(1);
	
}

static TaskHandle_t TaskHandle_ctrl;


void create_task_of_ctrl( void )
{
	sysprintf("%s\r\n",__FUNCTION__);
	//创建任务
	xTaskCreate( task_ctrl_entry, (const char *)"ctrl", 1024, NULL, 2, &TaskHandle_ctrl );
}


