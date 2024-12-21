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
#define LOGLEVEL_ERROR "ERROR"
#define LOGLEVEL_INFO "INFO"

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

// 实现 generate_question_list 函数
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
    if (exam_id == NULL || question_list_to_return == NULL)
    {
        log_message(LOGLEVEL_ERROR, "参数 exam_id 或 question_list_to_return 为 NULL");
        return 1;
    }

    // 定义保存问题的结构体列表
    struct SqlResponseQuestion *sql_questions = (struct SqlResponseQuestion *)malloc(sizeof(struct SqlResponseQuestion) * count);
    if (sql_questions == NULL)
    {
        log_message(LOGLEVEL_ERROR, "内存分配失败");
        return 1;
    }

    // 调用 query_questions_info_all 函数从数据库中获取指定考试id的所有题目
    if (query_questions_info_all(sql_questions, count, "exam_id", exam_id) != 0)
    {
        log_message(LOGLEVEL_ERROR, "查询考试问题失败");
        free(sql_questions);
        return 1;
    }

    // 初始化随机数生成器
    srand(time(NULL));

    // 遍历查询结果，构建问题链表
    int i = 0;
    struct Question *current = question_list_to_return;
    memset(current, 0, sizeof(struct Question)); // 初始化头节点

    for (i = 0; i < count && sql_questions[i].id[0] != '\0'; i++)
    {
        // 填充当前节点的数据
        current->data.num1 = (float)sql_questions[i].num1;
        current->data.op = sql_questions[i].op;
        current->data.num2 = (float)sql_questions[i].num2;

        // 如果不是最后一个问题，创建下一个节点
        if (i < count - 1 && sql_questions[i + 1].id[0] != '\0')
        {
            current->next_question = (struct Question *)malloc(sizeof(struct Question));
            if (current->next_question == NULL)
            {
                log_message(LOGLEVEL_ERROR, "内存分配失败");
                free(sql_questions);
                return 1;
            }
            memset(current->next_question, 0, sizeof(struct Question)); // 初始化新节点
            current = current->next_question;
        }
    }

    free(sql_questions);
    return 0;
}

// 实现 randomize_question_list 函数
/**
 * @brief 将问题链表随机顺序化
 *
 * @param question_list_to_return 返回随机化后问题链表的头节点
 * @param original_question_list 原来的问题链表
 * @return int 成功返回 0，否则返回 1
 */
int randomize_question_list(struct Question *question_list_to_return, struct Question *original_question_list)
{
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

    if (count == 0)
    {
        log_message(LOGLEVEL_INFO, "原始问题链表为空");
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

    // 使用 Fisher-Yates 算法打乱数组（无偏向等值随机化）
    for (int i = count - 1; i > 0; i--)
    {
        int j = rand() % (i + 1);
        // 交换 question_array[i] 和 question_array[j]
        struct Question *temp = question_array[i];
        question_array[i] = question_array[j];
        question_array[j] = temp;
    }

    // 构建新的随机顺序链表
    current = question_list_to_return;
    memset(current, 0, sizeof(struct Question)); // 初始化头节点

    // 复制第一个问题的数据
    current->data = question_array[0]->data;

    // 依次复制剩余的问题
    for (int i = 1; i < count; i++)
    {
        current->next_question = (struct Question *)malloc(sizeof(struct Question));
        if (current->next_question == NULL)
        {
            log_message(LOGLEVEL_ERROR, "内存分配失败");
            // 需要释放已分配的节点
            struct Question *temp = question_list_to_return->next_question;
            while (temp != NULL)
            {
                struct Question *next = temp->next_question;
                free(temp);
                temp = next;
            }
            free(question_array);
            return 1;
        }
        current = current->next_question;
        memset(current, 0, sizeof(struct Question)); // 初始化新节点
        current->data = question_array[i]->data;
    }

    // 释放临时数组
    free(question_array);

    return 0;
}

/**
 * @brief 释放问题链表的内存
 *
 * @param head_ptr 链表头指针
 */
void free_question_list(struct Question *head_ptr)
{
    struct Question *current = head_ptr;
    while (current != NULL)
    {
        struct Question *next = current->next_question;
        free(current);
        current = next;
    }
}

/**************************** 考试问题生成模块部分结束 ****************************/

/**
 * @brief 打印问题链表
 *
 * @param head 链表头指针
 */
void print_question_list(struct Question *head)
{
    struct Question *current = head;
    int index = 1;
    while (current != NULL)
    {
        printf("问题 %d: %.2f ", index, current->data.num1);
        switch (current->data.op)
        {
        case 0:
            printf("+ ");
            break;
        case 1:
            printf("- ");
            break;
        case 2:
            printf("* ");
            break;
        case 3:
            printf("/ ");
            break;
        default:
            printf("? ");
            break;
        }
        printf("%.2f %d\n", current->data.num2, current->data.op);
        current = current->next_question;
        index++;
    }
}

int main()
{
    const char *exam_id = "0fcec351-7bd9-4b8a-a7f3-84bdc7150324"; // 示例考试ID

    // 分配内存给 question_list_to_return
    struct Question *question_list = (struct Question *)malloc(sizeof(struct Question));
    if (question_list == NULL)
    {
        log_message(LOGLEVEL_ERROR, "内存分配失败");
        return 1;
    }
    memset(question_list, 0, sizeof(struct Question)); // 初始化头节点

    // 调用 generate_question_list 函数
    if (generate_question_list(exam_id, question_list, 4) != 0)
    {
        log_message(LOGLEVEL_ERROR, "生成问题链表失败");
        free(question_list);
        return 1;
    }

    printf("=== 原始问题链表 ===\n");
    print_question_list(question_list);

    // 分配内存给 randomized_list
    struct Question *randomized_list = (struct Question *)malloc(sizeof(struct Question));
    if (randomized_list == NULL)
    {
        log_message(LOGLEVEL_ERROR, "内存分配失败");
        free_question_list(question_list);
        return 1;
    }
    memset(randomized_list, 0, sizeof(struct Question)); // 初始化头节点

    // 调用 randomize_question_list 函数
    if (randomize_question_list(randomized_list, question_list) != 0)
    {
        log_message(LOGLEVEL_ERROR, "随机化问题链表失败");
        free_question_list(question_list);
        free(randomized_list);
        return 1;
    }

    printf("\n=== 随机化后的问题链表 ===\n");
    print_question_list(randomized_list);

    // 释放内存
    free_question_list(question_list);
    free_question_list(randomized_list);

    return 0;
}