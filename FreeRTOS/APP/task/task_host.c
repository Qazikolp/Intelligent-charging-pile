#include "SWM320.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

void myprint(char *format,...);

void task_host_entyr(void *arg)
{
    myprint("%s\r\n", __func__);
    
    
    while(1)
    {
        
        
    }
}

static TaskHandle_t  TaskHandle_host;

void create_task_of_host(void)
{
    //´´½¨Á´Â·
    xTaskCreate(task_host_entyr, (const char *)"host", 1024, NULL, 2, &TaskHandle_host);
    myprint("%s\r\n",__func__);
}


