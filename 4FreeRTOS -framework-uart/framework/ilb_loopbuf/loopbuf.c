#include "framework.h"
// 引入标准库头文件，stdlib.h提供malloc、free内存管理函数
#include <stdlib.h>

/*
 * 库函数科普：
 * void* malloc(uint32_t size);
 * 功能：向操作系统申请一块size字节大小的堆内存，成功返回内存起始地址；失败返回0(NULL)
 * void free(void* ptr);
 * 功能：释放之前malloc申请出来的堆内存，归还操作系统，防止内存泄漏
 *
 * 下面封装 my_malloc、my_free、my_memcpy
 * 好处：以后如果换到单片机、Linux、RTOS等别的平台，只需要修改这三个函数，
 * 后面所有环形缓冲代码不用改动，实现平台兼容
 */

// 内存分配函数替代包装
void* my_malloc(uint32_t size) {
    // 参数 size：需要申请多少字节的内存
    // 此处应实现平台无关的内存分配
    // 示例实现，实际使用时需替换为具体平台的实现
    // return (void*)0x1000; // 模拟返回内存地址，测试用假地址，正式不用

    // 调用系统malloc申请内存，申请成功返回内存地址
    return mem_alloc( size );
}

// 内存释放函数替代包装
void my_free(void* ptr) {
    // ptr：之前my_malloc拿到的内存起始地址
    // 此处应实现平台无关的内存释放
    mem_free( ptr ); // 调用系统free释放堆内存
}

/*
 * 功能：手动实现内存拷贝，代替系统memcpy
 * 参数解释
 * dest：目标地址（复制之后存放数据的地方）
 * src：源地址（需要被复制的原始数据）
 * n：一共拷贝多少个字节
 * uint8_t 就是1字节，缓冲区存储字节数据
 */
void my_memcpy(uint8_t* dest, const uint8_t* src, uint32_t n) {
    uint32_t i; // i循环计数器，从0开始遍历每一字节
    // 循环n次，逐个字节搬运数据
    for (i = 0; i < n; i++) {
        dest[i] = src[i]; // 将src第i个字节，赋值给dest第i个字节
    }
}


/**
 * @brief  创建并且初始化一个环形缓冲区
 * @param  size：你想要环形缓冲区多大(字节)
 * @retval 返回新建好的环形缓冲区指针；内存申请失败返回0(NULL)
 */
loopbuf_t* loopbuf_init(uint32_t size) 
{
    // 第一步：申请环形缓冲区结构体本身的内存
    // sizeof(loopbuf_t)：计算loopbuf_t这个结构体占多少字节
    loopbuf_t* lb = (loopbuf_t*)my_malloc(sizeof(loopbuf_t));

    // 判断内存申请失败，malloc失败返回0
    if (!lb) return 0;

    // 第二步：给缓冲区的数据数组申请size字节内存，用来存字节数据
    lb->buffer = (uint8_t*)my_malloc(size);

    // 如果存放数据的缓存内存申请失败
    if (!lb->buffer) {
        my_free(lb); // 已经申请成功的结构体必须释放，不然造成内存泄漏
        return 0; // 返回空指针，初始化失败
    }

    lb->size = size;   // 设置缓冲区最大容量
    lb->head = 0;      // 写入下标从头(数组下标0)开始
    lb->tail = 0;      // 读取下标从头(数组下标0)开始
    return lb;          // 返回初始化完成的环形缓冲指针
}

/**
 * @brief 销毁、释放环形缓冲区所有内存，防止内存泄漏
 * @param  lb：环形缓冲区指针
 */
void loopbuf_free(loopbuf_t* lb) {
    // 做安全判断，如果lb不是空指针才往下执行
    if (lb) {
        // 如果数据缓存指针不为空，先释放存放字节的buffer内存
        if (lb->buffer) 
        my_free(lb->buffer);
        my_free(lb); // 再释放结构体本身占用的内存
    }
}

