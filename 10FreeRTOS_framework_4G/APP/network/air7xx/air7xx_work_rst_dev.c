#include "network.h"


/*
复位4G模块的流程

(1)通过复位引脚复位
	//引脚号
	
(2)软件复位
	AT+RESET

(3)需要判断是否成功复位

(4)等待一阵子，4G模块连接移动网络



*/

enum{

	//--------------------------------------------
	_e_service_init = 0,			//服务初始化
	//--------------------------------------------	
	_e_rst_pin_ctrl_0,				//复位引脚控制步骤0
	_e_rst_pin_ctrl_1,				//复位引脚控制步骤1
	_e_rst_pin_ctrl_2,				//复位引脚控制步骤2
	//--------------------------------------------
	_e_AT_RESET,							//软件命令复位
	_e_wait_some_tick,				//等待一些时间
	_e_wait_ready,						//等待就绪
	//--------------------------------------------
	_e_wait_some_time,				//等待一段时间

};

static int tlen  = 0;
static int wait_ready( net_dev_t* netdev );


int air_work_rst_dev( net_dev_t* netdev )
{
	int ret = 0;
	
	switch( netdev->msta )
	{
		//--------------------------------------------
		case _e_service_init:
			NET_LINE();
			netdev->msta++;
			netdev->ssta=0;
			break;
		//--------------------------------------------
		
		case _e_rst_pin_ctrl_0:
			NET_LINE();
		
			//网络指示灯熄灭
			//TODO...
			
			//清除接收缓冲区
			netdev_opt->clr_rxbuf( netdev );
			
			//复位引脚输出低电平
			gpio->write( netdev->RstPin,0 );
			
			//记录当前的滴答到otick
			netdev->otick = get_sys_ticks();
			
			//切换下一个状态
			netdev->msta++;
			netdev->ssta=0;
			break;
		
		case _e_rst_pin_ctrl_1:
			//等待100ms
			netdev->ntick = get_sys_ticks();
			if( (netdev->ntick - netdev->otick) < 100 ) break;
		
			NET_LINE();
			//复位引脚输出高电平
			gpio->write( netdev->RstPin,1 );
			//记录当前的滴答到otick
			netdev->otick = get_sys_ticks();
		
			//切换下一个状态
			netdev->msta++;
			netdev->ssta=0;
		
			break;
		
		case _e_rst_pin_ctrl_2:
			//等待100ms
			netdev->ntick = get_sys_ticks();
			if( (netdev->ntick - netdev->otick) < 100 ) break;
		
			NET_LINE();
		
			//复位引脚输出低电平
			gpio->write( netdev->RstPin,0 );
			
			//记录当前的滴答到otick
			netdev->otick = get_sys_ticks();
		
			//切换下一个状态
			netdev->msta++;
			netdev->ssta=0;
			
			break;
		
		//--------------------------------------------	
		//指令复位，是肯定支持的，网上买的模块，不一定引出复位引脚，为了更好的兼容，所以增加指令复位
		case _e_AT_RESET:
			//等待100ms
			netdev->ntick = get_sys_ticks();
			if( (netdev->ntick - netdev->otick) < 100 ) break;
		
			NET_LINE();
			//发送AT+RESET
			netdev_opt->send_str( netdev,"AT+RESET\r\n" );
		
			//记录当前的滴答到otick
			netdev->otick = get_sys_ticks();
		
			//切换下一个状态
			netdev->msta++;
			netdev->ssta=0;

			break;
		
		case _e_wait_some_tick:
			//等待100ms
			netdev->ntick = get_sys_ticks();
			if( (netdev->ntick - netdev->otick) < 100 ) break;
			
			NET_LINE();
			//记录当前的滴答到otick
			netdev->otick = netdev->ntick;
		
			//切换下一个状态
			netdev->msta++;
			netdev->ssta=0;
		
		
			break;
		
		case _e_wait_ready:
			
			//接收数据到缓冲区，并判断是否存在"RDY"/"boot.rom"
			if( 0 != wait_ready( netdev ) )
			{//成立,说明就绪
				
				//打印收到的数据
				netdev_opt->ptf_rxbuf( netdev,__func__,__LINE__ );
				//记录当前的滴答到otick
				netdev->otick = netdev->ntick;
				//切换下一个状态
				netdev->msta++;
				netdev->ssta=0;
				
				NET_LINE();
				//清空rxbuf
				netdev_opt->clr_rxbuf( netdev );
				break;
			}
			
			//处理超时 2s超时, 4次尝试,如果失败，则返回 _e_AT_RESET ，如果重大错误，自动进入_e_dev_err_msta
			netdev_opt->deal_cmd_timeout( netdev,2000,4,_e_AT_RESET,__func__,__LINE__ );

			break;
		
		//--------------------------------------------
		case _e_wait_some_time:
			//等待10s,确保4G模块连接上移动网络
			
			//控制本状态的访问频率为500ms
			netdev->ntick = get_sys_ticks();
			if( (netdev->ntick - netdev->otick) < 500 ) break;
			
			netdev->otick = netdev->ntick;
		
			//借助ssta作为计数器，每次访问++
			netdev->ssta++;
			NET_LOG(".");
			//如果超过了10s，ssta>20
			if( netdev->ssta >= 20 )
			{//本work工作完毕
				NET_LINE();
				netdev->msta = _e_dev_done_msta;
				netdev->ssta = 0;
				
			}
			break;
		
		case _e_dev_done_msta:
			ret = 1;
			break;
		
		case _e_dev_err_msta:
			netdev_opt->clr_sta( netdev );
			break;
		
		
		default:
			break;
	}		
	

	return ret;
}

static int wait_ready( net_dev_t* netdev )
{
	
	int ret = 0;
	char* buf = netdev->rxbuf;
	
	char* str = NULL;
	
	//读取数据到接收缓冲区
//	tlen = uart->read( netdev->dev,buf,10 );
	tlen = uart->read( netdev->dev,&(buf[netdev->rx_len ]),64 );
	netdev->rx_len += tlen;
	
	
	
	//如果接收缓冲区里面，包含了"RDY",说明复位完毕
	str =  strstr( buf,"RDY" );
	if( str != NULL ) 
	{ 
		return 1; 
	
	}
	
	str =  strstr( buf,"boot.rom" );
	if( str != NULL ) 
	{ 
		return 1; 
	}
	
	
	
	return ret;
}









