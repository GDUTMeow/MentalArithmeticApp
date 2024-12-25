/*
Copyright © GamerNoTitle 2024. All rights reserved.
File name: utils.c
Author: 吴沛熹      ID: GamerNoTitle    Version: v1.0   Date:
Description:    本文件保存了一些自己写好的函数，方便使用
Others:         暂无
History:        暂无
    1.  Date: 2024/12/4
        Author: 吴沛熹
        ID: GamerNoTitle
        Modification: [+] 新建了文件，将 utils.h 中的函数挪入本文件
                      [+] 加入了 get_current_time 函数，用于获取当前时间并保存为 YYYY-MM-DD HH:mm:SS 的格式
    2.  Date: 2024/12/8
        Author: 吴沛熹
        ID: GamerNoTitle
        Modification: [+] 添加了日志记录函数，避免重复造轮子
 */

#include <stdarg.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <time.h>       // 引入 time.h 以支持时间函数
#include "utils.h"

/*** 日志等级 ***/
#define LOGLEVEL_ERROR "ERROR"  // 定义错误级别日志
#define LOGLEVEL_INFO "INFO"    // 定义信息级别日志

/*** 日志部分 ***/
#define LOG_FOLDER "logs"          // 日志文件夹路径
#define LOG_FILE "logs/latest.log" // 日志文件路径

/**
 * @brief 获取当前时间，格式化为 "YYYY-MM-DD HH:mm:SS"
 * 
 * @param buffer 存储格式化后的时间字符串的缓冲区。函数会将当前时间按照指定格式存储到这个字符数组中。
 * @param size   buffer 数组的大小，用于确保不发生溢出
 */
void get_current_time(char *buffer, size_t size)
{
    time_t t = time(NULL); // 获取当前时间戳
    struct tm tm_info;
    
    // 将当前时间转换为当地时间
    #ifdef _WIN32
        localtime_s(&tm_info, &t); // Windows 系统使用 localtime_s
    #else
        localtime_r(&t, &tm_info); // POSIX 系统使用 localtime_r
    #endif

    // 将时间格式化为 "YYYY-MM-DD HH:mm:SS" 格式并存储到 buffer 中
    strftime(buffer, size, "%Y-%m-%d %H:%M:%S", &tm_info);  // 按照 "YYYY-MM-DD HH:mm:SS" 的格式保存到 buffer
}

/**
 * @brief 记录日志信息到日志文件
 *
 * @param level 日志级别（例如：LOGLEVEL_ERROR, LOGLEVEL_INFO）
 * @param format 格式化字符串，类似 printf
 * @param ... 变长参数
 */
void log_message(const char *level, const char *format, ...) {
    // 打开日志文件以追加模式写入
    FILE *log_file = fopen(LOG_FILE, "a");
    if (log_file == NULL) { // 检查文件是否成功打开
        printf("无法打开日志文件：%s\n", strerror(errno)); // 打印错误信息到标准输出
        return;
    }

    char current_time[20];
    get_current_time(current_time, sizeof(current_time)); // 获取当前时间字符串

    // 写入时间和日志级别到日志文件
    fprintf(log_file, "%s [%s]: ", current_time, level);

    // 处理可变参数并写入格式化的日志消息
    va_list args;
    va_start(args, format);
    vfprintf(log_file, format, args); // 根据格式化字符串写入日志内容
    va_end(args);

    fprintf(log_file, "\n"); // 添加换行符
    fclose(log_file); // 关闭日志文件
}