/**
 * @brief 往环形缓冲区写入数据
 * @param  lb:环形缓冲指针
 * @param  data:你要写入的数据起始地址
 * @param  n:打算写入多少字节
 * @retval 最终成功写入了多少字节(空间不足时会只写入一部分)
 */
uint32_t loopbuf_write(loopbuf_t* lb, const uint8_t* data, uint32_t n) {
    // 安全校验：缓冲区指针为空 / 待写入数据是空指针 / 写入长度等于0 → 直接返回写入0字节
    if (!lb || !data || n == 0) return 0;

    /*
     * 计算现在缓冲区还有多少空闲空间可以写入数据
     * head >= tail：写指针在读指针后面，空闲处在head到缓冲区末尾
     * head < tail：写指针已经绕回开头，空闲处在head~末尾 和 开头~tail
     */
    uint32_t space = lb->size - ((lb->head >= lb->tail)? (lb->head - lb->tail) : (lb->size - (lb->tail - lb->head)));

    // 如果用户想要写入字节 > 剩余空闲空间；那就只能写入空闲空间大小的数据
    if (n > space) n = space;

    // 空闲为0，缓冲区满了，直接返回写入0字节
    if (n == 0) return 0;

    // part1：head位置到缓冲区数组末尾还剩下多少字节
    uint32_t part1 = lb->size - lb->head;

    // 情况1：剩下尾部空间足够放下所有待写入数据，不需要绕回数组头部
    if (n <= part1) {
        // buffer + head：从写入下标位置开始拷贝n字节数据
        my_memcpy(lb->buffer + lb->head, data, n);
        // 更新写入指针，向后移动n个字节；取模size到达末尾之后归零循环
        lb->head = (lb->head + n) % lb->size;
    } else {
        // 情况2：尾部剩下的空间装不下全部数据，需要分两段写入
        // 第一段：把head一直写到缓冲区数组最末尾
        my_memcpy(lb->buffer + lb->head, data, part1);
        // 第二段：剩下的数据从数组下标0开始接着存放
        my_memcpy(lb->buffer, data + part1, n - part1);
        // head现在定位到剩下数据长度的位置（已经绕回缓冲区开头）
        lb->head = n - part1;
    }

    return n; // 返回本次成功写入的数据字节数目
}

/**
 * @brief 从环形缓冲区读出数据
 * @param  lb:环形缓冲区指针
 * @param  data:读取出来之后存放数据的地址
 * @param  n:打算读取多少字节
 * @retval 实际成功读取到多少字节
 */
uint32_t loopbuf_read(loopbuf_t* lb, uint8_t* data, uint32_t n) {
    // 安全校验：缓冲区空、接收缓存空、读取长度0 → 返回读取0字节
    if (!lb || !data || n == 0) return 0;

    // 计算现在缓冲区里面存放着多少可读的数据
    uint32_t available = (lb->head >= lb->tail)? (lb->head - lb->tail) : (lb->size - (lb->tail - lb->head));

    // 可读数据小于用户想读的数量，则只读出现存所有数据
    if (n > available) n = available;

    // 没有可读的数据，直接返回0
    if (n == 0) return 0;

    // part1：tail读指针距离缓冲区数组末尾剩余字节
    uint32_t part1 = lb->size - lb->tail;

    // 情况一：尾部剩余空间足够读完所有数据，不需要环绕
    if (n <= part1) {
        // 从tail位置拷贝n字节到data接收区
        my_memcpy(data, lb->buffer + lb->tail, n);
        // 读指针往后偏移n，取模循环
        lb->tail = (lb->tail + n) % lb->size;
    } else {
        // 情况二：需要分两段读取，先读到缓存末尾，再从头接着读剩下的数据
        my_memcpy(data, lb->buffer + lb->tail, part1);
        my_memcpy(data + part1, lb->buffer, n - part1);
        // tail 跳到缓冲区开头剩下数据下标
        lb->tail = n - part1;
    }

    return n; // 返回成功读取字节数
}

void loopbuf_reset(loopbuf_t* lb)
{
    lb->head = 0;
    lb->tail = 0;
    lb->size = 0;
    
}
