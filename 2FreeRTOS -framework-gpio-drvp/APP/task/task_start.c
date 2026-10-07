#include "framework.h"

extern void create_task_of_listen(void);
extern void create_task_of_host(void);
extern void create_task_of_ctrl(void);  

void cslock_init(void);
void myprint(char *format,...);

void task_start_entyr(void *arg)
{
    printf("%s\r\n", __func__);
    
    //初始化myprintf
    
    //创建其他任务
    cslock_init();
    create_task_of_listen();
    create_task_of_host();
    create_task_of_ctrl();
    
    //设置GPIO的模式 M2(26),B12(3)
    drvp_gpio->set_mode(3,PIN_MODE_OUTPUT);
    drvp_gpio->set_mode(26,PIN_MODE_OUTPUT);
    
   
    while(1)
    {   
        //GPIO输出高电平
        drvp_gpio->write(3,PIN_HIGH);
        drvp_gpio->write(26,PIN_HIGH);
        
        //延时1000ms
        vTaskDelay(pdMS_TO_TICKS( 1000 ));
        
        //GPIO输出低电平
        drvp_gpio->write(3,PIN_LOW);
        drvp_gpio->write(26,PIN_LOW);
        
        //延时1000ms
        vTaskDelay(pdMS_TO_TICKS( 1000 ));
       
    }
}

static TaskHandle_t  TaskHandle_start;

void create_task_of_start(void)
{
    //创建链路
    xTaskCreate(task_start_entyr, (const char *)"start", 1024, NULL, 2, &TaskHandle_start);
    myprint("%s\r\n",__func__);

}

//创建互斥锁

static SemaphoreHandle_t cslock;
static int flag_int = 0;

void cslock_init(void)    //初始化锁
{
    if(flag_int == 1)   return;
    cslock = xSemaphoreCreateMutex();     //创建二进制信号量，初始值=0
    if(cslock == NULL)
    {
        printf("err:%s\r\n",__func__);
        while(1);
    }
    flag_int = 1;
}

void cslock_get(void)       //获取锁
{
    if(flag_int == 0)   return;
    //获取cslock，最多等待100个tick，超时返回 pdFALSE，成功返回pdTRUE
    int ret = xSemaphoreTake(cslock, 100);   //取信号量，信号量为 0 时任务阻塞等待
    if(ret != pdTRUE)
    {
        return;//说明成功获取了
    }
        
}

void cslock_free(void)       //释放/归还信号量
{
    if(flag_int == 0)   return;
    xSemaphoreGive(cslock);  //给出信号量 → 信号量变为 1，唤醒等待它的任务
}

void myprint(char *format,...)     //输入参数个数不固定
{
    char buf[256];                 //C语言老标准，在一对大括号 `{ }` 里面，所有局部变量，必须写在最开头，所有执行代码之前。
    if(flag_int == 0)   return;
    //prinf("%s%d\r\n",a,b);

    va_list v_args;
    va_start(v_args, format);
    
    vsnprintf(buf, sizeof(buf), format, v_args);
    va_end(v_args);
    //获取锁
    cslock_get();
//多个任务同时调用`printf`，而`printf`底层最终操作 UART 串口这一个硬件共享资源。如果不加保护，多个任务的打印文字会交错、乱码。
//所以每次调用 printf 前，任务获取互斥锁 (Mutex)；打印完成之后释放互斥锁。
//如果资源已经被别的任务占用，当前任务就会阻塞等待，直到其他任务打印完毕释放锁之后，才能继续执行打印。
    printf("%s",buf);
    //释放锁
    cslock_free();
}
