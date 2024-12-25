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
    2.  Date: 2024/12/18
        Author: 吴沛熹
        ID: GamerNoTitle
        Modification:   [+] 定义了问题列表相关函数 `generate_question_list`、`randomize_question_list`
    3.  Date: 2024/12/21
        Author: 吴沛熹
        ID: GamerNoTitle
        Modification:   [*] 完成了问题列表相关操作函数
 */

#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "../include/database.h"
#include "../include/model.h"
#include "../include/utils.h"

/*** 日志等级 ***/
#define LOGLEVEL_ERROR "ERROR" // 错误级别日志
#define LOGLEVEL_INFO "INFO"   // 信息级别日志

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
    log_message(LOGLEVEL_INFO, "计算结果: num1=%d, num2=%d, op=%d", num1, num2, op);
    float result;
    switch (op)
    {
    case 0:
        result = num1 + num2;
        log_message(LOGLEVEL_INFO, "执行加法: %d + %d = %.2f", num1, num2, result);
        return result;
    case 1:
        result = num1 - num2;
        log_message(LOGLEVEL_INFO, "执行减法: %d - %d = %.2f", num1, num2, result);
        return result;
    case 2:
        result = num1 * num2;
        log_message(LOGLEVEL_INFO, "执行乘法: %d * %d = %.2f", num1, num2, result);
        return result;
    case 3:
        if (num2 == 0)
        {
            log_message(LOGLEVEL_ERROR, "除以零错误: num1=%d, num2=%d", num1, num2);
            return 0.0f; // 或者根据需求处理错误
        }
        result = (float)num1 / (float)num2;
        log_message(LOGLEVEL_INFO, "执行除法: %d / %d = %.2f", num1, num2, result);
        return result;
    default:
        log_message(LOGLEVEL_ERROR, "未知的运算符: %d", op);
        return 0.0f;
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
    log_message(LOGLEVEL_INFO, "判断结果: 计算结果=%.2f, 用户输入=%.2f", result, user_input);
    // 将结果限定在小数点后两位
    float rounded_result = round(result * 100) / 100;
    float rounded_input = round(user_input * 100) / 100;

    if (fabs(rounded_result - rounded_input) < 0.01f)
    {
        log_message(LOGLEVEL_INFO, "用户答案正确");
        return 1; // 正确返回1
    }
    else
    {
        log_message(LOGLEVEL_INFO, "用户答案错误");
        return 0; // 不正确返回0
    }
}

/**************************** 问题模型部分结束 ****************************/

/**************************** 考试问题生成模块部分开始 ****************************/

/**
 * @brief 生成问题链表
 *
 * @param exam_id 需要生成问题链表的考试id，会根据此ID从题库获取题目
 * @param question_list_to_return 需要返回的问题链表的头节点
 * @param count 需要生成的问题数量
 * @return int 成功返回 0，否则返回 1
 */
int generate_question_list(const char *exam_id, struct Question *question_list_to_return, int count)
{
    void free_question_list(struct Question *head_ptr);
    log_message(LOGLEVEL_INFO, "生成问题链表: exam_id=%s, count=%d", exam_id, count);

    if (exam_id == NULL || question_list_to_return == NULL)
    {
        log_message(LOGLEVEL_ERROR, "参数 exam_id 或 question_list_to_return 为 NULL");
        return 1;
    }

    // 分配内存以存储从数据库查询到的问题
    struct SqlResponseQuestion *sql_questions = (struct SqlResponseQuestion *)malloc(sizeof(struct SqlResponseQuestion) * count);
    if (sql_questions == NULL)
    {
        log_message(LOGLEVEL_ERROR, "内存分配失败");
        return 1;
    }

    // 调用 query_questions_info_all 函数从数据库中获取指定考试id的所有题目
    if (query_questions_info_all(sql_questions, count, "exam_id", exam_id) != 0)
    {
        log_message(LOGLEVEL_ERROR, "查询考试问题失败: exam_id=%s", exam_id);
        free(sql_questions);
        return 1;
    }

    // 初始化随机数生成器
    srand((unsigned int)time(NULL));
    log_message(LOGLEVEL_INFO, "随机数生成器已初始化");

    // 遍历查询结果，构建问题链表
    int i = 0;
    struct Question *current = question_list_to_return;
    memset(current, 0, sizeof(struct Question)); // 初始化头节点

    for (i = 0; i < count && sql_questions[i].id[0] != '\0'; i++)
    {
        // 填充当前节点的数据
        current->data.num1 = sql_questions[i].num1;
        current->data.op = sql_questions[i].op;
        current->data.num2 = sql_questions[i].num2;

        log_message(LOGLEVEL_INFO, "添加问题: num1=%d, op=%d, num2=%d", current->data.num1, current->data.op, current->data.num2);

        // 如果不是最后一个问题，创建下一个节点
        if (i < count - 1 && sql_questions[i + 1].id[0] != '\0')
        {
            current->next_question = (struct Question *)malloc(sizeof(struct Question));
            if (current->next_question == NULL)
            {
                log_message(LOGLEVEL_ERROR, "内存分配失败在第 %d 个问题", i + 1);
                free_question_list(question_list_to_return);
                free(sql_questions);
                return 1;
            }
            memset(current->next_question, 0, sizeof(struct Question)); // 初始化新节点
            current = current->next_question;
        }
    }

    free(sql_questions);
    log_message(LOGLEVEL_INFO, "问题链表生成成功: 实际生成的问题数量=%d", i);
    return 0;
}

