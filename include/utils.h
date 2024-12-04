/*
Copyright © GamerNoTitle 2024. All rights reserved.
File name: utils.h
Author: 吴沛熹      ID: GamerNoTitle    Version: v1.0   Date:
Description:    本文件保存了一些自己写好的函数，方便使用
Others:         暂无
History:        暂无
    1.  Date: 2024/12/4
        Author: 吴沛熹
        ID: GamerNoTitle
        Modification: [+] 新建了文件，并加入了头部文件注释，说明本头文件的功能
                      [+] 加入了 get_current_time 函数，用于获取当前时间并保存为 YYYY-MM-DD HH:mm:SS 的格式
 */

#include <time.h>  // 用于获取当前时间

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
    localtime_s(&tm_info, &t);

    // 将时间格式化为 "YYYY-MM-DD HH:mm:SS" 格式并存储到 buffer 中
    strftime(buffer, size, "%Y-%m-%d %H:%M:%S", &tm_info);  // 按照 "YYYY-MM-DD HH:mm:SS" 的格式保存到buffer，返回到传入的字符串
}
