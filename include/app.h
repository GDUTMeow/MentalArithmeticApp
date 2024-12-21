/*
Copyright © GamerNoTitle 2024. All rights reserved.
File name: app.h
Author: 吴沛熹      ID: GamerNoTitle    Version: v1.0   Date: 2024/12/4
Description:    本文件在app运行中编写的各种自定义函数的声明
Others:         暂无
History:        暂无
    1.  Date: 2024/12/4
        Author: 吴沛熹
        ID: GamerNoTitle
        Modification:   [+] 新建了本文件，并把model.h中有关APP运行的自定义函数声明挪入本文件
    1.  Date: 2024/12/21
        Author: 吴沛熹
        ID: GamerNoTitle
        Modification:   [+] 添加了头文件包含保护
                        [+] 添加了一些函数的声明
 */

#ifndef APP_H
#define APP_H

#include "database.h"
#include "model.h"
#include "utils.h"


/**************************** 问题模型部分 ****************************/

float calculate_result(int num1, int num2, int op);
int judge(float result, float user_input);
/**************************** 问题模型部分结束 ****************************/

/**************************** 考试问题生成模块部分开始 ****************************/

int generate_question_list(const char *exam_id, struct Question *question_list_to_return);
int randomize_question_list(struct Question *question_list_to_return, struct Question *original_question_list);
void free_question_list(struct Question *head_ptr);
/**************************** 考试问题生成模块部分结束 ****************************/

#endif