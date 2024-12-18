/*
Copyright © GamerNoTitle 2024. All rights reserved.
File name: app.c
Author: 吴沛熹      ID: GamerNoTitle    Version: v1.0   Date: 2024/12/4
Description:    本文件在app运行中编写的各种自定义函数
Others:         暂无
History:        暂无
    1.  Date: 2024/12/4
        Author: 吴沛熹
        ID: GamerNoTitle
        Modification:   [+] 新建了本文件，并把model.c中有关APP运行的自定义函数挪入本文件
    1.  Date: 2024/12/18
        Author: 吴沛熹
        ID: GamerNoTitle
        Modification:   [+] 定义了问题列表相关函数 `generate_question_list`、`randomize_question_list`
 */

#include <math.h>
#include <stdlib.h>

/**************************** 问题模型部分 ****************************/
/**
 * @brief 计算式子的正确答案，并返回答案
 *
 * @param num1 第一个操作数
 * @param num2 第二个操作数
 * @param op   运算符标识，0123分别对应+-×÷
 * @return float 正确答案
 */
float calculate_result(int num1, int num2, int op)
{
    switch (op)
    {
    case 0:
        return num1 + num2;
    case 1:
        return num1 - num2;
    case 2:
        return num1 * num2;
    case 3:
        return (float)num1 / (float)num2;
    }
}

/**
 * @brief 判断用户输入与计算机计算的结果是否一致
 *
 * @param result 计算机计算的结果
 * @param user_input 用户输入的结果
 * @return int  正确返回1，否则返回0
 */
int judge(float result, float user_input)
{
    if (round(result * 100 + 0.5 / 100) == round(user_input * 100 + 0.5 / 100)) // 把结果限定在小数点后两位（如果有的话，就不需要考虑1E-6的问题了）
    {
        return 1; // 正确返回1
    }
    else
    {
        return 0; // 不正确返回0
    }
}

/**************************** 问题模型部分结束 ****************************/

/**************************** 考试问题生成模块部分开始 ****************************/

/**
 * @brief 生成问题链表
 * 
 * @param exam_id 需要生成问题链表的考试id，会根据此ID从题库获取题目
 * @param question_list_to_return 需要返回的问题数组
 * @return int 成功返回 0，否则返回 1
 */
int generate_question_list(const char *exam_id, struct Question *question_list_to_return)
{
    ;
}

/**
 * @brief 将问题链表随机顺序化
 * 
 * @param question_list_to_return 返回随机化后问题链表的变量
 * @param original_question_list 原来的问题链表
 * @return int 成功返回 0，否则返回 1
 */
int randomize_question_list(struct Question *question_list_to_return, struct Question *original_question_list)
{
    ;
}

/**************************** 考试问题生成模块部分结束 ****************************/