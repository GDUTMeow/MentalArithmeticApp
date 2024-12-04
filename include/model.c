/*
Copyright © GamerNoTitle 2024. All rights reserved.
File name: model.c
Author: 吴沛熹      ID: GamerNoTitle    Version: v1.0   Date: 2024/12/4
Description:    本文件用于放置model.h里面声明的各种函数，以便调用
Others:         暂无
History:        暂无
    1.  Date: 2024/12/4
        Author: 吴沛熹
        ID: GamerNoTitle
        Modification:   [+] 新建了本文件，并把model.h中的函数挪入此文件
                        [+] 移入get_permission函数，基于用户角色快速获取用户权限
                        [+] 移入calculate_result函数，用于计算式子的正确答案
                        [+] 移入judge函数，用于判断用户答案是否正确
 */

#include "model.h"

/**************************** 用户模型和权限部分 ****************************/

/**
 * @brief 定义函数get_permission，用于获取用户的权限，返回包含用户权限数据的Permission结构体
 *
 * Parameter:
 *  @user   User类型的结构体，通过获取User.role来决定用户的权限
 *
 * Returns:
 *  @permission Permission类型的结构体，里面包含了用户的权限
 */
struct Permission get_permission(struct User user)
{
    struct Permission current_permission;
    switch (user.role) // 0为学生，1为老师
    {
    case 0:                                               // 学生权限
        current_permission.stu_answer = 1;                // 学生：回答问题
        current_permission.stu_inspect_personal_info = 1; // 学生：查看个人信息
        current_permission.stu_inspect_exam_info = 1;     // 学生：查看考次信息
        current_permission.tea_manage_exam = 0;
        current_permission.tea_manage_student = 0;
        current_permission.tea_inspect_student_info = 0;
        current_permission.tea_inspect_exam_scores = 0;
        current_permission.general_edit_info = 1;
        break;
    case 1: // 老师权限
        current_permission.stu_answer = 0;
        current_permission.stu_inspect_personal_info = 0;
        current_permission.stu_inspect_exam_info = 0;
        current_permission.tea_manage_exam = 1;          // 教师：管理考试
        current_permission.tea_manage_student = 1;       // 教师：管理学生（导入、删除、修改）
        current_permission.tea_inspect_student_info = 1; // 教师：查看学生信息
        current_permission.tea_inspect_exam_scores = 1;  // 教师：查看考试成绩
        current_permission.general_edit_info = 1;        // 通用：修改个人信息
        break;
    default: // 未知用户类型，为了安全起见，除了更改个人信息的权限，其他权限一律不给
        current_permission.stu_answer = 0;
        current_permission.stu_inspect_personal_info = 0;
        current_permission.stu_inspect_exam_info = 0;
        current_permission.tea_manage_exam = 0;
        current_permission.tea_manage_student = 0;
        current_permission.tea_inspect_student_info = 0;
        current_permission.tea_inspect_exam_scores = 0;
        current_permission.general_edit_info = 1;
        break;
    }
    return current_permission;
}
/**************************** 用户模型和权限部分结束 ****************************/

/**************************** 问题模型部分 ****************************/
/**
 * @brief 计算式子的正确答案，并返回答案
 *
 * @param num1 第一个操作数
 * @param num2 第二个操作数
 * @param op   运算符标识，0123分别对应+-×÷
 * @return float 正确答案
 */
float calculate_result(float num1, float num2, int op)
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
        return num1 / num2;
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
