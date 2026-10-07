

/*-------------------------------------------------------------------------*/

typedef struct{
	int year;			//年
	int month;		//月
	int day;			//日
	int hour;			//时
	int minute;		//分
	int second; 	//秒
}utc_t; 

typedef struct{
	
	// 将 64 位时间戳（秒）转换为 UTC 时间
	void (*timestamp_to_utc)(uint64_t timestamp, utc_t *utc);
	
	// 将 UTC 时间转换为 64 位时间戳（秒）
	uint64_t (*utc_to_timestamp)(utc_t *utc);
	
	// 打印 UTC 时间（格式：YYYY-MM-DD HH:MM:SS）
	void (*print_utc_time)(utc_t *utc);
	
	
}utc_opt_t;
/*-------------------------------------------------------------------------*/

extern utc_opt_t* utc_opt;

/*-------------------------------------------------------------------------*/


