#include "network.h"


//连接服务器

//根据协议，登录服务器


//进行数据交互


//如果发生了异常，断开服务器


//如果发生了异常，处理超时


enum{
	//------------------------------------------------------------------------
	//连接服务器
	_e_AT_CIPSTART_start = 0,
	_e_AT_CIPSTART_wait,
	_e_AT_CIPSTART_work,
	
	//------------------------------------------------------------------------
	//根据协议，登录服务器
	_e_Login_start,
	_e_Login_wait,
	_e_Login_work,
	
	//------------------------------------------------------------------------
	//进行数据交互
	_e_msg_cycle,
	_e_clr_swap,
	
	//------------------------------------------------------------------------
	//处理服务器断开异常
	_e_AT_CIPCLOSE_start,
	_e_AT_CIPCLOSE_wait,
	_e_AT_CIPCLOSE_work,
	
	//------------------------------------------------------------------------
	//处理超时异常
	_e_deal_timeout,
	
	
	
};

//netdev 网卡
//usr: 终端用户名 
//pwd: 终端密码
//vision: 版本号


static int tlen  = 0;

int air_work_connect_serv( net_dev_t* netdev,char* usr,char* pwd,char* vision )
{
	int fun_ret = 0;
	char* rxbuf = netdev->rxbuf;
	
	switch( netdev->msta )
	{
		//------------------------------------------------------------------------
		//连接服务器
		case _e_AT_CIPSTART_start:
			NET_LINE( );
			netdev_opt->clr_rxbuf( netdev );
			netdev_opt->send_str(netdev,"AT+CIPSTART=");
			netdev_opt->send_str(netdev,"\"TCP\"");
			netdev_opt->send_str(netdev,",");
			netdev_opt->send_str(netdev,"\"www.armsoc.cn\"");
			netdev_opt->send_str(netdev,",");
			netdev_opt->send_str(netdev,"\"9002\"");
			netdev_opt->send_str(netdev,"\r\n");
			
			netdev->otick = get_sys_ticks();
			tlen = 0;
			netdev->msta++;
		
			break;
		case _e_AT_CIPSTART_wait:
			//等待100ms
			netdev->ntick = get_sys_ticks();
			if( (netdev->ntick - netdev->otick) < 100 ) break;
			
			//记录当前的滴答到otick
			netdev->otick = get_sys_ticks();
		
			//切换下一个状态
			netdev->msta++;
			netdev->ssta=0;
			break;
		case _e_AT_CIPSTART_work:
			//接收数据到rxbuf
			tlen = uart->read( netdev->dev,&( rxbuf[netdev->rx_len ]),64 );
			netdev->rx_len += tlen;
			
			if( netdev->rx_len >= Ndev_RX_BUF_LEN )
			{//检查接收缓冲区
				netdev->rx_len = 0;
				netdev->msta = netdev->msta - 2;	//回退2步
				NET_LINE( );
				break;
			}
			
			//判断是否收到 CONNECT
			if( strstr( rxbuf,"CONNECT" ) != NULL ) 
			{//成立说明连接上服务器
				netdev->msta++;
				netdev->ssta=0;
				netdev_opt->ptf_rxbuf( netdev,__func__,__LINE__ );
				NET_LINE();
				break;
			}
		
			if( strstr( rxbuf,"ERROR" ) != NULL ) 
			{//成立说明异常
				netdev->msta = _e_dev_err_msta;
				netdev->ssta=0;
				netdev_opt->ptf_rxbuf( netdev,__func__,__LINE__ );
				NET_LINE();
				break;
			}
			
			break;
	
		//------------------------------------------------------------------------
		//根据协议，登录服务器
			case _e_Login_start:
				NET_LINE();
				netdev_opt->clr_rxbuf( netdev );
			
				netdev_opt->clr_txbuf( netdev );
			
				snprintf( netdev->txbuf,Ndev_TX_BUF_LEN,"LOGIN %s %s %s\r\n",usr,pwd,vision );
				netdev_opt->send_str(netdev, netdev->txbuf );
				
				netdev->otick = get_sys_ticks();
				tlen = 0;
				netdev->msta++;
			
				break;
			case _e_Login_wait:
				//等待100ms
				netdev->ntick = get_sys_ticks();
				if( (netdev->ntick - netdev->otick) < 100 ) break;
				
				//记录当前的滴答到otick
				netdev->otick = get_sys_ticks();
			
				//切换下一个状态
				netdev->msta++;
				netdev->ssta=0;
				break;
			case _e_Login_work:
				//接收数据到rxbuf
			tlen = uart->read( netdev->dev,&( rxbuf[netdev->rx_len ]),64 );
			netdev->rx_len += tlen;
			
			if( netdev->rx_len >= Ndev_RX_BUF_LEN )
			{//检查接收缓冲区
				netdev->rx_len = 0;
				netdev->msta = netdev->msta - 2;	//回退2步
				NET_LINE( );
				break;
			}
			
			//判断是否收到 Logon
			if( strstr( rxbuf,"Logon" ) != NULL ) 
			{//成立 说明登录成功
				NET_LOG("LOGON->>>>>>>>>>work cycle\r\n");
				
				//网络灯点亮
				//Todo...
				
				netdev->msta++;
				netdev->ssta=0;
				netdev_opt->ptf_rxbuf( netdev,__func__,__LINE__ );
				NET_LINE();
				break;
			}
		
			if( strstr( rxbuf,"ERROR" ) != NULL ) 
			{//成立说明异常
				netdev->msta = _e_dev_err_msta;
				netdev->ssta=0;
				netdev_opt->ptf_rxbuf( netdev,__func__,__LINE__ );
				NET_LINE();
				break;
			}
				
				break;
		
		//------------------------------------------------------------------------
		//进行数据交互
			case _e_msg_cycle:
				
				
				break;
			case _e_clr_swap:
				break;
		
		//------------------------------------------------------------------------
		//处理服务器断开异常
			case _e_AT_CIPCLOSE_start:
				break;
			case _e_AT_CIPCLOSE_wait:
				break;
			case _e_AT_CIPCLOSE_work:
				break;
		
		//------------------------------------------------------------------------
		//处理超时异常
			case _e_deal_timeout:
				break;
		
		//------------------------------------------------------------------------
		case _e_dev_done_msta:
			fun_ret = 1;
			break;
		
		case _e_dev_err_msta:
			fun_ret = -1;
			break;
	
		default:
			netdev->msta = _e_dev_err_msta;
			break;
		
		
	}		
	

	return fun_ret;
}






