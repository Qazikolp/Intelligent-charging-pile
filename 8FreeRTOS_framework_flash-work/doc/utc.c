
#include "framework.h"

#define PRT_LOG(X...)	//mysysprt(X)



// 判断是否为闰年
static int is_leap_year(int year) {
    if ((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0)) {
        return 1; // 是闰年
    }
    return 0; // 不是闰年
}

// 获取指定月份的天数
static int get_days_in_month(int year, int month) {
    const int days_in_month[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

    if (month == 2 && is_leap_year(year)) {
        return 29; // 闰年二月
    }
    return days_in_month[month - 1];
}


// 将 64 位时间戳（秒）转换为 UTC 时间
static void timestamp_to_utc(uint64_t timestamp, utc_time_t *utc) {
    uint64_t days = timestamp / 86400; // 1天 = 86400秒
    uint64_t seconds_in_day = timestamp % 86400; // 该天的秒数

    int year = 1970;
    while (days >= (is_leap_year(year) ? 366 : 365)) {
        days -= (is_leap_year(year) ? 366 : 365);
        year++;
    }

    int month = 1;
    while (days >= get_days_in_month(year, month)) {
        days -= get_days_in_month(year, month);
        month++;
    }

    int day = days + 1;
    
    int hour = seconds_in_day / 3600;
    int minute = (seconds_in_day % 3600) / 60;
    int second = seconds_in_day % 60;

    utc->year = year;
    utc->month = month;
    utc->day = day;
    utc->hour = hour;
    utc->minute = minute;
    utc->second = second;
}

// 将 UTC 时间转换为 64 位时间戳（秒）
static uint64_t utc_to_timestamp(utc_time_t *utc) {
    uint64_t timestamp = 0;

    int year = 1970;
    while (year < utc->year) {
        timestamp += (is_leap_year(year) ? 366 : 365);
        year++;
    }

    for (int month = 1; month < utc->month; month++) {
        timestamp += get_days_in_month(utc->year, month);
    }

    timestamp += utc->day - 1;

    uint64_t seconds_in_day = (utc->hour * 3600) + (utc->minute * 60) + utc->second;
    timestamp = timestamp * 86400 + seconds_in_day;

    return timestamp;
}

// 打印 UTC 时间（格式：YYYY-MM-DD HH:MM:SS）
static void print_utc_time(utc_time_t *utc) 
{
    PRT_LOG("%04d-%02d-%02d %02d:%02d:%02d\r\n", 
		utc->year, utc->month, utc->day, utc->hour, utc->minute, utc->second);
}


static utc_opt_t do_utc_opt={
	.timestamp_to_utc = timestamp_to_utc,
	.utc_to_timestamp = utc_to_timestamp,
	.print_utc_time = print_utc_time,
	
};

utc_opt_t* utc_opt = &do_utc_opt;


