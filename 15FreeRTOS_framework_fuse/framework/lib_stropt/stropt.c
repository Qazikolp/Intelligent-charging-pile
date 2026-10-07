#include "framework.h"


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


static stropt_t do_stropt={
	.split_string_n = split_string_n,

};

stropt_t* stropt = &do_stropt;
