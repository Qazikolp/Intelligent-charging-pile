#include "SWM320.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

/* 
 start 
    -初始化一些外设
    -创建host     主机任务
    -创建listen   监听任务
    -创建ctrl     工作任务
    
    创建了多个任务之后，打印是乱的，不完整的
    如何解决
   （1）对打印这个操作上锁
    (2) 通过队列的方式，把打印的内容拍好，再统一打印 （缺点：浪费很多内存）
    
(1)创建任务的方法
    //静态创建一个任务句柄
    static TaskHandle_t TaskHandle_start;
    //实现任务的入口，相当于main函数
    void task_start_entyr(void *arg)
    {
        while(1)
        {
        
        }
    }
    //创建任务
    void create_task_of_start(void)
    {
        //创建链路
        xTaskCreate(task_start_entyr, (const char *)"start", 1024, NULL, 2, &TaskHandle_start);
        printf("%s\r\n",__func__);
    }

(2)获取系统滴答计数器的值
uint32_t tick = xTaskGetTickCount();

(3)任务延时n个滴答
 vTaskDelay( n );
 
(4)ms转换成tick
tick = ms * configTICK_RATE_HZ /1000.0

pdMS_TO_TICKS( ms )

(5)创建互斥量/互斥锁，解决多任务打印紊乱问题

*/

extern void create_task_of_start(void);

void SerialInit(void);

int main(void)
{ 	
 	SystemInit();
	
	SerialInit();
	
    printf("system run.....\r\n");
    create_task_of_start();

    
//	GPIO_Init(GPIOA, PIN5, 1, 0, 0);		//调试指示信号
	
//	xTaskCreate(TaskADC, (const char *)"ADC", 128, NULL, 2, NULL);
//	xTaskCreate(TaskPWM, (const char *)"PWM", 128, NULL, 3, NULL);
	
//	queueADC = xQueueCreate(16, 2);
	
	vTaskStartScheduler();     //启动FRreeRTOS任务调度器，开启多任务
    // 调用完这个函数之后，代码就不再回到main()函数往下跑了。
}



void SerialInit(void)
{
	UART_InitStructure UART_initStruct;
	
	PORT_Init(PORTA, PIN2, FUNMUX0_UART0_RXD, 1);	//GPIOA.2配置为UART0输入引脚
	PORT_Init(PORTA, PIN3, FUNMUX1_UART0_TXD, 0);	//GPIOA.3配置为UART0输出引脚
 	
 	UART_initStruct.Baudrate = 115200;
	UART_initStruct.RXThresholdIEn = 0;
	UART_initStruct.TXThresholdIEn = 0;
	UART_initStruct.TimeoutIEn = 0;
 	UART_Init(UART0, &UART_initStruct);
	UART_Open(UART0);
}

/****************************************************************************************************************************************** 
* 函数名称: fputc()
* 功能说明: printf()使用此函数完成实际的串口打印动作
* 输    入: int ch		要打印的字符
*			FILE *f		文件句柄
* 输    出: 无
* 注意事项: 无
******************************************************************************************************************************************/
int fputc(int ch, FILE *f)
{
 	while(UART_IsTXFIFOFull(UART0));
	
	UART_WriteByte(UART0, ch);
 	
	return ch;
}
