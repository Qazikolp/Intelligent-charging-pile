#include "network.h"


/*
读取模块的信息: 厂家信息，固件信息
读取卡的信息:是否插好，SIM卡ICCID号
(2025年开始，物联卡合规管理，停机后，需要1个月之内续费，否则可能停机销卡)

查看网络信息: 信号质量，注册状态，网络的附着状态

设置传输模式：透明传输，单链接

APN获取方式

激活移动厂家

查询IP地址



*/

enum{
	//------------------------------------------------------------------------
	//读取模块厂商信息
	_e_AT_CGMI_cat_start = 0,
	_e_AT_CGMI_cat_wait,
	_e_AT_CGMI_cat_work,
	
	//------------------------------------------------------------------------
	//读取固件版本信息
	_e_AT_CGMR_cat_start,		
	_e_AT_CGMR_cat_wait,
	_e_AT_CGMR_cat_work,
	
	//------------------------------------------------------------------------
	//查询卡是否插好
	_e_AT_CPIN_cat_start,		
	_e_AT_CPIN_cat_wait,
	_e_AT_CPIN_cat_work,
	
	//------------------------------------------------------------------------
	//查询 IMEI
	_e_AT_CGSN_cat_start,
	_e_AT_CGSN_cat_wait,
	_e_AT_CGSN_cat_work,
	
	//------------------------------------------------------------------------
	//查询 ICCID
	_e_AT_CCID_cat_start,
	_e_AT_CCID_cat_wait,
	_e_AT_CCID_cat_work,
	
	//------------------------------------------------------------------------
	//查询设置信号质量
	_e_AT_CSQ_cat_start,		
	_e_AT_CSQ_cat_wait,
	_e_AT_CSQ_cat_work,

	//------------------------------------------------------------------------
	//查询网络注册状态
	_e_AT_CREG_cat_start,		
	_e_AT_CREG_cat_wait,
	_e_AT_CREG_cat_work,

	//------------------------------------------------------------------------
	//查询附着GPRS网络
	_e_AT_CGATT_cat_start,		
	_e_AT_CGATT_cat_wait,
	_e_AT_CGATT_cat_work,


	//------------------------------------------------------------------------
	//设置透传模式
	_e_AT_CIPMODE_set_start,		
	_e_AT_CIPMODE_set_wait,
	_e_AT_CIPMODE_set_work,
	//------------------------------------------------------------------------
	//设置IP为单链接
	_e_AT_CIPMUX_set_start,		
	_e_AT_CIPMUX_set_wait,
	_e_AT_CIPMUX_set_work,
	//------------------------------------------------------------------------
	//设置自动获取APN
	_e_AT_CSTT_set_start,		
	_e_AT_CSTT_set_wait,
	_e_AT_CSTT_set_work,
	
	//------------------------------------------------------------------------
	//激活移动场景，激活后能获取到IP
	_e_AT_CIICR_set_start,		
	_e_AT_CIICR_set_wait,
	_e_AT_CIICR_set_work,
	
	//------------------------------------------------------------------------
	//查询IP，只有获取到IP后才能上网
	_e_AT_CIFSR_cat_start,		
	_e_AT_CIFSR_cat_wait,
	_e_AT_CIFSR_cat_work,

	//------------------------------------------------------------------------
	_e_work_init_dev_end,
	
	
	
};


