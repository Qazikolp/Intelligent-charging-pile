#ifndef __network_h__
#define __network_h__

#include "framework.h"

#define NET_LOG( X... ) sysprintf(X)
#define NET_LINE() 			{NET_LOG( "%s,%d\r\n",__func__,__LINE__ );}




//约定的状态值
#define _e_dev_done_msta 	0xD0
#define _e_dev_err_msta 	0xF0

//缓冲区的尺寸
#define Ndev_RX_BUF_LEN	1024
#define Ndev_TX_BUF_LEN	1024


//网卡设备的工作参数
typedef struct{
	uint8_t msta;
	uint8_t ssta;
	
	uint8_t err_cntr;		//错误计数器
	uint8_t big_err;		//描述是否发送重大错误
	
	char* rxbuf;				//接收缓冲区
	uint32_t rx_len;		//描述接收数据的长度
	
	char* txbuf;				//发送缓冲区
	uint32_t tx_len;		//描述发送数据的长度
	
	uint32_t otick;			
	uint32_t ntick;			
	
	int dev;						//串口号
	int RstPin;					//复位引脚号
	
}net_dev_t;


//网卡设备的工作操作
typedef struct{
	
	//构建一个设备
	int (*create_dev)( net_dev_t* netdev,int dev,int RstPin );
	//清除状态
	void (*clr_sta)( net_dev_t* netdev );
	
	//清除接收数据缓冲区
	void (*clr_rxbuf)( net_dev_t* netdev );
	
	//清除发送数据缓冲区
	void (*clr_txbuf)( net_dev_t* netdev );
	
	//打印发送缓冲区的内容
	void (*ptf_txbuf)( net_dev_t* netdev,const char* call,int line );
	
	//打印接收缓冲区的内容
	void (*ptf_rxbuf)( net_dev_t* netdev,const char* call,int line );

	//发送一个字符串
	void (*send_str)( net_dev_t* netdev,char* string );
	
	//处理命令超时
	int (*deal_cmd_timeout)( net_dev_t* netdev,uint32_t to,int max_err,int re_msta,const char* call,uint32_t line );

	
	//工作: 复位设备
	int (*work_rst_dev)( net_dev_t* netdev );
	
	
	//工作: 初始化设备
	int (*work_init_dev)( net_dev_t* netdev );
	
	//工作: 构建网络环境
	int (*work_connect_serv)( net_dev_t* netdev,char* usr,char* pwd,char* vision );
	//安装循环作业时的接收回调函数

	
}netdev_opt_t;


#include "./air7xx/air7xx.h"





//定义网络任务的工作参数
typedef struct{
	uint8_t msta;		//主状态
	uint8_t ssta;		//子状态
	uint8_t connect;	//描述网络是否已经连接
	uint8_t rec;		//保留

}wk_net_t;

extern void network_entry( void );


#endif