/**
 * @brief 将问题链表随机顺序化
 *
 * @param question_list_to_return 返回随机化后问题链表的头节点
 * @param original_question_list 原来的问题链表
 * @return int 成功返回 0，否则返回 1
 */
int randomize_question_list(struct Question *question_list_to_return, struct Question *original_question_list)
{
    void free_question_list(struct Question *head_ptr);
    log_message(LOGLEVEL_INFO, "开始随机化问题链表");

    if (original_question_list == NULL || question_list_to_return == NULL)
    {
        log_message(LOGLEVEL_ERROR, "参数 original_question_list 或 question_list_to_return 为 NULL");
        return 1;
    }

    // 遍历原始链表，计数并存储节点指针
    int count = 0;
    struct Question *current = original_question_list;
    while (current != NULL)
    {
        count++;
        current = current->next_question;
    }

    log_message(LOGLEVEL_INFO, "原始问题数量=%d", count);

    if (count == 0)
    {
        log_message(LOGLEVEL_INFO, "原始问题链表为空，无法随机化");
        return 0;
    }

    // 创建一个数组来存储指向问题节点的指针
    struct Question **question_array = (struct Question **)malloc(sizeof(struct Question *) * count);
    if (question_array == NULL)
    {
        log_message(LOGLEVEL_ERROR, "内存分配失败");
        return 1;
    }

    // 重新遍历原始链表，将指针存储到数组中
    current = original_question_list;
    for (int i = 0; i < count; i++)
    {
        question_array[i] = current;
        current = current->next_question;
    }

    // 使用 Fisher-Yates 算法打乱数组
    for (int i = count - 1; i > 0; i--)
    {
        int j = rand() % (i + 1);
        // 交换 question_array[i] 和 question_array[j]
        struct Question *temp = question_array[i];
        question_array[i] = question_array[j];
        question_array[j] = temp;
    }

    log_message(LOGLEVEL_INFO, "问题数组已打乱顺序");

    // 构建新的随机顺序链表
    current = question_list_to_return;
    memset(current, 0, sizeof(struct Question)); // 初始化头节点

    // 复制第一个问题的数据
    current->data = question_array[0]->data;
    log_message(LOGLEVEL_INFO, "随机化后问题 1: num1=%d, op=%d, num2=%d", current->data.num1, current->data.op, current->data.num2);

    // 依次复制剩余的问题
    for (int i = 1; i < count; i++)
    {
        current->next_question = (struct Question *)malloc(sizeof(struct Question));
        if (current->next_question == NULL)
        {
            log_message(LOGLEVEL_ERROR, "内存分配失败在随机化的第 %d 个问题", i + 1);
            free_question_list(question_list_to_return);
            free(question_array);
            return 1;
        }
        current = current->next_question;
        memset(current, 0, sizeof(struct Question)); // 初始化新节点
        current->data = question_array[i]->data;
        log_message(LOGLEVEL_INFO, "随机化后问题 %d: num1=%d, op=%d, num2=%d", i + 1, current->data.num1, current->data.op, current->data.num2);
    }

    // 释放临时数组
    free(question_array);
    log_message(LOGLEVEL_INFO, "问题链表随机化完成");
    return 0;
}

/**
 * @brief 释放问题链表的内存
 *
 * @param head_ptr 链表头指针
 */
void free_question_list(struct Question *head_ptr)
{
    log_message(LOGLEVEL_INFO, "开始释放问题链表内存");
    struct Question *current = head_ptr;
    while (current != NULL)
    {
        struct Question *next = current->next_question;
        free(current);
        current = next;
    }
    log_message(LOGLEVEL_INFO, "问题链表内存已释放");
}

/**************************** 考试问题生成模块部分结束 ****************************/

/**
 * @brief 打印问题链表
 *
 * @param head 链表头指针
 */
void print_question_list(struct Question *head)
{
    if (head == NULL)
    {
        log_message(LOGLEVEL_INFO, "打印问题链表: 链表为空");
        return;
    }

    log_message(LOGLEVEL_INFO, "开始打印问题链表");
    struct Question *current = head;
    int index = 1;
    while (current != NULL)
    {
        char op_char;
        switch (current->data.op)
        {
        case 0:
            op_char = '+';
            break;
        case 1:
            op_char = '-';
            break;
        case 2:
            op_char = '*';
            break;
        case 3:
            op_char = '/';
            break;
        default:
            op_char = '?';
            break;
        }
        log_message(LOGLEVEL_INFO, "问题 %d: %d %c %d", index, current->data.num1, op_char, current->data.num2);
        current = current->next_question;
        index++;
    }
    log_message(LOGLEVEL_INFO, "问题链表打印完成");
}
