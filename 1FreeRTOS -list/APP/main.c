#include "SWM320.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

/* 
链表
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
