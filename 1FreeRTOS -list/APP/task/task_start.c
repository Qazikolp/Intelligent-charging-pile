#include "SWM320.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "semphr.h"

#include <stdarg.h>
#include <string.h>


extern void create_task_of_listen(void);
extern void create_task_of_host(void);
extern void create_task_of_ctrl(void);
void cslock_init(void);
void cslock_get(void);
void cslock_free(void);
void myprint(char *format,...);
void task_start_entyr(void *arg)
{
    uint32_t tick = 0;
    printf("%s\r\n", __func__);
    
    //初始化myprintf
    
    //创建其他任务
    cslock_init();
    create_task_of_listen();
    create_task_of_host();
    create_task_of_ctrl();
    
//链表示例-----------------------------------------------------------------------------
    List_t mylist;
    ListItem_t node0;
    ListItem_t node1;
    ListItem_t node2;

    //①创建一个链表的根节点
    vListInitialise(&mylist);
    
    //②n个节点初始化
    vListInitialiseItem(&node0);
    vListInitialiseItem(&node1);
    vListInitialiseItem(&node2);
                        
    node0.xItemValue = 3;
    node1.xItemValue = 5;
    node2.xItemValue = 2;
    
    //③把这些节点插入到链表中去
    #if (0)
        //排序插入，将节点按小到大插入
        vListInsert(&mylist, &node0);
        vListInsert(&mylist, &node1);
        vListInsert(&mylist, &node2);
        
    #else
        //插入到尾部
    
        vListInsertEnd(&mylist, &node0);
        vListInsertEnd(&mylist, &node1);
        vListInsertEnd(&mylist, &node2);
        
    #endif
    ListItem_t *node = listGET_HEAD_ENTRY(&mylist);
    while(1)
    {
        //最后一个节点固定为根节点
        //判断是否为最后一个节点，如果是，则退出
        if(node == listGET_END_MARKER(&mylist)) break;
        
        myprint("val = %08x\r\n", node->xItemValue); //如果不是则打印
        
        node = node->pxNext;                         //并指向下移节点
    }
//-------------------------------------------------------------------------------------    
    
    
    while(1)
    {   //延时1000ms
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
    if(ret == pdTRUE)
    {
        //说明成功获取了
    }
    else
    {
        //说明获取失败
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
