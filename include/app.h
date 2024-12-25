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
    2.  Date: 2024/12/21
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

/**
 * @brief 计算式子的正确答案，并返回结果。
 *
 * 根据运算符标识执行加法、减法、乘法或除法运算。
 *
 * @param num1 第一个操作数
 * @param num2 第二个操作数
 * @param op 运算符标识，0: +, 1: -, 2: *, 3: /
 * @return float 计算结果
 */
float calculate_result(int num1, int num2, int op);

/**
 * @brief 判断用户输入的结果是否与计算结果一致。
 *
 * 比较计算结果与用户输入的结果，保留两位小数进行比较。
 *
 * @param result 计算机计算的结果
 * @param user_input 用户输入的结果
 * @return int 如果一致返回1，否则返回0
 */
int judge(float result, float user_input);

/**************************** 问题模型部分结束 ****************************/

/**************************** 考试问题生成模块部分开始 ****************************/

/**
 * @brief 生成指定考试ID的题目链表。
 *
 * 从数据库中获取指定考试ID的题目，并构建一个链表形式的问题列表。
 *
 * @param exam_id 需要生成问题链表的考试ID
 * @param question_list_to_return 需要返回的问题链表的头节点
 * @param count 需要生成的问题数量
 * @return int 成功返回0，失败返回1
 */
int generate_question_list(const char *exam_id, struct Question *question_list_to_return, int count);

/**
 * @brief 将原始问题链表随机化顺序，并返回新的问题链表。
 *
 * 使用Fisher-Yates算法打乱问题链表的顺序。
 *
 * @param question_list_to_return 返回随机化后问题链表的头节点
 * @param original_question_list 原始的问题链表
 * @return int 成功返回0，失败返回1
 */
int randomize_question_list(struct Question *question_list_to_return, struct Question *original_question_list);

/**
 * @brief 释放问题链表占用的内存。
 *
 * 遍历链表并释放每个节点的内存。
 *
 * @param head_ptr 链表头指针
 */
void free_question_list(struct Question *head_ptr);

/**************************** 考试问题生成模块部分结束 ****************************/

#endif
