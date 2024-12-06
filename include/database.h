/*
Copyright © GamerNoTitle 2024. All rights reserved.
File name: database.h
Author: 吴沛熹      ID: GamerNoTitle    Version: v1.0   Date: 2024/12/4
Description:    本文件用于声明预设的一些sql操作函数，包括成绩自定范围查询，学生的添加、修改、删除
                考试的查看、添加、删除，个人密码的修改等
Others:         暂无
History:        暂无
    1.  Date: 2024/12/6
        Author: 吴沛熹
        ID: GamerNoTitle
        Modification:   [+] 新建了本文件
                        [+] 进行了 query_user_info 函数的声明
 */

#include <stdio.h>
#include "model.h"

int query_user_info(const char key[], const char content[], struct User *user_to_return, FILE *log_file);