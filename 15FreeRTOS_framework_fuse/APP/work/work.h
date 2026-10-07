#ifndef __work_h__
#define __work_h__

#include "framework.h"

#define BootCode 0xA5

//16字节对齐(4个字)
typedef struct{
	
	//-----------------------------
	uint32_t bootcode;		//启动码
	
	uint8_t boot_app;			//启动哪一个APP
	uint8_t ota_req;			//是否存在ota请求
	uint8_t net_mode;			//联网方式
	uint8_t rec8;					//保留
	
	uint32_t rec32[2];		//保留2个字
	//-----------------------------	
	//HLW8012的线性拟合校准值
	float k[2];
	float b[2];
	
	//-----------------------------	
	char user_name[ 32 ];	//用户名
	char user_pwd [ 32 ];	//用户密码
	
}storage_t;





extern void work_data_load(void);


typedef struct{
	void (*init)(void);
	int (*cat_err)( int road );
	void (*work)(void);
}fuse_t;

extern fuse_t* fuse;

#endif

