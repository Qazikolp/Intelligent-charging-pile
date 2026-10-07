#include "network.h"

/*----------------------------------------------------------------------------------*/

//构建一个设备
int air_create_dev( net_dev_t* netdev,int dev,int RstPin )
{

	netdev->dev = dev;				//绑定的串口
	netdev->RstPin = RstPin;	//绑定的复位引脚
	//分配接收buf
	netdev->rxbuf = mem_alloc( Ndev_RX_BUF_LEN );
	if( netdev->rxbuf == NULL )
	{//说明分配失败
		NET_LOG( "err:%s,%d\r\n",__func__,__LINE__ );
		return -1;
	}
	
	//分配发送buf
	netdev->txbuf = mem_alloc( Ndev_RX_BUF_LEN );
	if( netdev->txbuf == NULL )
	{//说明分配失败
		NET_LOG( "err:%s,%d\r\n",__func__,__LINE__ );
		return -2;
	}
	
	return 1;
	
}
/*----------------------------------------------------------------------------------*/
//清除状态
void air_clr_sta( net_dev_t* netdev )
{
	netdev->msta = 0;
	netdev->ssta = 0;
	netdev->err_cntr = 0;
	netdev->big_err = 0;
	
}
/*----------------------------------------------------------------------------------*/
//清除接收数据缓冲区
void air_clr_rxbuf( net_dev_t* netdev )
{
	
	memset( netdev->rxbuf,0,Ndev_RX_BUF_LEN );
	netdev->rx_len = 0;
}
/*----------------------------------------------------------------------------------*/
//清除发送数据缓冲区
void air_clr_txbuf( net_dev_t* netdev )
{
	
	memset( netdev->txbuf,0,Ndev_TX_BUF_LEN );
	netdev->tx_len = 0;
}
/*----------------------------------------------------------------------------------*/
static int str_ischar( char c )
{
	if( (c>=32) && (c<=126) ) return 1;
	
	if( c == '\r' ) return 1;
	if( c == '\n' ) return 1;
	
	
	return 0;
}

//打印缓冲区数据内容
//tr =0,说明是接收	=1 说明是发送
#define PartLen 128
static void air_ptf_buf( net_dev_t* netdev,int tr,const char* call,int line )
{
	char* pbuf = NULL;
	uint32_t len = 0;
	
	char disp[ PartLen+1 ];
	
	if( 0 == tr )
	{//接收
		pbuf = netdev->rxbuf;
		len = netdev->rx_len;
		
		NET_LOG( "\r\nptf_rxbuf[%s,%d]:\r\n",call,line );
	}
	else
		{//发送
		pbuf = netdev->txbuf;
		len = netdev->tx_len;
		NET_LOG( "\r\nptf_txbuf[%s,%d]:\r\n",call,line );
	}
	
#if 1
	while(1)
	{
		if( len <= 0 ) break;
		
		//复制128字节数据到缓冲区
		memset( disp,'\0',sizeof( disp ) );
		int batch_size = ( len> PartLen )? PartLen : len;
		for( int i=0;i<batch_size;i++ )
		{
			//disp[i] = pbuf[0];
			disp[i] = str_ischar( pbuf[0] ) ? pbuf[0]:' ';
			pbuf++;								//指针偏移
		}
		disp[batch_size ] = '\0';
		//打印出来
		NET_LOG( "%s",disp );
		
		//减少剩余长度
		len -= batch_size;

	}
#endif
	NET_LOG( "\r\n" );
}

/*----------------------------------------------------------------------------------*/

//打印发送缓冲区的内容
void air_ptf_txbuf( net_dev_t* netdev,const char* call,int line )
{
	air_ptf_buf( netdev, 1,call,line );
}

/*----------------------------------------------------------------------------------*/
//打印接收缓冲区的内容
void air_ptf_rxbuf( net_dev_t* netdev,const char* call,int line )
{
	air_ptf_buf( netdev, 0,call,line );
}
/*----------------------------------------------------------------------------------*/
//发送一个字符串
void air_send_str( net_dev_t* netdev,char* string )
{
	uint32_t index = 0;
	
	while(1)
	{
		
		//发送之前, 判断是否为字符结束符号
		if( string[index] == '\0'  ) break;
		
		uart->write( netdev->dev,string[index] );
		index++;
		//限制发送的长度
		if( index >= Ndev_TX_BUF_LEN ) break;
		
		
		
	}
	
	
	
}

/*----------------------------------------------------------------------------------*/

//处理命令超时
/*
	判断命令响应时间，是否超时,
	如果超时，则记录一次错误,重新尝试指令。
	如果记录的错误次数，超过了允许值，则认为发生重大错误，
	发生重大错误，网络任务，需要响应错误，执行对应的操作(重启模块)

netdev网络设备
to	超时时间
max_err	最大允许多少次出错
call	哪一个函数调用的
line	哪一行代码调用的



//返回0 ，说明没问题
//返回-1, 说明需要重新尝试指令
//返回-2, 说明发生重大错误，需要重启网络模块

*/
int air_deal_cmd_timeout( net_dev_t* netdev,uint32_t to,int max_err,int re_msta,const char* call,uint32_t line )
{
	int ret = 0;
	
	netdev->ntick = get_sys_ticks();	
	if( (netdev->ntick - netdev->otick) > to )
	{//成立，说明发送了错误
		netdev->err_cntr++;
		
		netdev->otick = netdev->ntick;
		//选择性打印缓冲区的内容
		//TODO
		
		air_clr_rxbuf( netdev );
		
		//判断错误次数，是否足以发送重大错误
		if( netdev->err_cntr >= max_err )
		{//成立，说明服务异常,重大错误，需要重启网络
			netdev->msta = _e_dev_err_msta;
			netdev->ssta = 0;
			ret = -2;
			NET_LOG("BigErr:%s,%d\r\n",call,line );
		}
		else
		{//重新尝试指令
			
			netdev->msta = re_msta;
			netdev->ssta = 0;
			ret = -1;
		}
	}
	
	return ret;
	
}










/*----------------------------------------------------------------------------------*/



