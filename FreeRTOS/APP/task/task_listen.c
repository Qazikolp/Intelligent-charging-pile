#include "SWM320.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
void myprint(char *format,...);

void task_listen_entyr(void *arg)
{
    myprint("%s\r\n", __func__);
    
    
    while(1)
    {
        
        
    }
}

static TaskHandle_t  TaskHandle_listen;

void create_task_of_listen(void)
{
    //´´½¨Á´Â·
    xTaskCreate(task_listen_entyr, (const char *)"listen", 1024, NULL, 2, &TaskHandle_listen);
    myprint("%s\r\n",__func__);
}
