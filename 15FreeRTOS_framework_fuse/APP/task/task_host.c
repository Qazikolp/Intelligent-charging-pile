

#include "framework.h"

#include "cmdline.h"
#include "led_board.h"



void task_host_entry(void *arg)
{
	sysprintf("%s\r\n",__FUNCTION__);
	
	while(1)
	{
		led_board_opt->work( 200 );
		cmd_line_work();
		vTaskDelay( pdMS_TO_TICKS( 5 ) );
	}
	
}

static TaskHandle_t TaskHandle_host;


void create_task_of_host( void )
{
	sysprintf("%s\r\n",__FUNCTION__);
	//创建任务
	xTaskCreate( task_host_entry, (const char *)"host", 1024, NULL, 2, &TaskHandle_host );
}






