#include "network.h"

//-----------------------------------------------------------------------------------


//切割字符串
//max 最多解析几个参数
//str 需要处理的字符串
//result 存放处理的结果
//delimiters 分隔符 " \t\n\r"
static int split_string_n(int max,char *str, char **result, const char *delimiters) 
{
    int count = 0;
    char *token;
    char *saveptr; // 用于保存上下文的指针

    // 使用 strtok_r 函数拆分字符串
    token = strtok_r(str, delimiters, &saveptr);

    while (token != NULL) {
        result[count] = token; // 保存当前的 token
        count++;
				if( count>=max ) break;
        // 获取下一个 token
        token = strtok_r(NULL, delimiters, &saveptr);
    }

    return count;
}


//-----------------------------------------------------------------------------------

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
#define MaxArgc 10
int air_work_connect_serv( net_dev_t* netdev,char* usr,char* pwd,char* vision )
{
	int fun_ret = 0;
	char* rxbuf = netdev->rxbuf;
	
	int argc = 0;
	//凡无名同学发现bug
	char* argv[ MaxArgc ];
	
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
			
			//处理超时 2s超时, 4次尝试,如果失败，则返回 前2步 ，如果重大错误，自动进入_e_dev_err_msta
			netdev_opt->deal_cmd_timeout( netdev,2000,4,netdev->msta-2,__func__,__LINE__ );
			
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
				
				//处理超时 2s超时, 4次尝试,如果失败，则返回 前2步 ，如果重大错误，自动进入_e_dev_err_msta
				netdev_opt->deal_cmd_timeout( netdev,2000,4,netdev->msta-2,__func__,__LINE__ );
				break;
		
		//------------------------------------------------------------------------
		//进行数据交互
			case _e_msg_cycle:
				
				//负责循环作业，无论是否收到数据，都会执行
				//net_cycle( netdev );
			
				//负责解析收到的数据，收到数据才会执行
				
				tlen = uart->read( netdev->dev,&( rxbuf[netdev->rx_len ]),64 );
				netdev->rx_len += tlen;
				
				if( netdev->rx_len >= Ndev_RX_BUF_LEN )
				{//检查接收缓冲区
					netdev->rx_len = 0;
					NET_LINE( );
					break;
				}
				
				//处理超时 30s超时, 4次尝试,如果失败，则进入 _e_AT_CIPCLOSE_start ，如果重大错误，自动进入_e_dev_err_msta
				netdev_opt->deal_cmd_timeout( netdev,30000,4,_e_AT_CIPCLOSE_start,__func__,__LINE__ );
				
				//检查是否收到 回车
				if( strstr( rxbuf,"\r\n" ) == NULL ) break; 

				//执行到这里, 说明收到了回车
				netdev_opt->ptf_rxbuf( netdev,__func__,__LINE__ );
				
				//拆分参数
				
				argc = split_string_n( MaxArgc,rxbuf,argv," \t\n\r" );
				//参数不对，则跳转到 _e_clr_swap 清空数据
				if( argc == 0 ) { netdev->msta = _e_clr_swap;break;}

				//执行解析消息的函数( 约定返回值,0 不做任何处理, -1 则需要重新登录服务器 )
				//int cmd_ret = cmd_cycle( netdev,argc,argv );
				netdev->msta = _e_clr_swap;
				break;
				
			case _e_clr_swap:
				netdev_opt->clr_rxbuf( netdev );
				netdev->otick = get_sys_ticks();
				netdev->msta -- ;
				tlen = 0;
				
				break;
		
		//------------------------------------------------------------------------
		//处理服务器断开异常
			case _e_AT_CIPCLOSE_start:
				NET_LINE( );
				//退出透传模式
				thread_delay_ms( 1500 );
				netdev_opt->send_str( netdev, "+++");
				thread_delay_ms( 1000 );
			
				netdev_opt->clr_rxbuf( netdev );
				netdev_opt->send_str( netdev, "AT+CIPCLOSE\r\n");
				
				netdev->otick = get_sys_ticks();
				netdev->msta ++ ;
				tlen = 0;
				break;
			
			case _e_AT_CIPCLOSE_wait:
				//等待100ms
				netdev->ntick = get_sys_ticks();
				if( (netdev->ntick - netdev->otick) < 100 ) break;
				
				//记录当前的滴答到otick
				netdev->otick = get_sys_ticks();
			
				//切换下一个状态
				netdev->msta++;
				netdev->ssta=0;
				break;
			
			case _e_AT_CIPCLOSE_work:
				tlen = uart->read( netdev->dev,&( rxbuf[netdev->rx_len ]),64 );
				netdev->rx_len += tlen;
				
				if( netdev->rx_len >= Ndev_RX_BUF_LEN )
				{//检查接收缓冲区
					netdev->rx_len = 0;
					netdev->msta = _e_dev_err_msta;
					NET_LINE( );
					break;
				}
			
				//检查是否收到 OK
				if( strstr( rxbuf,"OK" ) != NULL )
				{
					netdev_opt->ptf_rxbuf( netdev,__func__,__LINE__ );
					netdev->msta = _e_dev_err_msta + 1;
					NET_LINE( );
					break;
				}
				
				//检查是否收到 ERROR
				if( strstr( rxbuf,"ERROR" ) != NULL )
				{
					netdev_opt->ptf_rxbuf( netdev,__func__,__LINE__ );
					netdev->msta = _e_dev_err_msta;
					NET_LINE( );
					break;
				}
				
				//处理超时 2s超时, 4次尝试,如果失败，则返回 _e_dev_err_msta ，如果重大错误，自动进入_e_dev_err_msta
				netdev_opt->deal_cmd_timeout( netdev,2000,4,_e_dev_err_msta,__func__,__LINE__ );
				
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
		
		case (_e_dev_err_msta+1):
			fun_ret = -2;
			break;
		
		default:
			netdev->msta = _e_dev_err_msta;
			break;
		
		
	}		
	

	return fun_ret;
}






