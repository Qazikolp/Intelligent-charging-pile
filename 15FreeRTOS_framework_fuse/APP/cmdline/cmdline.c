#include "cmdline.h"
#include <string.h>

// \r 0x0D
// \n 0x0A



typedef int (*deal_cmd_t)(int argc,char** argv);

typedef struct{
	char* name;					//字符串
	deal_cmd_t entry;		//操作函数

}cmd_t;
/*----------------------------------------------------------------------------------*/
static int get_cmdlist_len(void);
static cmd_t* get_cmdlist(void);
/*----------------------------------------------------------------------------------*/

//test/clear/help/reboot

int cmd_test( int argc,char** argv ) 
{
	sysprintf( "%s\r\n",__func__ );
	
	for( int i=0;i<argc;i++ )
	{
		sysprintf("argv[%d]:%s\r\n",i,argv[i] );
		
	}
	return 0;
	
}
int cmd_clear( int argc,char** argv ) 
{
	sysprintf( "%s\r\n",__func__ );
	return 0;
}
int cmd_help( int argc,char** argv ) 
{
	sysprintf( "%s\r\n",__func__ );
	cmd_t* cmdlist = get_cmdlist();
	for( int i=0;i<get_cmdlist_len();i++ )
	{
		sysprintf( "\t%d:%s\r\n",i,cmdlist[i].name );
	}
	sysprintf( "\r\n" );
	return 0;
}

int cmd_reboot( int argc,char** argv ) 
{
	sysprintf( "%s\r\n",__func__ );
	return 0;
}
/*----------------------------------------------------------------------------------*/


static cmd_t __cmd_list[]={
	{"test",	cmd_test},
	{"clear",	cmd_clear},
	{"help",	cmd_help},
	{"reboot",cmd_reboot},
	
};

static int get_cmdlist_len(void)
{
	return ITEM_NUM( __cmd_list );
}

static cmd_t* get_cmdlist(void)
{
	return __cmd_list;
}

/*----------------------------------------------------------------------------------*/


#define MaxArgc	10		//最多支持10个输入参数
#define MaxLen 128
char rxbuf[ MaxLen ];	//接收缓冲区，存储从环形缓冲区读取出来数据
int rx_len = 0;				//描述已获取数据长度



/*
状态机实现

状态0:
	打印提示符
	


状态1:
	等回车符：如果收到的数据，不是回车符，则直接打印出来

	否则，分割字符串，解析，执行

	
	学习一个宏,获取数组的元素个数


*/
static int sta = 0;

void cmd_line_work( void )
{
	uint8_t* buf = NULL;
	int ret = 0;
	
	int argc = 0;
	char* argv[MaxArgc];
	
	switch( sta )
	{
		//打印提示符
		case 0:
			sysprintf("\r\n>");
			memset( rxbuf,0,sizeof( rxbuf ) );
			rx_len = 0;
			sta++;
			break;
		
		//等回车符
		case 1:
			
			//每次读取一个字节数据出来
			buf = (uint8_t*)( &(rxbuf[rx_len])  );
//			int ret = loopbuf_read( lb_uart0,buf,1 );
			ret = uart->read( _e_uart0,buf,1 );
			if( ret == 0 ) break;
			rx_len += ret;
			
			
		
			//需要判断缓冲区是否溢出
			if( rx_len >= MaxLen )
			{//成立，说明缓冲区满了
				sta = 0;
				break;
			}
			
			
			switch( buf[0] )
			{
				
				case 0x09://TAB键
					//检索适配命令，并打印出来提示
				
					break;
				
				case 0x7F:
				case '\b':					
					//需要先搞懂 rx_len 代表什么含义
					//rx_len 此时，代表了收到退格符之后，下一个接收数据的数组下标
					rx_len --;
					//rx_len 此时，代表了退格符的数组下标
					rxbuf[ rx_len ] = 0;	//清除退格符号
					
					//按下退格键的目的，是为了把原有的数据，清掉一个
					
					//先判断原来有没有数据
					if( rx_len == 0 )
					{//说明，原本是没有数据的
						
					}
					else
					{//说明，原本是有数据的
						rx_len--;
						//rx_len 此时,代表了原有数据的最后一个字符的数组下标
						rxbuf[ rx_len ] = '\0';	//清除原有数据的最后一个字符 
						
					}
					sysprintf("\r\n>%s",rxbuf);
					
				
					
					break;
				
				case '\n':
					//有些串口工具发送回车换行，\n不是我们想要的
					//如果命令首部是\n，则直接丢弃。
					if( rx_len == 1 )	
						rx_len = 0;
					break;
					
				case '\r':
					argc = stropt->split_string_n( MaxArgc,rxbuf,argv," \t\n\r" );
					if( argc != 0 )
					{
						
						//在命令表里面匹配 argv[0]
						for( int i=0;i< (ITEM_NUM( __cmd_list ) );i++ )
						{
							if( 0 == strncmp( rxbuf,__cmd_list[i].name ,strlen( __cmd_list[i].name ) ) )
							{//成立，说明字符串一致，找到相应的命令了
								sysprintf("\r\n");
								__cmd_list[i].entry( argc,argv );
								
								sta = 0;
								return;
							}

						}
						sysprintf("\r\n\tunknow");
					}
			
					sta = 0;
		
				
					break;
				
				default:
					//直接打印出来(回显)
					sysprintf("%c",buf[0] );
					break;
				
			}
			
			break;
		
		default:
			break;
	
	
	}
}

