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

void get_current_time(char *buffer, size_t size);
void log_message(const char *level, const char *format, ...);