#include "network.h"

extern int air_create_dev( net_dev_t* netdev,int dev,int RstPin );
extern void air_clr_sta( net_dev_t* netdev );
extern void air_clr_rxbuf( net_dev_t* netdev );
extern void air_clr_txbuf( net_dev_t* netdev );
extern void air_ptf_txbuf( net_dev_t* netdev,const char* call,int line );
extern void air_ptf_rxbuf( net_dev_t* netdev,const char* call,int line );
extern void air_send_str( net_dev_t* netdev,char* string );
extern int air_deal_cmd_timeout( net_dev_t* netdev,uint32_t to,int max_err,int re_msta,const char* call,uint32_t line );

extern int air_work_rst_dev( net_dev_t* netdev );
extern int air_work_init_dev( net_dev_t* netdev );
extern int air_work_connect_serv( net_dev_t* netdev,char* usr,char* pwd,char* vision );

static netdev_opt_t do_netdev_opt ={
	.create_dev = air_create_dev,
	.clr_sta = air_clr_sta,
	.clr_rxbuf = air_clr_rxbuf,
	.clr_txbuf = air_clr_txbuf,
	.ptf_txbuf = air_ptf_txbuf,
	.ptf_rxbuf = air_ptf_rxbuf,
	.send_str = air_send_str,
	.deal_cmd_timeout = air_deal_cmd_timeout,
	
	.work_rst_dev = air_work_rst_dev,
	.work_init_dev = air_work_init_dev,
	.work_connect_serv = air_work_connect_serv,
};

netdev_opt_t* netdev_opt = &do_netdev_opt;
