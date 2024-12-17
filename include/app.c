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
 */

#include <math.h>

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

/**************************** 用户操作部分开始 ****************************/

/**
 * @brief 用户重置密码调用的函数
 * 
 * @param username 用户的用户名，必须根据此进行索引
 * @param hashpass 用户新密码的哈希值
 * @param salt 用户新密码的盐
 * @return int 成功返回0，失败返回1
 */
int reset_password(const char *username, const char *hashpass, const char *salt)
{
    ;
}

/**
 * @brief 用户登录函数
 * 
 * @param username 用户的用户名
 * @param hashpass 用户输入的密码经过哈希后的值
 * @return int 成功返回0，失败返回1
 */
int do_login_verify(const char *username, const char *hashpass)
{
    ;
}

/**************************** 用户操作部分结束 ****************************/