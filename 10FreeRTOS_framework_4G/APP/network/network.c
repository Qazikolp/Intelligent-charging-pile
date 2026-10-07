#include "network.h"


enum{
	_e_port_init = 0,	//初始化端口
	_e_rst_net_dev,		//复位网络设备
	_e_init_net_dev,	//初始化网络设备
	_e_connect_serv,	//连接服务器
	
	
};


static wk_net_t m_wk_net;
wk_net_t* wk_net = &m_wk_net;

static net_dev_t m_net_dev;
static net_dev_t* air7xx = &m_net_dev;


void network_entry( void )
{
	int val = 0;
	
	switch( wk_net->msta )
	{
		//初始化端口
		case _e_port_init:
			
			//创建一个网络设备，并指定网络接口,指定复位引脚
			netdev_opt->create_dev( air7xx,_e_uart1,NetRST );
		
			
		
			//初始化串口
			uart->init( air7xx->dev,115200 );
			uart->open( air7xx->dev );
		
			//初始化复位引脚
			gpio->set_mode( air7xx->RstPin,PIN_MODE_OUTPUT );
			gpio->write( air7xx->RstPin,0 );
		
			wk_net->msta++;
			break;
		
		//复位网络设备
		case _e_rst_net_dev:
			val = netdev_opt->work_rst_dev( air7xx );
			if( val != 1 ) break;
			//执行到这里，说明网络设备复位完毕
			netdev_opt->clr_sta( air7xx );
			wk_net->msta++;
			wk_net->ssta=0;

			break;
		
		//初始化网络环境
		case _e_init_net_dev:
			val = netdev_opt->work_init_dev( air7xx );
			if( val == 0 ) break;

			//执行到这里，说明成功/或者发生了异常
			netdev_opt->clr_sta( air7xx );
			if( val == -1 )
			{//发生了异常
				wk_net->msta = _e_rst_net_dev;
				wk_net->ssta=0;
			}
			else if( val == 1 )
			{//说明成功
				wk_net->msta++;
				wk_net->ssta=0;
				
			}
			
			
			break;
		
		//连接服务器
		//工具获取地址:http://www.armsoc.cn/zxx/doc/share/
			
		case _e_connect_serv:
			val = netdev_opt->work_connect_serv( air7xx,"lz_0001","lz_123456","01" );
			if( val == 0 ) break;
		
			netdev_opt->clr_sta( air7xx );
			
			if( val == -1 )
			{//发生了不可预料的错误，直接重启网络
				wk_net->msta = _e_rst_net_dev;
				wk_net->ssta=0;
			}
			break;
		
		//循环工作
		
		default:
			break;
	}

}