static int tlen  = 0;
int air_work_init_dev( net_dev_t* netdev )
{
	int fun_ret = 0;
	char* rxbuf = netdev->rxbuf;
	
	
	switch( netdev->msta )
	{
		//------------------------------------------------------------------------
		//读取模块厂商信息
		case _e_AT_CGMI_cat_start:
			
			uart->clr_rxbuf( netdev->dev );
			
			netdev_opt->clr_rxbuf( netdev );
			NET_LINE();
			//发送AT+RESET
			netdev_opt->send_str( netdev,"AT+CGMI\r\n" );
		
			//记录当前的滴答到otick
			netdev->otick = get_sys_ticks();
		
			//切换下一个状态
			netdev->msta++;
			netdev->ssta=0;
			
			tlen = 0;
			break;
		case _e_AT_CGMI_cat_wait:
			
			//等待100ms
			netdev->ntick = get_sys_ticks();
			if( (netdev->ntick - netdev->otick) < 100 ) break;
			
			//记录当前的滴答到otick
			netdev->otick = get_sys_ticks();
		
			//切换下一个状态
			netdev->msta++;
			netdev->ssta=0;
			break;
		case _e_AT_CGMI_cat_work:
			
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
			
			//判断是否收到OK
			if( strstr( rxbuf,"OK" ) != NULL ) 
			{//成立说明收到了OK回应
				netdev->msta++;
				netdev->ssta=0;
				netdev_opt->ptf_rxbuf( netdev,__func__,__LINE__ );
				NET_LINE();
				break;
			}
			
			//处理超时 2s超时, 4次尝试,如果失败，则返回 前2步 ，如果重大错误，自动进入_e_dev_err_msta
			netdev_opt->deal_cmd_timeout( netdev,2000,4,netdev->msta - 2,__func__,__LINE__ );
			break;
		
		//------------------------------------------------------------------------
		//读取固件版本信息
		case _e_AT_CGMR_cat_start:
			
			netdev_opt->clr_rxbuf( netdev );
			NET_LINE();
			//发送AT+RESET
			netdev_opt->send_str( netdev,"AT+CGMR\r\n" );
		
			//记录当前的滴答到otick
			netdev->otick = get_sys_ticks();
		
			//切换下一个状态
			netdev->msta++;
			netdev->ssta=0;
			
			tlen = 0;
			break;		
		case _e_AT_CGMR_cat_wait:
			//等待100ms
			netdev->ntick = get_sys_ticks();
			if( (netdev->ntick - netdev->otick) < 100 ) break;
			
			//记录当前的滴答到otick
			netdev->otick = get_sys_ticks();
		
			//切换下一个状态
			netdev->msta++;
			netdev->ssta=0;
			break;
		case _e_AT_CGMR_cat_work:
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
		
			//判断是否收到OK
			if( strstr( rxbuf,"OK" ) != NULL ) 
			{//成立说明收到了OK回应
				netdev->msta++;
				netdev->ssta=0;
				netdev_opt->ptf_rxbuf( netdev,__func__,__LINE__ );
				NET_LINE();
				break;
			}
			
			//处理超时 2s超时, 4次尝试,如果失败，则返回 前2步 ，如果重大错误，自动进入_e_dev_err_msta
			netdev_opt->deal_cmd_timeout( netdev,2000,4,netdev->msta - 2,__func__,__LINE__ );
			
			break;
		
		//------------------------------------------------------------------------
		case _e_AT_CPIN_cat_start:
			NET_LINE( );
			netdev_opt->clr_rxbuf( netdev );
			netdev_opt->send_str(netdev,"AT+CPIN?\r\n");
			netdev->otick = get_sys_ticks();
			tlen = 0;
			netdev->msta++;		
			break;	
		case _e_AT_CPIN_cat_wait:
			netdev->ntick = get_sys_ticks();
			if( (netdev->ntick - netdev->otick) < 50  ) break;
			netdev->otick = netdev->ntick;
			netdev->msta++;
			break;
		case _e_AT_CPIN_cat_work:
			tlen = uart->read( netdev->dev,&( rxbuf[netdev->rx_len ]),64 );
			netdev->rx_len += tlen;
			
			if( netdev->rx_len >= Ndev_RX_BUF_LEN )
			{//检查接收缓冲区
				netdev->rx_len = 0;
				netdev->msta = netdev->msta - 2;	//回退2步
				NET_LINE( );
				break;
			}
			
			//判断是否收到OK
			if( strstr( rxbuf,"OK" ) != NULL ) 
			{//成立说明收到了OK回应
				netdev->msta++;
				netdev->ssta=0;
				netdev_opt->ptf_rxbuf( netdev,__func__,__LINE__ );
				NET_LINE();
				break;
			}
			
			//处理超时 2s超时, 4次尝试,如果失败，则退2个状态  ，如果重大错误，自动进入_e_dev_err_msta
			netdev_opt->deal_cmd_timeout( netdev,2000,4,netdev->msta - 2,__func__,__LINE__ );
			
			break;
		
		//------------------------------------------------------------------------
		//查询 IMEI
		case _e_AT_CGSN_cat_start:
			NET_LINE( );
			netdev_opt->clr_rxbuf( netdev );
			netdev_opt->send_str(netdev,"AT+CGSN\r\n");
			netdev->otick = get_sys_ticks();
			tlen = 0;
			netdev->msta++;		
			break;
		case _e_AT_CGSN_cat_wait:
			netdev->ntick = get_sys_ticks();
			if( (netdev->ntick - netdev->otick) < 50  ) break;
			netdev->otick = netdev->ntick;
			netdev->msta++;
			
			break;
		case _e_AT_CGSN_cat_work:
			tlen = uart->read( netdev->dev,&( rxbuf[netdev->rx_len ]),64 );
			netdev->rx_len += tlen;
			
			if( netdev->rx_len >= Ndev_RX_BUF_LEN )
			{//检查接收缓冲区
				netdev->rx_len = 0;
				netdev->msta = netdev->msta - 2;	//回退2步
				NET_LINE( );
				break;
			}
			
			//判断是否收到OK
			if( strstr( rxbuf,"OK" ) != NULL ) 
			{//成立说明收到了OK回应
				netdev->msta++;
				netdev->ssta=0;
				netdev_opt->ptf_rxbuf( netdev,__func__,__LINE__ );
				NET_LINE();
				break;
			}
			
			//处理超时 2s超时, 4次尝试,如果失败，则退2个状态  ，如果重大错误，自动进入_e_dev_err_msta
			netdev_opt->deal_cmd_timeout( netdev,2000,4,netdev->msta - 2,__func__,__LINE__ );
			
			break;
		
		//------------------------------------------------------------------------
		//查询 ICCID
		case _e_AT_CCID_cat_start:
			NET_LINE( );
			netdev_opt->clr_rxbuf( netdev );
			netdev_opt->send_str(netdev,"AT+CCID\r\n");
			netdev->otick = get_sys_ticks();
			tlen = 0;
			netdev->msta++;		
			break;
		case _e_AT_CCID_cat_wait:
			netdev->ntick = get_sys_ticks();
			if( (netdev->ntick - netdev->otick) < 50  ) break;
			netdev->otick = netdev->ntick;
			netdev->msta++;
			break;
		case _e_AT_CCID_cat_work:
			tlen = uart->read( netdev->dev,&( rxbuf[netdev->rx_len ]),64 );
			netdev->rx_len += tlen;
			
			if( netdev->rx_len >= Ndev_RX_BUF_LEN )
			{//检查接收缓冲区
				netdev->rx_len = 0;
				netdev->msta = netdev->msta - 2;	//回退2步
				NET_LINE( );
				break;
			}
			
			//判断是否收到OK
			if( strstr( rxbuf,"OK" ) != NULL ) 
			{//成立说明收到了OK回应
				netdev->msta++;
				netdev->ssta=0;
				netdev_opt->ptf_rxbuf( netdev,__func__,__LINE__ );
				NET_LINE();
				break;
			}
			
			//处理超时 2s超时, 4次尝试,如果失败，则退2个状态  ，如果重大错误，自动进入_e_dev_err_msta
			netdev_opt->deal_cmd_timeout( netdev,2000,4,netdev->msta - 2,__func__,__LINE__ );

			break;
		
		//------------------------------------------------------------------------
		//查询设置信号质量
		case _e_AT_CSQ_cat_start:
			NET_LINE( );
			netdev_opt->clr_rxbuf( netdev );
			netdev_opt->send_str(netdev,"AT+CSQ\r\n");
			netdev->otick = get_sys_ticks();
			tlen = 0;
			netdev->msta++;		
			break;		
		case _e_AT_CSQ_cat_wait:
			netdev->ntick = get_sys_ticks();
			if( (netdev->ntick - netdev->otick) < 50  ) break;
			netdev->otick = netdev->ntick;
			netdev->msta++;
			break;
		case _e_AT_CSQ_cat_work:
			tlen = uart->read( netdev->dev,&( rxbuf[netdev->rx_len ]),64 );
			netdev->rx_len += tlen;
			
			if( netdev->rx_len >= Ndev_RX_BUF_LEN )
			{//检查接收缓冲区
				netdev->rx_len = 0;
				netdev->msta = netdev->msta - 2;	//回退2步
				NET_LINE( );
				break;
			}
			
			//判断是否收到OK
			if( strstr( rxbuf,"OK" ) != NULL ) 
			{//成立说明收到了OK回应
				netdev->msta++;
				netdev->ssta=0;
				netdev_opt->ptf_rxbuf( netdev,__func__,__LINE__ );
				NET_LINE();
				break;
			}
			
			//处理超时 2s超时, 4次尝试,如果失败，则退2个状态  ，如果重大错误，自动进入_e_dev_err_msta
			netdev_opt->deal_cmd_timeout( netdev,2000,4,netdev->msta - 2,__func__,__LINE__ );
			
			break;

		//------------------------------------------------------------------------
		//查询网络注册状态
		case _e_AT_CREG_cat_start:
			NET_LINE( );
			netdev_opt->clr_rxbuf( netdev );
			netdev_opt->send_str(netdev,"AT+CREG?\r\n");
			netdev->otick = get_sys_ticks();
			tlen = 0;
			netdev->msta++;		
			break;		
		case _e_AT_CREG_cat_wait:
			netdev->ntick = get_sys_ticks();
			if( (netdev->ntick - netdev->otick) < 50  ) break;
			netdev->otick = netdev->ntick;
			netdev->msta++;
			break;
		case _e_AT_CREG_cat_work:
			tlen = uart->read( netdev->dev,&( rxbuf[netdev->rx_len ]),64 );
			netdev->rx_len += tlen;
			
			if( netdev->rx_len >= Ndev_RX_BUF_LEN )
			{//检查接收缓冲区
				netdev->rx_len = 0;
				netdev->msta = netdev->msta - 2;	//回退2步
				NET_LINE( );
				break;
			}
			
			//判断是否收到OK
			if( strstr( rxbuf,"OK" ) != NULL ) 
			{//成立说明收到了OK回应
				netdev->msta++;
				netdev->ssta=0;
				netdev_opt->ptf_rxbuf( netdev,__func__,__LINE__ );
				NET_LINE();
				break;
			}
			
			//处理超时 2s超时, 4次尝试,如果失败，则退2个状态  ，如果重大错误，自动进入_e_dev_err_msta
			netdev_opt->deal_cmd_timeout( netdev,2000,4,netdev->msta - 2,__func__,__LINE__ );
			
			break;

		//------------------------------------------------------------------------
		//查询附着GPRS网络
		case _e_AT_CGATT_cat_start:
			NET_LINE( );
			netdev_opt->clr_rxbuf( netdev );
			netdev_opt->send_str(netdev,"AT+CGATT?\r\n");
			netdev->otick = get_sys_ticks();
			tlen = 0;
			netdev->msta++;		
			break;		
		case _e_AT_CGATT_cat_wait:
			netdev->ntick = get_sys_ticks();
			if( (netdev->ntick - netdev->otick) < 50  ) break;
			netdev->otick = netdev->ntick;
			netdev->msta++;
			break;
		case _e_AT_CGATT_cat_work:
			tlen = uart->read( netdev->dev,&( rxbuf[netdev->rx_len ]),64 );
			netdev->rx_len += tlen;
			
			if( netdev->rx_len >= Ndev_RX_BUF_LEN )
			{//检查接收缓冲区
				netdev->rx_len = 0;
				netdev->msta = netdev->msta - 2;	//回退2步
				NET_LINE( );
				break;
			}
			
			//判断是否收到OK
			if( strstr( rxbuf,"OK" ) != NULL ) 
			{//成立说明收到了OK回应
				netdev->msta++;
				netdev->ssta=0;
				netdev_opt->ptf_rxbuf( netdev,__func__,__LINE__ );
				NET_LINE();
				break;
			}
			
			//处理超时 2s超时, 4次尝试,如果失败，则退2个状态  ，如果重大错误，自动进入_e_dev_err_msta
			netdev_opt->deal_cmd_timeout( netdev,2000,4,netdev->msta - 2,__func__,__LINE__ );
			
			break;


		//------------------------------------------------------------------------
		//设置透传模式
		//设置透传模式
		case _e_AT_CIPMODE_set_start:
			NET_LINE( );
			netdev_opt->clr_rxbuf( netdev );
			netdev_opt->send_str(netdev,"AT+CIPMODE=1\r\n");
			netdev->otick = get_sys_ticks();
			tlen = 0;
			netdev->msta++;		
			break;		
		case _e_AT_CIPMODE_set_wait:
			netdev->ntick = get_sys_ticks();
			if( (netdev->ntick - netdev->otick) < 50  ) break;
			netdev->otick = netdev->ntick;
			netdev->msta++;
			break;
		case _e_AT_CIPMODE_set_work:
			tlen = uart->read( netdev->dev,&( rxbuf[netdev->rx_len ]),64 );
			netdev->rx_len += tlen;
			
			if( netdev->rx_len >= Ndev_RX_BUF_LEN )
			{//检查接收缓冲区
				netdev->rx_len = 0;
				netdev->msta = netdev->msta - 2;	//回退2步
				NET_LINE( );
				break;
			}
			
			//判断是否收到OK
			if( strstr( rxbuf,"OK" ) != NULL ) 
			{//成立说明收到了OK回应
				netdev->msta++;
				netdev->ssta=0;
				netdev_opt->ptf_rxbuf( netdev,__func__,__LINE__ );
				NET_LINE();
				break;
			}
			
			//处理超时 2s超时, 4次尝试,如果失败，则退2个状态  ，如果重大错误，自动进入_e_dev_err_msta
			netdev_opt->deal_cmd_timeout( netdev,2000,4,netdev->msta - 2,__func__,__LINE__ );
			
			break;
		//------------------------------------------------------------------------
		//设置IP为单链接
		case _e_AT_CIPMUX_set_start:
			NET_LINE( );
			netdev_opt->clr_rxbuf( netdev );
			netdev_opt->send_str(netdev,"AT+CIPMUX=0\r\n");
			netdev->otick = get_sys_ticks();
			tlen = 0;
			netdev->msta++;		
			break;		
		case _e_AT_CIPMUX_set_wait:
			netdev->ntick = get_sys_ticks();
			if( (netdev->ntick - netdev->otick) < 50  ) break;
			netdev->otick = netdev->ntick;
			netdev->msta++;
			break;
		case _e_AT_CIPMUX_set_work:
			tlen = uart->read( netdev->dev,&( rxbuf[netdev->rx_len ]),64 );
			netdev->rx_len += tlen;
			
			if( netdev->rx_len >= Ndev_RX_BUF_LEN )
			{//检查接收缓冲区
				netdev->rx_len = 0;
				netdev->msta = netdev->msta - 2;	//回退2步
				NET_LINE( );
				break;
			}
			
			//判断是否收到OK
			if( strstr( rxbuf,"OK" ) != NULL ) 
			{//成立说明收到了OK回应
				netdev->msta++;
				netdev->ssta=0;
				netdev_opt->ptf_rxbuf( netdev,__func__,__LINE__ );
				NET_LINE();
				break;
			}
			
			//处理超时 2s超时, 4次尝试,如果失败，则退2个状态  ，如果重大错误，自动进入_e_dev_err_msta
			netdev_opt->deal_cmd_timeout( netdev,2000,4,netdev->msta - 2,__func__,__LINE__ );
			
			break;
		//------------------------------------------------------------------------
		//设置自动获取APN
		case _e_AT_CSTT_set_start:
			NET_LINE( );
			netdev_opt->clr_rxbuf( netdev );
			//AT+CSTT="","",""
			netdev_opt->send_str(netdev,"AT+CSTT=\"\",\"\",\"\"\r\n");
			netdev->otick = get_sys_ticks();
			tlen = 0;
			netdev->msta++;	
			break;		
		case _e_AT_CSTT_set_wait:
			netdev->ntick = get_sys_ticks();
			if( (netdev->ntick - netdev->otick) < 50  ) break;
			netdev->otick = netdev->ntick;
			netdev->msta++;
			break;
		case _e_AT_CSTT_set_work:
			tlen = uart->read( netdev->dev,&( rxbuf[netdev->rx_len ]),64 );
			netdev->rx_len += tlen;
			
			if( netdev->rx_len >= Ndev_RX_BUF_LEN )
			{//检查接收缓冲区
				netdev->rx_len = 0;
				netdev->msta = netdev->msta - 2;	//回退2步
				NET_LINE( );
				break;
			}
			
			//判断是否收到OK
			if( strstr( rxbuf,"OK" ) != NULL ) 
			{//成立说明收到了OK回应
				netdev->msta++;
				netdev->ssta=0;
				netdev_opt->ptf_rxbuf( netdev,__func__,__LINE__ );
				NET_LINE();
				break;
			}
			
			//处理超时 2s超时, 4次尝试,如果失败，则退2个状态  ，如果重大错误，自动进入_e_dev_err_msta
			netdev_opt->deal_cmd_timeout( netdev,2000,4,netdev->msta - 2,__func__,__LINE__ );
			
			break;
		
		//------------------------------------------------------------------------
		//激活移动场景，激活后能获取到IP
		case _e_AT_CIICR_set_start:
			NET_LINE( );
			netdev_opt->clr_rxbuf( netdev );
			netdev_opt->send_str(netdev,"AT+CIICR\r\n");
			netdev->otick = get_sys_ticks();
			tlen = 0;
			netdev->msta++;		
			break;		
		case _e_AT_CIICR_set_wait:
			netdev->ntick = get_sys_ticks();
			if( (netdev->ntick - netdev->otick) < 50  ) break;
			netdev->otick = netdev->ntick;
			netdev->msta++;
			break;
		case _e_AT_CIICR_set_work:
			tlen = uart->read( netdev->dev,&( rxbuf[netdev->rx_len ]),64 );
			netdev->rx_len += tlen;
			
			if( netdev->rx_len >= Ndev_RX_BUF_LEN )
			{//检查接收缓冲区
				netdev->rx_len = 0;
				netdev->msta = netdev->msta - 2;	//回退2步
				NET_LINE( );
				break;
			}
			
			//判断是否收到OK
			if( strstr( rxbuf,"OK" ) != NULL ) 
			{//成立说明收到了OK回应
				netdev->msta++;
				netdev->ssta=0;
				netdev_opt->ptf_rxbuf( netdev,__func__,__LINE__ );
				NET_LINE();
				break;
			}
			
			//处理超时 2s超时, 4次尝试,如果失败，则退2个状态  ，如果重大错误，自动进入_e_dev_err_msta
			netdev_opt->deal_cmd_timeout( netdev,2000,4,netdev->msta - 2,__func__,__LINE__ );
			
			break;
		
		//------------------------------------------------------------------------
		//查询IP，只有获取到IP后才能上网
		case _e_AT_CIFSR_cat_start:
			NET_LINE( );
			netdev_opt->clr_rxbuf( netdev );
			netdev_opt->send_str(netdev,"AT+CIFSR\r\n");
			netdev->otick = get_sys_ticks();
			tlen = 0;
			netdev->msta++;
			break;		
		case _e_AT_CIFSR_cat_wait:
			netdev->ntick = get_sys_ticks();
			if( (netdev->ntick - netdev->otick) < 1000  ) break;
			netdev->otick = netdev->ntick;
			netdev->msta++;
			break;
		case _e_AT_CIFSR_cat_work:
			tlen = uart->read( netdev->dev,&( rxbuf[netdev->rx_len ]),64 );
			netdev->rx_len += tlen;
			
			if( netdev->rx_len >= Ndev_RX_BUF_LEN )
			{//检查接收缓冲区
				netdev->rx_len = 0;
				netdev->msta = netdev->msta - 2;	//回退2步
				NET_LINE( );
				break;
			}
			
			//检查是否收到 回车			
			if( strstr( (char*)rxbuf,"\r\n") != NULL )
			{				
				//保存IP
				//todo...
				netdev->msta ++;
				netdev->ssta = 0;
				netdev_opt->ptf_rxbuf( netdev,__func__,__LINE__ );
				NET_LINE( );
			
				
				break;
			}
			
			//处理超时 2s超时, 4次尝试,如果失败，则退2个状态  ，如果重大错误，自动进入_e_dev_err_msta
			netdev_opt->deal_cmd_timeout( netdev,2000,4,netdev->msta - 2,__func__,__LINE__ );
			
			break;

		//------------------------------------------------------------------------
		case _e_work_init_dev_end:
			NET_LINE( );
			netdev->msta = _e_dev_done_msta;
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


