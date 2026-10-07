#include "SWM320.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

void myprint(char *format,...);

void task_ctrl_entyr(void *arg)
{
    myprint("%s\r\n", __func__);
    
    
    while(1)
    {
        
        
    }
}

static TaskHandle_t  TaskHandle_ctrl;

void create_task_of_ctrl(void)
{
    //´´½¨Á´Â·
    xTaskCreate(task_ctrl_entyr, (const char *)"ctrl", 1024, NULL, 2, &TaskHandle_ctrl);
    myprint("%s\r\n",__func__);
}
