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
#include "utils.h" // 引入日志功能

/*** 日志等级 ***/
#define LOGLEVEL_ERROR "ERROR"  // 定义错误级别日志
#define LOGLEVEL_INFO "INFO"    // 定义信息级别日志

/**************************** 用户模型和权限部分 ****************************/

/**
 * @brief 定义函数get_permission，用于获取用户的权限，返回包含用户权限数据的Permission结构体
 *
 * @param user 要获取权限的用户
 * @return struct Permission 用户的权限
 */
struct Permission get_permission(struct User user)
{
    // 记录获取权限的操作
    log_message(LOGLEVEL_INFO, "获取权限: 用户ID=%s, 角色=%d", user.id, user.role);

    struct Permission current_permission;
    switch (user.role) // 0为学生，1为老师
    {
    case 0: // 学生权限
        current_permission.stu_answer = 1;                // 学生：回答问题
        current_permission.stu_inspect_personal_info = 1; // 学生：查看个人信息
        current_permission.stu_inspect_exam_info = 1;     // 学生：查看考次信息
        current_permission.tea_manage_exam = 0;
        current_permission.tea_manage_student = 0;
        current_permission.tea_inspect_student_info = 0;
        current_permission.tea_inspect_exam_scores = 0;
        current_permission.general_edit_info = 1;
        // 记录分配学生权限
        log_message(LOGLEVEL_INFO, "分配学生权限给用户ID=%s", user.id);
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
        // 记录分配教师权限
        log_message(LOGLEVEL_INFO, "分配教师权限给用户ID=%s", user.id);
        break;
    default: // 未知用户类型，记录错误
        current_permission.stu_answer = 0;
        current_permission.stu_inspect_personal_info = 0;
        current_permission.stu_inspect_exam_info = 0;
        current_permission.tea_manage_exam = 0;
        current_permission.tea_manage_student = 0;
        current_permission.tea_inspect_student_info = 0;
        current_permission.tea_inspect_exam_scores = 0;
        current_permission.general_edit_info = 1;
        // 记录未知用户类型错误
        log_message(LOGLEVEL_ERROR, "未知用户类型: 用户ID=%s, 角色=%d", user.id, user.role);
        break;
    }
    return current_permission;
}
/**************************** 用户模型和权限部分结束 ****************************/
