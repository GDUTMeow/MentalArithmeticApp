/*
Copyright © GamerNoTitle 2024. All rights reserved.
File name: database.c
Author: 吴沛熹      ID: GamerNoTitle    Version: v1.0   Date: 2024/12/4
Description:    本文件用于定义预设的一些sql操作函数，包括成绩自定范围查询，学生的添加、修改、删除
                考试的查看、添加、删除，个人密码的修改等
Others:         暂无
History:        暂无
    1.  Date: 2024/12/6
        Author: 吴沛熹
        ID: GamerNoTitle
        Modification:   [+] 新建了本文件，并进行了必要的初始化
                        [+] 新增了函数 query_user_info 用于快速查找符合条件的用户，并返回到传入的 User 类型结构体
    2.  Date: 2024/12/7
        Author: 吴沛熹
        ID: GamerNoTitle
        Modification:   [+] 添加了函数 query_exam_info 用于查询单次考试的信息，并返回到传入的 SqlResponseExam 类型结构体
                        [+] 添加了函数 query_question_info 用于查询某一题的信息，并返回到传入的 SqlResponseQuestion 类型结构体
                        [+] 添加了函数 query_score_info 用于查询特定用户特定考次的成绩，并返回到传入的 SqlResponseScore 类型结构体
                        [+] 添加了函数 query_exams_info_all 用于对特定条件的考试进行多条查询，并返回到传入的 SqlResponseExam 类型结构体数组
                        [+] 添加了函数 query_user_info_all 用于按照特定条件对用户进行多条查询，并返回到传入的 SqlResponseUser 类型结构体数组
                        [+] 添加了函数 query_questions_info_all 用于按照特定条件对考试题目进行多条查询，并返回到传入的 SqlResponseQuestion 类型结构体数组
                        [+] 添加了函数 query_scores_info_all 用于按照特定条件对考试成绩进行多条查询，并返回到传入的 SqlResponseScore 类型结构体数组
                        [*] 添加了原本忘记的队model.h的引用
    3.  Date: 2024/12/8
        Author: 吴沛熹
        ID: GamerNoTitle
        Modification:   [+] 完成了数据库的增删改部分的代码
                        [+] 添加了绑定类型，使得数据库类型与参数类型的绑定更加容易
                        [+] 添加了测试用例代码
                        [*] 修复了一些因为数据未进行初始化而导致的bug

 */

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#include <windows.h>
#include "model.h"
#include "utils.h"
#include "../lib/sqlite3.h"

/*** 数据库部分 ***/
#define DB_FOLDER "db"                     // 数据库保存文件夹名
#define EXAMINATION_DB "db/examination.db" // 考试数据库
#define SCORES_DB "db/score.db"            // 成绩数据库
#define USER_DB "db/user.db"               // 用户数据库

/*** 日志等级 ***/
#define LOGLEVEL_ERROR "ERROR"
#define LOGLEVEL_INFO "INFO"

/*** 日志部分 ***/
#define LOG_FOLDER "logs"          // 日志文件夹路径
#define LOG_FILE "logs/latest.log" // 日志文件路径

/**
 * @brief 定义绑定参数的类型枚举
 *
 * @details 此枚举用于指定在 SQL 语句中绑定参数的类型，确保数据
 *          在传递给数据库时类型正确。它支持三种基本的数据类型：
 *          文本、整数和浮点数。
 *
 *          - **BIND_TYPE_TEXT**: 用于绑定字符串类型的数据，如 `char *`。
 *          - **BIND_TYPE_INT**: 用于绑定整数类型的数据，如 `int`。
 *          - **BIND_TYPE_FLOAT**: 用于绑定浮点数类型的数据，如 `float` 或 `double`。
 *
 *          在调用通用的插入函数 `insert_data_to_db` 时，通过
 *          提供对应的 `BindType` 数组，可以确保每个参数按照预期的类型
 *          被正确绑定到 SQL 语句中。
 */
typedef enum BINDING
{
    BIND_TYPE_TEXT,
    BIND_TYPE_INT,
    BIND_TYPE_FLOAT
} BindType;

/**
 * @brief 打开数据库并处理错误
 *
 * @param db_path 数据库路径
 * @param db 输出参数，数据库指针
 * @return int 成功返回0，否则返回1
 */
int open_database(const char *db_path, sqlite3 **db)
{
    int rc = sqlite3_open(db_path, db);
    if (rc != SQLITE_OK)
    {
        log_message(LOGLEVEL_ERROR, "无法打开数据库 '%s'：%s", db_path, sqlite3_errmsg(*db));
        sqlite3_close(*db);
        return 1;
    }
    return 0;
}

/**************************** 单条数据查询开始 ****************************/

/**
 * @brief 按照特定标准查询数据库 db/user.db 中符合条件的用户信息（只返回第一条）
 *
 * @param key 查询标准列名，可选值为 "id", "number", "name", "username"
 * @param content 查询内容，根据key进行匹配
 * @param user_to_return 返回的User结构体
 * @return int 执行成功返回0，否则返回1
 */
int query_user_info(const char *key, const char *content, struct User *user_to_return)
{
    sqlite3 *db;
    sqlite3_stmt *stmt;
    int rc;

    // 验证查询键是否合法
    if (strcmp(key, "id") != 0 && strcmp(key, "number") != 0 && strcmp(key, "name") != 0 && strcmp(key, "username") != 0)
    {
        log_message(LOGLEVEL_ERROR, "查询条件 %s 不合法！", key);
        return 1;
    }

    // 构建SQL语句
    char sql[256];
    snprintf(sql, sizeof(sql), "SELECT id, username, hashpass, salt, role, name, class_name, number, belong_to FROM users WHERE %s = ? LIMIT 1;", key);

    // 打开数据库
    if (open_database(USER_DB, &db))
    {
        return 1; // 打开数据库失败
    }

    // 准备查询语句
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK)
    {
        log_message(LOGLEVEL_ERROR, "准备 SQL 语句失败：%s", sqlite3_errmsg(db));
        sqlite3_close(db);
        return 1;
    }

    // 绑定查询参数
    rc = sqlite3_bind_text(stmt, 1, content, -1, SQLITE_STATIC);
    if (rc != SQLITE_OK)
    {
        log_message(LOGLEVEL_ERROR, "绑定查询内容参数失败：%s", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return 1;
    }

    // 执行查询并获取结果
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW)
    {
        // 从查询结果中提取数据
        strncpy(user_to_return->id, (const char *)sqlite3_column_text(stmt, 0), sizeof(user_to_return->id) - 1);
        strncpy(user_to_return->username, (const char *)sqlite3_column_text(stmt, 1), sizeof(user_to_return->username) - 1);

        // hashpass和salt不在User结构体中使用，略过
        // sqlite3_column_text(stmt, 2) => hashpass
        // sqlite3_column_text(stmt, 3) => salt

        user_to_return->role = sqlite3_column_int(stmt, 4);
        strncpy(user_to_return->name, (const char *)sqlite3_column_text(stmt, 5), sizeof(user_to_return->name) - 1);
        strncpy(user_to_return->class_name, (const char *)sqlite3_column_text(stmt, 6), sizeof(user_to_return->class_name) - 1);
        user_to_return->number = (unsigned int)sqlite3_column_int(stmt, 7);
        strncpy(user_to_return->belong_to, (const char *)sqlite3_column_text(stmt, 8), sizeof(user_to_return->belong_to) - 1);

        // 获取用户权限
        user_to_return->permission = get_permission(*user_to_return);

        log_message(LOGLEVEL_INFO, "成功查询到用户信息，用户ID：%s", user_to_return->id);
    }
    else
    {
        // 没有查询到结果
        log_message(LOGLEVEL_INFO, "没有找到符合条件的用户信息");
    }

    // 清理和关闭数据库
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return 0;
}
/**
 * @brief 按照特定标准查询数据库 db/examination.db 中符合条件的考试条目（只返回第一条）
 *
 * @param key 查询的标准，合法的范围为 id(考试唯一标识符), name(考试名称)
 * @param content 查询的内容，依据提供的查询标准，需要查询的内容需要填入content
 * @param exam_to_return 查询结果返回的 SqlResponseExam 结构体类型变量
 *
 * @return int 函数执行成功与否，成功返回0，否则为1
 */
int query_exam_info(const char *key, const char *content, struct SqlResponseExam *exam_to_return)
{
    sqlite3 *db;
    sqlite3_stmt *stmt;
    int rc;

    // 验证查询键是否合法
    if (strcmp(key, "id") != 0 && strcmp(key, "name") != 0)
    {
        log_message(LOGLEVEL_ERROR, "查询条件 %s 不合法！", key);
        return 1;
    }

    // 动态构建SQL语句
    char sql[256];
    snprintf(sql, sizeof(sql), "SELECT id, name, start_time, end_time, allow_answer_when_expired, random_question FROM examinations WHERE %s = ? LIMIT 1;", key);

    // 打开数据库
    if (open_database(EXAMINATION_DB, &db))
    {
        return 1;
    }

    // 准备查询语句
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK)
    {
        log_message(LOGLEVEL_ERROR, "准备 SQL 语句失败：%s", sqlite3_errmsg(db));
        sqlite3_close(db);
        return 1;
    }

    // 绑定查询内容
    rc = sqlite3_bind_text(stmt, 1, content, -1, SQLITE_STATIC);
    if (rc != SQLITE_OK)
    {
        log_message(LOGLEVEL_ERROR, "绑定查询内容参数失败：%s", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return 1;
    }

    // 执行查询并获取结果
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW)
    {
        // 填充SqlResponseExam结构体
        strncpy(exam_to_return->id, (const char *)sqlite3_column_text(stmt, 0), sizeof(exam_to_return->id) - 1);
        strncpy(exam_to_return->name, (const char *)sqlite3_column_text(stmt, 1), sizeof(exam_to_return->name) - 1);
        exam_to_return->start_time = sqlite3_column_int(stmt, 2);
        exam_to_return->end_time = sqlite3_column_int(stmt, 3);
        exam_to_return->allow_answer_when_expired = sqlite3_column_int(stmt, 4);
        exam_to_return->random_question = sqlite3_column_int(stmt, 5);

        log_message(LOGLEVEL_INFO, "成功查询到考试信息，考试ID：%s", exam_to_return->id);
    }
    else
    {
        log_message(LOGLEVEL_INFO, "没有找到符合条件的考试信息");
    }

    // 清理和关闭数据库
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return 0;
}

/**
 * @brief 按照特定标准查询数据库 db/examination.db 中符合条件的题目信息（只返回第一条）
 *
 * @param key 查询的标准，合法的范围为 id(题目唯一标识符), exam_id(考试ID)
 * @param content 查询的内容，依据提供的查询标准，需要查询的内容需要填入content
 * @param question_to_return 查询结果返回的 Question 结构体类型变量
 *
 * @return int 函数执行成功与否，成功返回0，否则为1
 */
int query_question_info(const char *key, const char *content, struct SqlResponseQuestion *question_to_return)
{
    sqlite3 *db;
    sqlite3_stmt *stmt;
    int rc;

    // 验证查询键是否合法
    if (strcmp(key, "id") != 0 && strcmp(key, "exam_id") != 0)
    {
        log_message(LOGLEVEL_ERROR, "查询条件 %s 不合法！", key);
        return 1;
    }

    // 动态构建SQL语句
    char sql[256];
    snprintf(sql, sizeof(sql), "SELECT id, exam_id, num1, op, num2 FROM questions WHERE %s = ? LIMIT 1;", key);

    // 打开数据库
    if (open_database(EXAMINATION_DB, &db))
    {
        return 1;
    }

    // 采用了一个很笨的方式来初始化，避免后面读取再出问题
    strcpy(question_to_return->id, "");
    strcpy(question_to_return->exam_id, "");
    question_to_return->id[36] = '\0';
    question_to_return->exam_id[36] = '\0';

    // 准备查询语句
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK)
    {
        log_message(LOGLEVEL_ERROR, "准备 SQL 语句失败：%s", sqlite3_errmsg(db));
        sqlite3_close(db);
        return 1;
    }

    // 绑定查询内容
    rc = sqlite3_bind_text(stmt, 1, content, -1, SQLITE_STATIC);
    if (rc != SQLITE_OK)
    {
        log_message(LOGLEVEL_ERROR, "绑定查询内容参数失败：%s", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return 1;
    }

    // 执行查询并获取结果
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW)
    {
        // 填充SqlResponseQuestion结构体
        strncpy(question_to_return->id, (const char *)sqlite3_column_text(stmt, 0), sizeof(question_to_return->id) - 1);
        strncpy(question_to_return->exam_id, (const char *)sqlite3_column_text(stmt, 1), sizeof(question_to_return->exam_id) - 1);
        question_to_return->num1 = sqlite3_column_double(stmt, 2);
        question_to_return->op = sqlite3_column_int(stmt, 3);
        question_to_return->num2 = sqlite3_column_double(stmt, 4);

        log_message(LOGLEVEL_INFO, "成功查询到题目信息，题目ID：%s", question_to_return->id);
    }
    else
    {
        log_message(LOGLEVEL_INFO, "没有找到符合条件的题目信息");
    }

    // 清理和关闭数据库
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return 0;
}

/**
 * @brief 根据考试ID和用户ID查询成绩信息，并将结果存储到 SqlResponseScore 结构体中
 *
 * @param exam_id 考试ID，查询指定考试的成绩
 * @param user_id 用户ID，查询指定用户的成绩
 * @param score_to_return 查询结果返回的 SqlResponseScore 结构体类型变量
 *
 * @return int 函数执行成功与否，成功返回0，否则为1
 */
int query_score_info(const char *exam_id, const char *user_id, struct SqlResponseScore *score_to_return)
{
    sqlite3 *db;
    sqlite3_stmt *stmt;
    int rc;

    const char *sql = "SELECT id, exam_id, user_id, score, expired_flag FROM scores WHERE exam_id = ? AND user_id = ? LIMIT 1;";

    // 打开数据库
    if (open_database(SCORES_DB, &db))
    {
        return 1;
    }

    // 初始化传入的scores_to_return来避免出错
    // 用了一个比较傻的方式，但是管用
    strcpy(score_to_return->id, "");
    strcpy(score_to_return->exam_id, "");
    strcpy(score_to_return->user_id, "");
    score_to_return->id[36] = '\0';
    score_to_return->exam_id[36] = '\0';
    score_to_return->user_id[36] = '\0';

    // 准备查询语句
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK)
    {
        log_message(LOGLEVEL_ERROR, "准备 SQL 语句失败：%s", sqlite3_errmsg(db));
        sqlite3_close(db);
        return 1;
    }

    // 绑定考试ID
    rc = sqlite3_bind_text(stmt, 1, exam_id, -1, SQLITE_STATIC);
    if (rc != SQLITE_OK)
    {
        log_message(LOGLEVEL_ERROR, "绑定考试ID参数失败：%s", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return 1;
    }

    // 绑定用户ID
    rc = sqlite3_bind_text(stmt, 2, user_id, -1, SQLITE_STATIC);
    if (rc != SQLITE_OK)
    {
        log_message(LOGLEVEL_ERROR, "绑定用户ID参数失败：%s", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return 1;
    }

    // 执行查询并获取结果
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW)
    {
        // 填充SqlResponseScore结构体
        strncpy(score_to_return->id, (const char *)sqlite3_column_text(stmt, 0), sizeof(score_to_return->id) - 1);
        strncpy(score_to_return->exam_id, (const char *)sqlite3_column_text(stmt, 1), sizeof(score_to_return->exam_id) - 1);
        strncpy(score_to_return->user_id, (const char *)sqlite3_column_text(stmt, 2), sizeof(score_to_return->user_id) - 1);
        score_to_return->score = (float)sqlite3_column_double(stmt, 3);
        score_to_return->expired_flag = sqlite3_column_int(stmt, 4);

        log_message(LOGLEVEL_INFO, "成功查询到成绩信息，考试ID：%s，用户ID：%s", exam_id, user_id);
    }
    else
    {
        log_message(LOGLEVEL_INFO, "没有找到符合条件的成绩信息，考试ID：%s，用户ID：%s", exam_id, user_id);
    }

    // 清理和关闭数据库
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return 0;
}

/**************************** 单条数据查询结束 ****************************/

/**************************** 多条数据查询开始 ****************************/

/**
 * @brief 查询数据库 db/examination.db 中符合条件的所有考试信息（返回多条数据）
 *
 * @param name 考试名称，支持模糊查询，可以为空表示查询所有考试
 * @param start_time 查询起始时间的最小值，-1表示不限制
 * @param end_time 查询结束时间的最大值，-1表示不限制
 * @param exams_to_return 查询结果返回的 SqlResponseExam 结构体类型数组
 * @param length 查询结果数组的大小，同时也是查询结果返回的限制数量，类似于一个外置的LIMIT
 *
 * @return int 函数执行成功与否，成功返回0，否则为1
 */
int query_exams_info_all(const char *name, int start_time, int end_time, struct SqlResponseExam *exams_to_return, int length)
{
    sqlite3 *db;
    sqlite3_stmt *stmt;
    int rc;
    int count = 0;

    // 构建SQL语句
    char sql[512];
    snprintf(sql, sizeof(sql),
             "SELECT id, name, start_time, end_time, allow_answer_when_expired, random_question FROM examinations "
             "WHERE (name LIKE ?) AND (start_time >= ?) AND (end_time <= ?) LIMIT ?;");

    // 初始化传入的exams_to_return，避免返回出错
    for (int i = 0; i < length; i++)
    {
        strcpy(exams_to_return[i].id, "");
    }

    // 打开数据库
    if (open_database(EXAMINATION_DB, &db))
    {
        return 1;
    }

    // 准备查询语句
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK)
    {
        log_message(LOGLEVEL_ERROR, "准备 SQL 语句失败：%s", sqlite3_errmsg(db));
        sqlite3_close(db);
        return 1;
    }

    // 绑定参数
    const char *name_param = (strlen(name) > 0) ? name : "%%";
    rc = sqlite3_bind_text(stmt, 1, name_param, -1, SQLITE_STATIC);
    if (rc != SQLITE_OK)
    {
        log_message(LOGLEVEL_ERROR, "绑定名称参数失败：%s", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return 1;
    }

    if (start_time == -1)
    {
        rc = sqlite3_bind_int(stmt, 2, 0); // 不限制开始时间
    }
    else
    {
        rc = sqlite3_bind_int(stmt, 2, start_time); // 绑定开始时间起始条件
    }
    if (rc != SQLITE_OK)
    {
        log_message(LOGLEVEL_ERROR, "绑定开始时间条件失败：%s", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return 1;
    }

    if (end_time == -1)
    {
        rc = sqlite3_bind_int(stmt, 3, INT32_MAX); // 设置为INT32_MAX表示不限制
    }
    else
    {
        rc = sqlite3_bind_int(stmt, 3, end_time); // 绑定结束时间截止条件
    }
    if (rc != SQLITE_OK)
    {
        log_message(LOGLEVEL_ERROR, "绑定结束时间条件失败：%s", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return 1;
    }

    rc = sqlite3_bind_int(stmt, 4, length); // 绑定LIMIT参数
    if (rc != SQLITE_OK)
    {
        log_message(LOGLEVEL_ERROR, "绑定LIMIT参数失败：%s", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return 1;
    }

    // 执行查询
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW && count < length)
    {
        strncpy(exams_to_return[count].id, (const char *)sqlite3_column_text(stmt, 0), sizeof(exams_to_return[count].id) - 1);
        exams_to_return[count].id[sizeof(exams_to_return[count].id) - 1] = '\0'; // 确保字符串终止

        strncpy(exams_to_return[count].name, (const char *)sqlite3_column_text(stmt, 1), sizeof(exams_to_return[count].name) - 1);
        exams_to_return[count].name[sizeof(exams_to_return[count].name) - 1] = '\0'; // 确保字符串终止

        exams_to_return[count].start_time = sqlite3_column_int(stmt, 2);
        exams_to_return[count].end_time = sqlite3_column_int(stmt, 3);
        exams_to_return[count].allow_answer_when_expired = sqlite3_column_int(stmt, 4);
        exams_to_return[count].random_question = sqlite3_column_int(stmt, 5);
        count++;
    }

    if (count > 0)
    {
        log_message(LOGLEVEL_INFO, "成功查询到 %d 条考试信息", count);
    }
    else
    {
        log_message(LOGLEVEL_INFO, "没有找到符合条件的考试信息");
    }

    // 清理和关闭数据库
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return 0;
}

/**
 * @brief 查询数据库 db/user.db 中符合条件的所有用户信息（返回多条数据）
 *
 * @param name 用户姓名，支持模糊查询，可以为空表示查询所有用户
 * @param class_name 用户班级，支持模糊查询，可以为空表示查询所有班级
 * @param role 用户角色，需要精确查询，0表示学生，1表示老师
 * @param number 用户学号/工号，支持模糊查询，可以为空表示查询所有用户
 * @param belong_to (仅学生) 学生归属的老师，需要填入老师的UUID，可以为空表示不限制
 * @param users_to_return 查询结果返回的 User 结构体类型数组
 * @param length 查询结果数组的大小
 *
 * @return int 函数执行成功与否，成功返回0，否则为1
 */
int query_users_info_all(const char *name, const char *class_name, int role, const char *number, const char *belong_to, struct User *users_to_return, int length)
{
    sqlite3 *db;
    sqlite3_stmt *stmt;
    int rc;
    int count = 0;

    // 构建SQL语句，使用 CAST 将 number 转换为 TEXT 以支持 LIKE 运算符
    const char *cmd = "SELECT id, username, role, name, class_name, number, belong_to FROM users "
                      "WHERE (name LIKE ?) AND (class_name LIKE ?) AND (role = ?) AND (CAST(number AS TEXT) LIKE ?) AND (belong_to LIKE ?) LIMIT ?;";

    // 打开数据库
    if (open_database(USER_DB, &db))
    {
        return 1; // 打开数据库失败
    }

    // 启用外键支持
    sqlite3_exec(db, "PRAGMA foreign_keys = ON;", NULL, NULL, NULL);

    // 初始化传入的user_to_return，避免返回出错
    for (int count = 0; count < length; count++)
    {
        strcpy(users_to_return[count].id, "");
    }

    // 准备查询语句
    rc = sqlite3_prepare_v2(db, cmd, -1, &stmt, 0);
    if (rc != SQLITE_OK)
    {
        log_message(LOGLEVEL_ERROR, "准备 SQL 语句失败：%s", sqlite3_errmsg(db));
        sqlite3_close(db);
        return 1; // 执行失败
    }

    // 绑定查询条件
    // 如果参数为空，则使用通配符 '%' 进行匹配所有
    const char *name_param = (name && strlen(name) > 0) ? name : "%%";
    const char *class_param = (class_name && strlen(class_name) > 0) ? class_name : "%%";
    const char *number_param = (number && strlen(number) > 0) ? number : "%%";
    const char *belong_to_param = (belong_to && strlen(belong_to) > 0) ? belong_to : "%%";

    rc = sqlite3_bind_text(stmt, 1, name_param, -1, SQLITE_STATIC);
    if (rc != SQLITE_OK)
    {
        log_message(LOGLEVEL_ERROR, "绑定姓名参数失败：%s", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return 1; // 执行失败
    }

    rc = sqlite3_bind_text(stmt, 2, class_param, -1, SQLITE_STATIC);
    if (rc != SQLITE_OK)
    {
        log_message(LOGLEVEL_ERROR, "绑定班级参数失败：%s", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return 1; // 执行失败
    }

    rc = sqlite3_bind_int(stmt, 3, role);
    if (rc != SQLITE_OK)
    {
        log_message(LOGLEVEL_ERROR, "绑定角色参数失败：%s", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return 1; // 执行失败
    }

    rc = sqlite3_bind_text(stmt, 4, number_param, -1, SQLITE_STATIC);
    if (rc != SQLITE_OK)
    {
        log_message(LOGLEVEL_ERROR, "绑定学号/工号参数失败：%s", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return 1; // 执行失败
    }

    rc = sqlite3_bind_text(stmt, 5, belong_to_param, -1, SQLITE_STATIC);
    if (rc != SQLITE_OK)
    {
        log_message(LOGLEVEL_ERROR, "绑定归属教师参数失败：%s", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return 1; // 执行失败
    }

    rc = sqlite3_bind_int(stmt, 6, length);
    if (rc != SQLITE_OK)
    {
        log_message(LOGLEVEL_ERROR, "绑定LIMIT参数失败：%s", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return 1; // 执行失败
    }

    // 执行查询并填充结果
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW && count < length)
    {
        // 提取查询结果并确保字符串正确终止
        const unsigned char *id_text = sqlite3_column_text(stmt, 0);
        const unsigned char *username_text = sqlite3_column_text(stmt, 1);
        const unsigned char *name_text = sqlite3_column_text(stmt, 3);
        const unsigned char *class_name_text = sqlite3_column_text(stmt, 4);
        const unsigned char *belong_to_text = sqlite3_column_text(stmt, 6);

        if (id_text && username_text && name_text && class_name_text && belong_to_text)
        {
            strncpy(users_to_return[count].id, (const char *)id_text, sizeof(users_to_return[count].id) - 1);
            users_to_return[count].id[sizeof(users_to_return[count].id) - 1] = '\0'; // 确保字符串终止

            strncpy(users_to_return[count].username, (const char *)username_text, sizeof(users_to_return[count].username) - 1);
            users_to_return[count].username[sizeof(users_to_return[count].username) - 1] = '\0'; // 确保字符串终止

            users_to_return[count].role = sqlite3_column_int(stmt, 2);

            strncpy(users_to_return[count].name, (const char *)name_text, sizeof(users_to_return[count].name) - 1);
            users_to_return[count].name[sizeof(users_to_return[count].name) - 1] = '\0'; // 确保字符串终止

            strncpy(users_to_return[count].class_name, (const char *)class_name_text, sizeof(users_to_return[count].class_name) - 1);
            users_to_return[count].class_name[sizeof(users_to_return[count].class_name) - 1] = '\0'; // 确保字符串终止

            users_to_return[count].number = (unsigned int)sqlite3_column_int(stmt, 5); // 使用 sqlite3_column_int

            strncpy(users_to_return[count].belong_to, (const char *)belong_to_text, sizeof(users_to_return[count].belong_to) - 1);
            users_to_return[count].belong_to[sizeof(users_to_return[count].belong_to) - 1] = '\0'; // 确保字符串终止

            // 获取用户权限
            users_to_return[count].permission = get_permission(users_to_return[count]);

            count++;
        }
    }

    if (count > 0)
    {
        log_message(LOGLEVEL_INFO, "成功查询到 %d 条用户信息", count);
    }
    else
    {
        log_message(LOGLEVEL_INFO, "没有找到符合条件的用户信息", cmd);
    }

    // 清理和关闭数据库
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return 0; // 执行成功
}
/**
 * @brief 查询数据库 db/examination.db 中符合条件的所有问题信息（返回多条数据）
 *
 * @param exam_id 所属考试的ID，只允许此方式查询，用来构建问题列表
 * @param questions_to_return 查询结果返回的 SqlResponseQuestion 结构体类型数组
 * @param length 查询结果数组的大小
 *
 * @return int 函数执行成功与否，成功返回0，否则为1
 */
int query_questions_info_all(const char *exam_id, struct SqlResponseQuestion *questions_to_return, int length)
{
    sqlite3 *db;
    sqlite3_stmt *stmt;
    int rc;
    int count = 0;

    const char *sql = "SELECT id, exam_id, num1, op, num2 FROM questions WHERE exam_id = ? LIMIT ?;";

    // 打开数据库
    if (open_database(EXAMINATION_DB, &db))
    {
        return 1;
    }

    // 启用外键支持
    sqlite3_exec(db, "PRAGMA foreign_keys = ON;", NULL, NULL, NULL);

    // 初始化传入的questions_to_return，避免出错
    for (int count = 0; count < length; count++)
    {
        strcpy(questions_to_return[count].id, "");
    }

    // 准备查询语句
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK)
    {
        log_message(LOGLEVEL_ERROR, "准备 SQL 语句失败：%s", sqlite3_errmsg(db));
        sqlite3_close(db);
        return 1;
    }

    // 绑定考试ID
    rc = sqlite3_bind_text(stmt, 1, exam_id, -1, SQLITE_STATIC);
    if (rc != SQLITE_OK)
    {
        log_message(LOGLEVEL_ERROR, "绑定考试ID参数失败：%s", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return 1;
    }

    // 绑定LIMIT参数
    rc = sqlite3_bind_int(stmt, 2, length);
    if (rc != SQLITE_OK)
    {
        log_message(LOGLEVEL_ERROR, "绑定LIMIT参数失败：%s", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return 1;
    }

    // 执行查询
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW && count < length)
    {
        // 提取查询结果并确保字符串正确终止
        const unsigned char *id_text = sqlite3_column_text(stmt, 0);
        const unsigned char *exam_id_text = sqlite3_column_text(stmt, 1);

        if (id_text && exam_id_text)
        {
            strncpy(questions_to_return[count].id, (const char *)id_text, sizeof(questions_to_return[count].id) - 1);
            questions_to_return[count].id[sizeof(questions_to_return[count].id) - 1] = '\0'; // 确保字符串终止

            strncpy(questions_to_return[count].exam_id, (const char *)exam_id_text, sizeof(questions_to_return[count].exam_id) - 1);
            questions_to_return[count].exam_id[sizeof(questions_to_return[count].exam_id) - 1] = '\0'; // 确保字符串终止

            questions_to_return[count].num1 = (float)sqlite3_column_double(stmt, 2);
            questions_to_return[count].op = sqlite3_column_int(stmt, 3);
            questions_to_return[count].num2 = (float)sqlite3_column_double(stmt, 4);
            count++;
        }
    }

    if (count > 0)
    {
        log_message(LOGLEVEL_INFO, "成功查询到 %d 条问题信息", count);
    }
    else
    {
        log_message(LOGLEVEL_INFO, "没有找到符合条件的问题信息");
    }

    // 清理和关闭数据库
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return 0;
}

/**
 * @brief 查询数据库 db/examination.db 中符合条件的所有问题信息（返回多条数据）
 *
 * @param exam_id 考试ID（UUID，唯一）
 * @param student_number 学生的学号
 * @param scores_to_return 将数据返回的 SqlResponseScore 类型结构体模型
 * @param length 返回数据数量
 * @return int 成功返回0，否则返回1
 */
int query_scores_info_all(const char *exam_id, const char *student_number, struct SqlResponseScore *scores_to_return, int length)
{
    sqlite3 *db;
    sqlite3_stmt *stmt;
    int rc;
    int count = 0;

    // 构建SQL语句，使用 '=' 进行精确匹配
    const char *sql = "SELECT id, exam_id, user_id, score, expired_flag FROM scores "
                      "WHERE (exam_id LIKE ?) AND (user_id LIKE ?) LIMIT ?;";

    // 打开数据库
    if (open_database(SCORES_DB, &db))
    {
        return 1;
    }

    // 启用外键支持
    sqlite3_exec(db, "PRAGMA foreign_keys = ON;", NULL, NULL, NULL);

    // 初始化传入的scores_to_return，避免出错
    for (int count = 0; count < length; count++)
    {
        strcpy(scores_to_return[count].id, "");
    }

    // 准备查询语句
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK)
    {
        log_message(LOGLEVEL_ERROR, "准备 SQL 语句失败：%s", sqlite3_errmsg(db));
        sqlite3_close(db);
        return 1;
    }

    // 绑定考试ID
    const char *exam_param = (strlen(exam_id) > 0) ? exam_id : "%%";
    rc = sqlite3_bind_text(stmt, 1, exam_param, -1, SQLITE_STATIC);
    if (rc != SQLITE_OK)
    {
        log_message(LOGLEVEL_ERROR, "绑定考试ID参数失败：%s", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return 1;
    }

    // 绑定学号/工号
    const char *student_param = (strlen(student_number) > 0) ? student_number : "%%";
    rc = sqlite3_bind_text(stmt, 2, student_param, -1, SQLITE_STATIC);
    if (rc != SQLITE_OK)
    {
        log_message(LOGLEVEL_ERROR, "绑定学号/工号参数失败：%s", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return 1;
    }

    // 绑定LIMIT参数
    rc = sqlite3_bind_int(stmt, 3, length);
    if (rc != SQLITE_OK)
    {
        log_message(LOGLEVEL_ERROR, "绑定LIMIT参数失败：%s", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return 1;
    }

    // 执行查询
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW && count < length)
    {
        // 提取查询结果并确保字符串正确终止
        const unsigned char *id_text = sqlite3_column_text(stmt, 0);
        const unsigned char *exam_id_text = sqlite3_column_text(stmt, 1);
        const unsigned char *user_id_text = sqlite3_column_text(stmt, 2);

        if (id_text && exam_id_text && user_id_text)
        {
            strncpy(scores_to_return[count].id, (const char *)id_text, sizeof(scores_to_return[count].id) - 1);
            scores_to_return[count].id[sizeof(scores_to_return[count].id) - 1] = '\0'; // 确保字符串终止

            strncpy(scores_to_return[count].exam_id, (const char *)exam_id_text, sizeof(scores_to_return[count].exam_id) - 1);
            scores_to_return[count].exam_id[sizeof(scores_to_return[count].exam_id) - 1] = '\0'; // 确保字符串终止

            strncpy(scores_to_return[count].user_id, (const char *)user_id_text, sizeof(scores_to_return[count].user_id) - 1);
            scores_to_return[count].user_id[sizeof(scores_to_return[count].user_id) - 1] = '\0'; // 确保字符串终止

            scores_to_return[count].score = (float)sqlite3_column_double(stmt, 3);
            scores_to_return[count].expired_flag = sqlite3_column_int(stmt, 4);
            count++;
        }
    }

    if (count > 0)
    {
        log_message(LOGLEVEL_INFO, "成功查询到 %d 条成绩信息", count);
    }
    else
    {
        log_message(LOGLEVEL_INFO, "没有找到符合条件的成绩信息");
    }

    // 清理和关闭数据库
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return 0;
}

/**************************** 多条数据查询结束 ****************************/

/**************************** 单条数据插入开始 ****************************/

/**
 * @brief 对指定的数据库进行指定表的数据插入操作
 *
 * @param db_path 数据库路径
 * @param sql 插入的 SQL 语句
 * @param bindings 参数绑定（替换问号），按顺序排列
 * @param types 参数类型数组，对应每个绑定参数的类型
 * @param num_bindings 绑定参数的数量
 * @return int 成功返回0，否则返回1
 */
int insert_data_to_db(const char *db_path, const char *sql, const void **bindings, const BindType *types, int num_bindings)
{
    FILE *log_file = fopen(LOG_FILE, "a"); // 'a' 表示附加模式
    if (log_file == NULL)
    {
        printf("无法打开日志文件：%s\n", strerror(errno));
        return 1;
    }

    sqlite3 *db;
    sqlite3_stmt *stmt;
    int rc;

    char current_time[20];

    // 打开数据库
    rc = sqlite3_open(db_path, &db);
    if (rc != SQLITE_OK)
    {
        get_current_time(current_time, sizeof(current_time));
        fprintf(log_file, "%s [%s]: 无法打开数据库 '%s'：%s\n", current_time, LOGLEVEL_ERROR, db_path, sqlite3_errmsg(db));
        sqlite3_close(db);
        fclose(log_file);
        return 1;
    }

    // 准备SQL语句
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK)
    {
        get_current_time(current_time, sizeof(current_time));
        fprintf(log_file, "%s [%s]: 无法准备SQL语句：%s\n", current_time, LOGLEVEL_ERROR, sqlite3_errmsg(db));
        sqlite3_close(db);
        fclose(log_file);
        return 1;
    }

    // 绑定参数
    for (int i = 0; i < num_bindings; ++i)
    {
        switch (types[i])
        {
        case BIND_TYPE_TEXT:
            rc = sqlite3_bind_text(stmt, i + 1, (const char *)bindings[i], -1, SQLITE_STATIC);
            break;
        case BIND_TYPE_INT:
            rc = sqlite3_bind_int(stmt, i + 1, *(const int *)bindings[i]);
            break;
        case BIND_TYPE_FLOAT:
            rc = sqlite3_bind_double(stmt, i + 1, *(const double *)bindings[i]);
            break;
        default:
            // 未知类型
            get_current_time(current_time, sizeof(current_time));
            fprintf(log_file, "%s [%s]: 未知的绑定类型 %d\n", current_time, LOGLEVEL_ERROR, types[i]);
            sqlite3_finalize(stmt);
            sqlite3_close(db);
            fclose(log_file);
            return 1;
        }

        if (rc != SQLITE_OK)
        {
            get_current_time(current_time, sizeof(current_time));
            fprintf(log_file, "%s [%s]: 绑定参数失败：%s\n", current_time, LOGLEVEL_ERROR, sqlite3_errmsg(db));
            sqlite3_finalize(stmt);
            sqlite3_close(db);
            fclose(log_file);
            return 1;
        }
    }

    // 执行SQL语句
    rc = sqlite3_step(stmt);
    get_current_time(current_time, sizeof(current_time));
    if (rc != SQLITE_DONE)
    {
        fprintf(log_file, "%s [%s]: 执行插入操作失败：%s\n", current_time, LOGLEVEL_ERROR, sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        fclose(log_file);
        return 1;
    }
    else
    {
        fprintf(log_file, "%s [%s]: 数据插入成功！\n", current_time, LOGLEVEL_INFO);
    }

    // 清理和关闭数据库
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    fclose(log_file);
    return 0;
}

/**
 * @brief 向考试表中插入新的考试数据
 *
 * @param exam_id 考试ID（唯一，UUID）
 * @param name 考试名称
 * @param start_time 考试的开始时间（时间戳）
 * @param end_time 考试的结束时间（时间戳）
 * @param allow_answer_when_expired 是否允许逾期作答，0表示不允许，1表示允许
 * @param random_question 是否开启随机问题顺序，0表示关闭，1表示开启
 * @return int 函数是否成功执行，成功返回0，否则返回1
 */
int insert_exam_data(const char *exam_id, const char *name, int start_time, int end_time, int allow_answer_when_expired, int random_question)
{
    FILE *log_file = fopen(LOG_FILE, "a"); // 'a' 表示附加模式
    char current_time[20];
    const char *sql = "INSERT INTO examinations (id, name, start_time, end_time, allow_answer_when_expired, random_question) VALUES (?, ?, ?, ?, ?, ?);";

    // 数据校验
    if (allow_answer_when_expired != 0 && allow_answer_when_expired != 1)
    {
        get_current_time(current_time, sizeof(current_time));
        fprintf(log_file, "%s [%s]: 对考试数据库执行插入操作的时候遇到了问题: 逾期作答值非法！%d\n", current_time, LOGLEVEL_ERROR, allow_answer_when_expired);
        fclose(log_file);
        return 1;
    }
    if (random_question != 0 && random_question != 1)
    {
        get_current_time(current_time, sizeof(current_time));
        fprintf(log_file, "%s [%s]: 对考试数据库执行插入操作的时候遇到了问题: 随机问题值非法！%d\n", current_time, LOGLEVEL_ERROR, random_question);
        fclose(log_file);
        return 1;
    }

    // 定义绑定参数
    const void *bindings[] = {exam_id, name, &start_time, &end_time, &allow_answer_when_expired, &random_question};
    const BindType types[] = {BIND_TYPE_TEXT, BIND_TYPE_TEXT, BIND_TYPE_INT, BIND_TYPE_INT, BIND_TYPE_INT, BIND_TYPE_INT};

    // 调用通用插入函数
    int result = insert_data_to_db(EXAMINATION_DB, sql, bindings, types, 6);

    fclose(log_file);
    return result;
}

/**
 * @brief 向考试数据库的 questions 表插入新的问题
 *
 * @param question_id 问题ID（唯一，UUID）
 * @param exam_id 考试ID（UUID）
 * @param num1 第一个操作数
 * @param op 运算符，只有0123是合法的
 * @param num2 第二个操作数
 * @return int 函数是否成功执行，成功返回0，否则返回1
 */
int insert_question_data(const char *question_id, const char *exam_id, float num1, int op, float num2)
{
    FILE *log_file = fopen(LOG_FILE, "a"); // 'a' 表示附加模式
    char current_time[20];
    const char *sql = "INSERT INTO questions (id, exam_id, num1, op, num2) VALUES (?, ?, ?, ?, ?);";

    // 数据校验
    if (op != 0 && op != 1 && op != 2 && op != 3)
    {
        get_current_time(current_time, sizeof(current_time));
        fprintf(log_file, "%s [%s]: 对问题数据库执行插入操作的时候遇到了问题: 运算符值非法！%d\n", current_time, LOGLEVEL_ERROR, op);
        fclose(log_file);
        return 1;
    }

    // 定义绑定参数
    const void *bindings[] = {question_id, exam_id, &num1, &op, &num2};
    const BindType types[] = {BIND_TYPE_TEXT, BIND_TYPE_TEXT, BIND_TYPE_FLOAT, BIND_TYPE_INT, BIND_TYPE_FLOAT};

    // 调用通用插入函数
    int result = insert_data_to_db(EXAMINATION_DB, sql, bindings, types, 5);

    fclose(log_file);
    return result;
}

/**
 * @brief 向成绩数据库的 scores 表插入新的成绩
 *
 * @param score_id 成绩ID（UUID，唯一）
 * @param exam_id 考试ID（UUID）
 * @param user_id 用户ID（UUID）
 * @param score 分数
 * @param expired_flag 逾期作答标记，只有0和1合法
 * @return int 函数是否成功执行，成功返回0，否则返回1
 */
int insert_score_data(const char *score_id, const char *exam_id, const char *user_id, float score, int expired_flag)
{
    FILE *log_file = fopen(LOG_FILE, "a"); // 'a' 表示附加模式
    char current_time[20];
    const char *sql = "INSERT INTO scores (id, exam_id, user_id, score, expired_flag) VALUES (?, ?, ?, ?, ?);";

    // 数据校验
    if (expired_flag != 0 && expired_flag != 1)
    {
        get_current_time(current_time, sizeof(current_time));
        fprintf(log_file, "%s [%s]: 对成绩数据库执行插入操作的时候遇到了问题: 逾期作答标记值非法！%d\n", current_time, LOGLEVEL_ERROR, expired_flag);
        fclose(log_file);
        return 1;
    }

    // 定义绑定参数
    const void *bindings[] = {score_id, exam_id, user_id, &score, &expired_flag};
    const BindType types[] = {BIND_TYPE_TEXT, BIND_TYPE_TEXT, BIND_TYPE_TEXT, BIND_TYPE_FLOAT, BIND_TYPE_INT};

    // 调用通用插入函数
    int result = insert_data_to_db(SCORES_DB, sql, bindings, types, 5);

    fclose(log_file);
    return result;
}

/**
 * @brief 向用户数据库的 users 表插入新的用户
 *
 * @param user_id 用户ID（唯一，UUID）
 * @param username 用户名，范围为[a-zA-Z0-9]{3, 24}
 * @param hashpass sha512(salt + passwd)
 * @param salt 盐
 * @param role 用户角色
 * @param name 真实姓名
 * @param class_name 班级名（可空）
 * @param number 学号/工号
 * @param belong_to 归属教师（UUID，可空）
 * @return int 函数是否成功执行，成功返回0，否则返回1
 */
int insert_user_data(const char *user_id, const char *username, const char *hashpass, const char *salt, int role, const char *name, const char *class_name, int number, const char *belong_to)
{
    const char *sql = "INSERT INTO users (id, username, hashpass, salt, role, name, class_name, number, belong_to) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?);";

    // 定义绑定参数
    const void *bindings[] = {user_id, username, hashpass, salt, &role, name, class_name, &number, belong_to};
    const BindType types[] = {BIND_TYPE_TEXT, BIND_TYPE_TEXT, BIND_TYPE_TEXT, BIND_TYPE_TEXT, BIND_TYPE_INT, BIND_TYPE_TEXT, BIND_TYPE_TEXT, BIND_TYPE_INT, BIND_TYPE_TEXT};

    // 调用通用插入函数
    return insert_data_to_db(USER_DB, sql, bindings, types, 9);
}

/**************************** 单条数据插入结束 ****************************/

/**************************** 单条数据删除开始 ****************************/

/**
 * @brief 删除用户数据库中指定ID的用户数据
 *
 * @param user_id 要删除的用户的唯一ID（UUID）
 * @return int 函数执行成功返回0，否则返回1
 */
int del_user_data(const char *user_id)
{
    FILE *log_file = fopen(LOG_FILE, "a"); // 'a' 表示附加模式
    if (log_file == NULL)
    {
        printf("无法打开日志文件：%s\n", strerror(errno));
        return 1;
    }

    sqlite3 *db;
    sqlite3_stmt *stmt;
    int rc;
    char current_time[20];
    const char *sql = "DELETE FROM users WHERE id = ?;";

    // 打开数据库
    rc = sqlite3_open(USER_DB, &db);
    if (rc != SQLITE_OK)
    {
        get_current_time(current_time, sizeof(current_time));
        fprintf(log_file, "%s [%s]: 无法打开数据库 '%s'：%s\n", current_time, LOGLEVEL_ERROR, USER_DB, sqlite3_errmsg(db));
        sqlite3_close(db);
        fclose(log_file);
        return 1;
    }

    // 准备SQL语句
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK)
    {
        get_current_time(current_time, sizeof(current_time));
        fprintf(log_file, "%s [%s]: 无法准备SQL语句：%s\n", current_time, LOGLEVEL_ERROR, sqlite3_errmsg(db));
        sqlite3_close(db);
        fclose(log_file);
        return 1;
    }

    // 绑定用户ID参数
    rc = sqlite3_bind_text(stmt, 1, user_id, -1, SQLITE_STATIC);
    if (rc != SQLITE_OK)
    {
        get_current_time(current_time, sizeof(current_time));
        fprintf(log_file, "%s [%s]: 绑定用户ID参数失败：%s\n", current_time, LOGLEVEL_ERROR, sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        fclose(log_file);
        return 1;
    }

    // 执行删除操作
    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE)
    {
        get_current_time(current_time, sizeof(current_time));
        fprintf(log_file, "%s [%s]: 删除用户数据失败：%s\n", current_time, LOGLEVEL_ERROR, sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        fclose(log_file);
        return 1;
    }
    else
    {
        get_current_time(current_time, sizeof(current_time));
        fprintf(log_file, "%s [%s]: 成功删除用户数据，用户ID：%s\n", current_time, LOGLEVEL_INFO, user_id);
    }

    // 清理和关闭数据库
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    fclose(log_file);
    return 0;
}

/**
 * @brief 删除考试数据库中指定ID的考试数据
 *
 * @param exam_id 要删除的考试的唯一ID（UUID）
 * @return int 函数执行成功返回0，否则返回1
 */
int del_exam_data(const char *exam_id)
{
    FILE *log_file = fopen(LOG_FILE, "a"); // 'a' 表示附加模式
    if (log_file == NULL)
    {
        printf("无法打开日志文件：%s\n", strerror(errno));
        return 1;
    }

    sqlite3 *db;
    sqlite3_stmt *stmt;
    int rc;
    char current_time[20];
    const char *sql = "DELETE FROM examinations WHERE id = ?;";

    // 打开数据库
    rc = sqlite3_open(EXAMINATION_DB, &db);
    if (rc != SQLITE_OK)
    {
        get_current_time(current_time, sizeof(current_time));
        fprintf(log_file, "%s [%s]: 无法打开数据库 '%s'：%s\n", current_time, LOGLEVEL_ERROR, EXAMINATION_DB, sqlite3_errmsg(db));
        sqlite3_close(db);
        fclose(log_file);
        return 1;
    }

    // 准备SQL语句
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK)
    {
        get_current_time(current_time, sizeof(current_time));
        fprintf(log_file, "%s [%s]: 无法准备SQL语句：%s\n", current_time, LOGLEVEL_ERROR, sqlite3_errmsg(db));
        sqlite3_close(db);
        fclose(log_file);
        return 1;
    }

    // 绑定考试ID参数
    rc = sqlite3_bind_text(stmt, 1, exam_id, -1, SQLITE_STATIC);
    if (rc != SQLITE_OK)
    {
        get_current_time(current_time, sizeof(current_time));
        fprintf(log_file, "%s [%s]: 绑定考试ID参数失败：%s\n", current_time, LOGLEVEL_ERROR, sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        fclose(log_file);
        return 1;
    }

    // 执行删除操作
    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE)
    {
        get_current_time(current_time, sizeof(current_time));
        fprintf(log_file, "%s [%s]: 删除考试数据失败：%s\n", current_time, LOGLEVEL_ERROR, sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        fclose(log_file);
        return 1;
    }
    else
    {
        get_current_time(current_time, sizeof(current_time));
        fprintf(log_file, "%s [%s]: 成功删除考试数据，考试ID：%s\n", current_time, LOGLEVEL_INFO, exam_id);
    }

    // 清理和关闭数据库
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    fclose(log_file);
    return 0;
}

/**
 * @brief 删除成绩数据库中指定ID的成绩数据
 *
 * @param score_id 要删除的成绩的唯一ID（UUID）
 * @return int 函数执行成功返回0，否则返回1
 */
int del_score_data(const char *score_id)
{
    FILE *log_file = fopen(LOG_FILE, "a"); // 'a' 表示附加模式
    if (log_file == NULL)
    {
        printf("无法打开日志文件：%s\n", strerror(errno));
        return 1;
    }

    sqlite3 *db;
    sqlite3_stmt *stmt;
    int rc;
    char current_time[20];
    const char *sql = "DELETE FROM scores WHERE id = ?;";

    // 打开数据库
    rc = sqlite3_open(SCORES_DB, &db);
    if (rc != SQLITE_OK)
    {
        get_current_time(current_time, sizeof(current_time));
        fprintf(log_file, "%s [%s]: 无法打开数据库 '%s'：%s\n", current_time, LOGLEVEL_ERROR, SCORES_DB, sqlite3_errmsg(db));
        sqlite3_close(db);
        fclose(log_file);
        return 1;
    }

    // 准备SQL语句
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK)
    {
        get_current_time(current_time, sizeof(current_time));
        fprintf(log_file, "%s [%s]: 无法准备SQL语句：%s\n", current_time, LOGLEVEL_ERROR, sqlite3_errmsg(db));
        sqlite3_close(db);
        fclose(log_file);
        return 1;
    }

    // 绑定成绩ID参数
    rc = sqlite3_bind_text(stmt, 1, score_id, -1, SQLITE_STATIC);
    if (rc != SQLITE_OK)
    {
        get_current_time(current_time, sizeof(current_time));
        fprintf(log_file, "%s [%s]: 绑定成绩ID参数失败：%s\n", current_time, LOGLEVEL_ERROR, sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        fclose(log_file);
        return 1;
    }

    // 执行删除操作
    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE)
    {
        get_current_time(current_time, sizeof(current_time));
        fprintf(log_file, "%s [%s]: 删除成绩数据失败：%s\n", current_time, LOGLEVEL_ERROR, sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        fclose(log_file);
        return 1;
    }
    else
    {
        get_current_time(current_time, sizeof(current_time));
        fprintf(log_file, "%s [%s]: 成功删除成绩数据，成绩ID：%s\n", current_time, LOGLEVEL_INFO, score_id);
    }

    // 清理和关闭数据库
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    fclose(log_file);
    return 0;
}

/**
 * @brief 删除问题数据库中指定ID的问题数据
 *
 * @param question_id 要删除的问题的唯一ID（UUID）
 * @return int 函数执行成功返回0，否则返回1
 */
int del_question_data(const char *question_id)
{
    FILE *log_file = fopen(LOG_FILE, "a"); // 'a' 表示附加模式
    if (log_file == NULL)
    {
        printf("无法打开日志文件：%s\n", strerror(errno));
        return 1;
    }

    sqlite3 *db;
    sqlite3_stmt *stmt;
    int rc;
    char current_time[20];
    const char *sql = "DELETE FROM questions WHERE id = ?;";

    // 打开数据库
    rc = sqlite3_open(EXAMINATION_DB, &db);
    if (rc != SQLITE_OK)
    {
        get_current_time(current_time, sizeof(current_time));
        fprintf(log_file, "%s [%s]: 无法打开数据库 '%s'：%s\n", current_time, LOGLEVEL_ERROR, EXAMINATION_DB, sqlite3_errmsg(db));
        sqlite3_close(db);
        fclose(log_file);
        return 1;
    }

    // 准备SQL语句
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK)
    {
        get_current_time(current_time, sizeof(current_time));
        fprintf(log_file, "%s [%s]: 无法准备SQL语句：%s\n", current_time, LOGLEVEL_ERROR, sqlite3_errmsg(db));
        sqlite3_close(db);
        fclose(log_file);
        return 1;
    }

    // 绑定问题ID参数
    rc = sqlite3_bind_text(stmt, 1, question_id, -1, SQLITE_STATIC);
    if (rc != SQLITE_OK)
    {
        get_current_time(current_time, sizeof(current_time));
        fprintf(log_file, "%s [%s]: 绑定问题ID参数失败：%s\n", current_time, LOGLEVEL_ERROR, sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        fclose(log_file);
        return 1;
    }

    // 执行删除操作
    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE)
    {
        get_current_time(current_time, sizeof(current_time));
        fprintf(log_file, "%s [%s]: 删除问题数据失败：%s\n", current_time, LOGLEVEL_ERROR, sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        fclose(log_file);
        return 1;
    }
    else
    {
        get_current_time(current_time, sizeof(current_time));
        fprintf(log_file, "%s [%s]: 成功删除问题数据，问题ID：%s\n", current_time, LOGLEVEL_INFO, question_id);
    }

    // 清理和关闭数据库
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    fclose(log_file);
    return 0;
}

/**************************** 单条数据删除结束 ****************************/

/**************************** 单条数据修改开始 ****************************/

/**
 * @brief 修改用户数据库中指定ID的用户数据
 *
 * @param user_id 要修改的用户的唯一ID（UUID）
 * @param username 新的用户名，范围为[a-zA-Z0-9]{3,24}
 * @param hashpass 新的密码哈希值，sha512(salt + passwd)
 * @param salt 新的盐值，16字节
 * @param role 新的用户角色
 * @param name 新的真实姓名
 * @param class_name 新的班级名称（可为空）
 * @param number 新的学号/工号
 * @param belong_to 新的归属教师ID（UUID，可为空）
 * @return int 函数执行成功返回0，否则返回1
 */
int edit_user_data(const char *user_id, const char *username, const char *hashpass, const char *salt, int role, const char *name, const char *class_name, int number, const char *belong_to)
{
    FILE *log_file = fopen(LOG_FILE, "a"); // 'a' 表示附加模式
    if (log_file == NULL)
    {
        printf("无法打开日志文件：%s\n", strerror(errno));
        return 1;
    }

    sqlite3 *db;
    sqlite3_stmt *stmt;
    int rc;
    char current_time[20];
    const char *sql = "UPDATE users SET username = ?, hashpass = ?, salt = ?, role = ?, name = ?, class_name = ?, number = ?, belong_to = ? WHERE id = ?;";

    // 打开数据库
    rc = sqlite3_open(USER_DB, &db);
    if (rc != SQLITE_OK)
    {
        get_current_time(current_time, sizeof(current_time));
        fprintf(log_file, "%s [%s]: 无法打开数据库 '%s'：%s\n", current_time, LOGLEVEL_ERROR, USER_DB, sqlite3_errmsg(db));
        sqlite3_close(db);
        fclose(log_file);
        return 1;
    }

    // 准备SQL语句
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK)
    {
        get_current_time(current_time, sizeof(current_time));
        fprintf(log_file, "%s [%s]: 无法准备SQL语句：%s\n", current_time, LOGLEVEL_ERROR, sqlite3_errmsg(db));
        sqlite3_close(db);
        fclose(log_file);
        return 1;
    }

    // 绑定参数
    rc = sqlite3_bind_text(stmt, 1, username, -1, SQLITE_STATIC);
    if (rc != SQLITE_OK)
    {
        get_current_time(current_time, sizeof(current_time));
        fprintf(log_file, "%s [%s]: 绑定用户名参数失败：%s\n", current_time, LOGLEVEL_ERROR, sqlite3_errmsg(db));
        goto cleanup;
    }

    rc = sqlite3_bind_text(stmt, 2, hashpass, -1, SQLITE_STATIC);
    if (rc != SQLITE_OK)
    {
        get_current_time(current_time, sizeof(current_time));
        fprintf(log_file, "%s [%s]: 绑定密码哈希参数失败：%s\n", current_time, LOGLEVEL_ERROR, sqlite3_errmsg(db));
        goto cleanup;
    }

    rc = sqlite3_bind_text(stmt, 3, salt, -1, SQLITE_STATIC);
    if (rc != SQLITE_OK)
    {
        get_current_time(current_time, sizeof(current_time));
        fprintf(log_file, "%s [%s]: 绑定盐值参数失败：%s\n", current_time, LOGLEVEL_ERROR, sqlite3_errmsg(db));
        goto cleanup;
    }

    rc = sqlite3_bind_text(stmt, 4, (const char *)&role, -1, SQLITE_STATIC); // 建议使用 sqlite3_bind_int
    if (rc != SQLITE_OK)
    {
        get_current_time(current_time, sizeof(current_time));
        fprintf(log_file, "%s [%s]: 绑定角色参数失败：%s\n", current_time, LOGLEVEL_ERROR, sqlite3_errmsg(db));
        goto cleanup;
    }

    rc = sqlite3_bind_text(stmt, 5, name, -1, SQLITE_STATIC);
    if (rc != SQLITE_OK)
    {
        get_current_time(current_time, sizeof(current_time));
        fprintf(log_file, "%s [%s]: 绑定姓名参数失败：%s\n", current_time, LOGLEVEL_ERROR, sqlite3_errmsg(db));
        goto cleanup;
    }

    rc = sqlite3_bind_text(stmt, 6, class_name, -1, SQLITE_STATIC);
    if (rc != SQLITE_OK)
    {
        get_current_time(current_time, sizeof(current_time));
        fprintf(log_file, "%s [%s]: 绑定班级名称参数失败：%s\n", current_time, LOGLEVEL_ERROR, sqlite3_errmsg(db));
        goto cleanup;
    }

    rc = sqlite3_bind_text(stmt, 7, (const char *)&number, -1, SQLITE_STATIC); // 建议使用 sqlite3_bind_int
    if (rc != SQLITE_OK)
    {
        get_current_time(current_time, sizeof(current_time));
        fprintf(log_file, "%s [%s]: 绑定学号/工号参数失败：%s\n", current_time, LOGLEVEL_ERROR, sqlite3_errmsg(db));
        goto cleanup;
    }

    rc = sqlite3_bind_text(stmt, 8, belong_to, -1, SQLITE_STATIC);
    if (rc != SQLITE_OK)
    {
        get_current_time(current_time, sizeof(current_time));
        fprintf(log_file, "%s [%s]: 绑定归属教师参数失败：%s\n", current_time, LOGLEVEL_ERROR, sqlite3_errmsg(db));
        goto cleanup;
    }

    rc = sqlite3_bind_text(stmt, 9, user_id, -1, SQLITE_STATIC);
    if (rc != SQLITE_OK)
    {
        get_current_time(current_time, sizeof(current_time));
        fprintf(log_file, "%s [%s]: 绑定用户ID参数失败：%s\n", current_time, LOGLEVEL_ERROR, sqlite3_errmsg(db));
        goto cleanup;
    }

    // 执行更新操作
    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE)
    {
        get_current_time(current_time, sizeof(current_time));
        fprintf(log_file, "%s [%s]: 更新用户数据失败：%s\n", current_time, LOGLEVEL_ERROR, sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        fclose(log_file);
        return 1;
    }
    else
    {
        get_current_time(current_time, sizeof(current_time));
        fprintf(log_file, "%s [%s]: 成功更新用户数据，用户ID：%s\n", current_time, LOGLEVEL_INFO, user_id);
    }

cleanup:
    // 清理和关闭数据库
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    fclose(log_file);
    return (rc == SQLITE_DONE) ? 0 : 1;
}

/**
 * @brief 修改考试数据库中指定ID的考试数据
 *
 * @param exam_id 要修改的考试的唯一ID（UUID）
 * @param name 新的考试名称
 * @param start_time 新的开始时间（时间戳）
 * @param end_time 新的结束时间（时间戳）
 * @param allow_answer_when_expired 是否允许逾期作答（0或1）
 * @param random_question 是否开启问题乱序（0或1）
 * @return int 函数执行成功返回0，否则返回1
 */
int edit_exam_data(const char *exam_id, const char *name, int start_time, int end_time, int allow_answer_when_expired, int random_question)
{
    FILE *log_file = fopen(LOG_FILE, "a"); // 'a' 表示附加模式
    if (log_file == NULL)
    {
        printf("无法打开日志文件：%s\n", strerror(errno));
        return 1;
    }

    sqlite3 *db;
    sqlite3_stmt *stmt;
    int rc;
    char current_time[20];
    const char *sql = "UPDATE examinations SET name = ?, start_time = ?, end_time = ?, allow_answer_when_expired = ?, random_question = ? WHERE id = ?;";

    // 打开数据库
    rc = sqlite3_open(EXAMINATION_DB, &db);
    if (rc != SQLITE_OK)
    {
        get_current_time(current_time, sizeof(current_time));
        fprintf(log_file, "%s [%s]: 无法打开数据库 '%s'：%s\n", current_time, LOGLEVEL_ERROR, EXAMINATION_DB, sqlite3_errmsg(db));
        sqlite3_close(db);
        fclose(log_file);
        return 1;
    }

    // 准备SQL语句
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK)
    {
        get_current_time(current_time, sizeof(current_time));
        fprintf(log_file, "%s [%s]: 无法准备SQL语句：%s\n", current_time, LOGLEVEL_ERROR, sqlite3_errmsg(db));
        sqlite3_close(db);
        fclose(log_file);
        return 1;
    }

    // 绑定参数
    rc = sqlite3_bind_text(stmt, 1, name, -1, SQLITE_STATIC);
    if (rc != SQLITE_OK)
    {
        get_current_time(current_time, sizeof(current_time));
        fprintf(log_file, "%s [%s]: 绑定考试名称参数失败：%s\n", current_time, LOGLEVEL_ERROR, sqlite3_errmsg(db));
        goto cleanup;
    }

    rc = sqlite3_bind_int(stmt, 2, start_time);
    if (rc != SQLITE_OK)
    {
        get_current_time(current_time, sizeof(current_time));
        fprintf(log_file, "%s [%s]: 绑定开始时间参数失败：%s\n", current_time, LOGLEVEL_ERROR, sqlite3_errmsg(db));
        goto cleanup;
    }

    rc = sqlite3_bind_int(stmt, 3, end_time);
    if (rc != SQLITE_OK)
    {
        get_current_time(current_time, sizeof(current_time));
        fprintf(log_file, "%s [%s]: 绑定结束时间参数失败：%s\n", current_time, LOGLEVEL_ERROR, sqlite3_errmsg(db));
        goto cleanup;
    }

    rc = sqlite3_bind_int(stmt, 4, allow_answer_when_expired);
    if (rc != SQLITE_OK)
    {
        get_current_time(current_time, sizeof(current_time));
        fprintf(log_file, "%s [%s]: 绑定允许逾期作答参数失败：%s\n", current_time, LOGLEVEL_ERROR, sqlite3_errmsg(db));
        goto cleanup;
    }

    rc = sqlite3_bind_int(stmt, 5, random_question);
    if (rc != SQLITE_OK)
    {
        get_current_time(current_time, sizeof(current_time));
        fprintf(log_file, "%s [%s]: 绑定问题乱序参数失败：%s\n", current_time, LOGLEVEL_ERROR, sqlite3_errmsg(db));
        goto cleanup;
    }

    rc = sqlite3_bind_text(stmt, 6, exam_id, -1, SQLITE_STATIC);
    if (rc != SQLITE_OK)
    {
        get_current_time(current_time, sizeof(current_time));
        fprintf(log_file, "%s [%s]: 绑定考试ID参数失败：%s\n", current_time, LOGLEVEL_ERROR, sqlite3_errmsg(db));
        goto cleanup;
    }

    // 执行更新操作
    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE)
    {
        get_current_time(current_time, sizeof(current_time));
        fprintf(log_file, "%s [%s]: 更新考试数据失败：%s\n", current_time, LOGLEVEL_ERROR, sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        fclose(log_file);
        return 1;
    }
    else
    {
        get_current_time(current_time, sizeof(current_time));
        fprintf(log_file, "%s [%s]: 成功更新考试数据，考试ID：%s\n", current_time, LOGLEVEL_INFO, exam_id);
    }

cleanup:
    // 清理和关闭数据库
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    fclose(log_file);
    return (rc == SQLITE_DONE) ? 0 : 1;
}

/**
 * @brief 修改成绩数据库中指定ID的成绩数据
 *
 * @param score_id 要修改的成绩的唯一ID（UUID）
 * @param exam_id 新的考试ID（UUID）
 * @param user_id 新的用户ID（UUID）
 * @param score 新的成绩值
 * @param expired_flag 是否逾期作答（0或1）
 * @return int 函数执行成功返回0，否则返回1
 */
int edit_score_data(const char *score_id, const char *exam_id, const char *user_id, int score, int expired_flag)
{
    FILE *log_file = fopen(LOG_FILE, "a"); // 'a' 表示附加模式
    if (log_file == NULL)
    {
        printf("无法打开日志文件：%s\n", strerror(errno));
        return 1;
    }

    sqlite3 *db;
    sqlite3_stmt *stmt;
    int rc;
    char current_time[20];
    const char *sql = "UPDATE scores SET exam_id = ?, user_id = ?, score = ?, expired_flag = ? WHERE id = ?;";

    // 打开数据库
    rc = sqlite3_open(SCORES_DB, &db);
    if (rc != SQLITE_OK)
    {
        get_current_time(current_time, sizeof(current_time));
        fprintf(log_file, "%s [%s]: 无法打开数据库 '%s'：%s\n", current_time, LOGLEVEL_ERROR, SCORES_DB, sqlite3_errmsg(db));
        sqlite3_close(db);
        fclose(log_file);
        return 1;
    }

    // 准备SQL语句
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK)
    {
        get_current_time(current_time, sizeof(current_time));
        fprintf(log_file, "%s [%s]: 无法准备SQL语句：%s\n", current_time, LOGLEVEL_ERROR, sqlite3_errmsg(db));
        sqlite3_close(db);
        fclose(log_file);
        return 1;
    }

    // 绑定参数
    rc = sqlite3_bind_text(stmt, 1, exam_id, -1, SQLITE_STATIC);
    if (rc != SQLITE_OK)
    {
        get_current_time(current_time, sizeof(current_time));
        fprintf(log_file, "%s [%s]: 绑定考试ID参数失败：%s\n", current_time, LOGLEVEL_ERROR, sqlite3_errmsg(db));
        goto cleanup;
    }

    rc = sqlite3_bind_text(stmt, 2, user_id, -1, SQLITE_STATIC);
    if (rc != SQLITE_OK)
    {
        get_current_time(current_time, sizeof(current_time));
        fprintf(log_file, "%s [%s]: 绑定用户ID参数失败：%s\n", current_time, LOGLEVEL_ERROR, sqlite3_errmsg(db));
        goto cleanup;
    }

    rc = sqlite3_bind_int(stmt, 3, score);
    if (rc != SQLITE_OK)
    {
        get_current_time(current_time, sizeof(current_time));
        fprintf(log_file, "%s [%s]: 绑定成绩参数失败：%s\n", current_time, LOGLEVEL_ERROR, sqlite3_errmsg(db));
        goto cleanup;
    }

    rc = sqlite3_bind_int(stmt, 4, expired_flag);
    if (rc != SQLITE_OK)
    {
        get_current_time(current_time, sizeof(current_time));
        fprintf(log_file, "%s [%s]: 绑定逾期标志参数失败：%s\n", current_time, LOGLEVEL_ERROR, sqlite3_errmsg(db));
        goto cleanup;
    }

    rc = sqlite3_bind_text(stmt, 5, score_id, -1, SQLITE_STATIC);
    if (rc != SQLITE_OK)
    {
        get_current_time(current_time, sizeof(current_time));
        fprintf(log_file, "%s [%s]: 绑定成绩ID参数失败：%s\n", current_time, LOGLEVEL_ERROR, sqlite3_errmsg(db));
        goto cleanup;
    }

    // 执行更新操作
    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE)
    {
        get_current_time(current_time, sizeof(current_time));
        fprintf(log_file, "%s [%s]: 更新成绩数据失败：%s\n", current_time, LOGLEVEL_ERROR, sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        fclose(log_file);
        return 1;
    }
    else
    {
        get_current_time(current_time, sizeof(current_time));
        fprintf(log_file, "%s [%s]: 成功更新成绩数据，成绩ID：%s\n", current_time, LOGLEVEL_INFO, score_id);
    }

cleanup:
    // 清理和关闭数据库
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    fclose(log_file);
    return (rc == SQLITE_DONE) ? 0 : 1;
}

/**
 * @brief 修改问题数据库中指定ID的问题数据
 *
 * @param question_id 要修改的问题的唯一ID（UUID）
 * @param exam_id 新的考试ID（UUID）
 * @param num1 新的第一个操作数
 * @param op 新的运算符（0: +, 1: -, 2: *, 3: /）
 * @param num2 新的第二个操作数
 * @return int 函数执行成功返回0，否则返回1
 */
int edit_question_data(const char *question_id, const char *exam_id, int num1, int op, int num2)
{
    FILE *log_file = fopen(LOG_FILE, "a"); // 'a' 表示附加模式
    if (log_file == NULL)
    {
        printf("无法打开日志文件：%s\n", strerror(errno));
        return 1;
    }

    sqlite3 *db;
    sqlite3_stmt *stmt;
    int rc;
    char current_time[20];
    const char *sql = "UPDATE questions SET exam_id = ?, num1 = ?, op = ?, num2 = ? WHERE id = ?;";

    // 打开数据库
    rc = sqlite3_open(EXAMINATION_DB, &db);
    if (rc != SQLITE_OK)
    {
        get_current_time(current_time, sizeof(current_time));
        fprintf(log_file, "%s [%s]: 无法打开数据库 '%s'：%s\n", current_time, LOGLEVEL_ERROR, EXAMINATION_DB, sqlite3_errmsg(db));
        sqlite3_close(db);
        fclose(log_file);
        return 1;
    }

    // 准备SQL语句
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK)
    {
        get_current_time(current_time, sizeof(current_time));
        fprintf(log_file, "%s [%s]: 无法准备SQL语句：%s\n", current_time, LOGLEVEL_ERROR, sqlite3_errmsg(db));
        sqlite3_close(db);
        fclose(log_file);
        return 1;
    }

    // 绑定参数
    rc = sqlite3_bind_text(stmt, 1, exam_id, -1, SQLITE_STATIC);
    if (rc != SQLITE_OK)
    {
        get_current_time(current_time, sizeof(current_time));
        fprintf(log_file, "%s [%s]: 绑定考试ID参数失败：%s\n", current_time, LOGLEVEL_ERROR, sqlite3_errmsg(db));
        goto cleanup;
    }

    rc = sqlite3_bind_int(stmt, 2, num1);
    if (rc != SQLITE_OK)
    {
        get_current_time(current_time, sizeof(current_time));
        fprintf(log_file, "%s [%s]: 绑定第一个操作数参数失败：%s\n", current_time, LOGLEVEL_ERROR, sqlite3_errmsg(db));
        goto cleanup;
    }

    rc = sqlite3_bind_int(stmt, 3, op);
    if (rc != SQLITE_OK)
    {
        get_current_time(current_time, sizeof(current_time));
        fprintf(log_file, "%s [%s]: 绑定运算符参数失败：%s\n", current_time, LOGLEVEL_ERROR, sqlite3_errmsg(db));
        goto cleanup;
    }

    rc = sqlite3_bind_int(stmt, 4, num2);
    if (rc != SQLITE_OK)
    {
        get_current_time(current_time, sizeof(current_time));
        fprintf(log_file, "%s [%s]: 绑定第二个操作数参数失败：%s\n", current_time, LOGLEVEL_ERROR, sqlite3_errmsg(db));
        goto cleanup;
    }

    rc = sqlite3_bind_text(stmt, 5, question_id, -1, SQLITE_STATIC);
    if (rc != SQLITE_OK)
    {
        get_current_time(current_time, sizeof(current_time));
        fprintf(log_file, "%s [%s]: 绑定问题ID参数失败：%s\n", current_time, LOGLEVEL_ERROR, sqlite3_errmsg(db));
        goto cleanup;
    }

    // 执行更新操作
    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE)
    {
        get_current_time(current_time, sizeof(current_time));
        fprintf(log_file, "%s [%s]: 更新问题数据失败：%s\n", current_time, LOGLEVEL_ERROR, sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        fclose(log_file);
        return 1;
    }
    else
    {
        get_current_time(current_time, sizeof(current_time));
        fprintf(log_file, "%s [%s]: 成功更新问题数据，问题ID：%s\n", current_time, LOGLEVEL_INFO, question_id);
    }

cleanup:
    // 清理和关闭数据库
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    fclose(log_file);
    return (rc == SQLITE_DONE) ? 0 : 1;
}

/**************************** 单条数据修改结束 ****************************/

int main()
{
    SetConsoleOutputCP(65001);
    printf("===== 开始数据库操作测试 =====\n\n");
    system("pause");
    // 1. 插入新用户
    printf("1. 插入新用户...\n");
    printf("1.1 插入新教师用户...\n");
    insert_user_data("c21e94d5-20d8-44d0-8299-eeaf7339fb22", "teacher_1", "a7117a4b2f5bc0ab7b16bea3dd831dd33d78a49d91c89dfa522eb04e7bf9e7c4ec72e49091245af7e11efc49002962569427f10de2beeb62d11010106f7d7cf0", "LWaaGwm6envH9sGj", 1, "黄洋", "", 7725, "");
    insert_user_data("17acdda4-7b2f-48b7-956d-db59627e2d0a", "teacher_2", "f55a3d39a81358cf2a54091144dfaa287b1066a258ad968b86ed86cc9b197c45bd8b3dd4ac2e948a800a0f5e7e6529d2697c5280aa9d39fe419ba31a13d4af62", "JRYTiouBho6HxwNK", 1, "张芳", "", 6803, "");
    insert_user_data("63cc2618-5cc7-45cb-bcb8-456432a23691", "teacher_3", "1353d4d6441145b51a44b1d3c1ad0365a0b291139bffc51c6601bf4d694793643e04a7f1fde3658513e7dcd79bb0010df52be3376f5a794c2320a0e08910f722", "BNx5oY8m0XwDL3zd", 1, "王伟", "", 6921, "");

    printf("1.2 插入新学生用户\n");
    insert_user_data("eac40007-86f7-43cc-9cc0-5a863f35fe51", "user_9870", "66c510e9e8e10d6be747d39912e198dd7c0a4adc7d5234dfe64f762b3b804944ecf6b66802ccb481e5361bd806fb6ecb91ef035f9fc563a6db8a7fdb19d95fca", "rGQ4LuvwvE75bl12", 0, "周静", "数据科学", 3083067581, "17acdda4-7b2f-48b7-956d-db59627e2d0a");
    insert_user_data("f83415ca-9770-4405-b34e-7ad9cb3004b8", "user_7811", "63e891cdeb384b1f7ec807f75cb9d00d1295c622966c60cb73df45f0a500ab35fb5936eca45218fee45a529fb6f0dd1a04dfa4d129a8c9de3608f2c0c5b8bac7", "gYU3H09CIjVvp5oZ", 0, "张丽", "信息安全", 3576443143, "63cc2618-5cc7-45cb-bcb8-456432a23691");
    insert_user_data("3f92bd64-26f3-46d3-9d3e-3da068274c0f", "user_2931", "f748ee6ce2f37aeab3ebad20cd1d75db9395113174ed758075a3c99845e1c30908ef23d35cd123ac3c22a25db89a7b5dc8d14d548dec587169b72eb9d619c9df", "Qcf0WFBmnIvy6lrH", 0, "黄敏", "计算机科学与技术", 3890070435, "17acdda4-7b2f-48b7-956d-db59627e2d0a");
    insert_user_data("627cb285-9122-4460-8556-3e950e7142a9", "user_3830", "e6e092e789a93cc6b46679dc0fcceb208e03a6df915a683e5e315a2c3ad90c9268def0e54167b3e34160a7a490b3679a534780f59c48a2bcdc0ecfa1e026c1d8", "1CEy6CvXIrNRcjA4", 0, "杨芳", "软件工程", 3349463823, "c21e94d5-20d8-44d0-8299-eeaf7339fb22");
    insert_user_data("473537fe-a158-4e77-8ade-2a228f12d65d", "user_2054", "09ebadd95836e7972b1e3d91bb1945a9cac7b548e3b6d35483c3b054b4c50a6909b477a4d79d43f84bdeede6d2c3a06d1afc3216cd94f39a0df4c53a92f5c875", "zi9S0wsXOHJCioKI", 0, "陈强", "计算机科学与技术", 1518534718, "63cc2618-5cc7-45cb-bcb8-456432a23691");
    insert_user_data("f2a3820f-f919-41ea-b399-4f2186d9081a", "user_6856", "e95dfa7f57a4682e18900027f3f8d0d2cb1cbed878aa75c03f07bd632436526a00d68a9a8b51cee9a11d5e4222bc868a58101c9b976817f18a4ccfeb6ac90fd5", "NyrLFaqF6s4MqEvf", 0, "张强", "软件工程", 1538069630, "c21e94d5-20d8-44d0-8299-eeaf7339fb22");
    insert_user_data("082367d8-03b6-4701-82fc-8feff6e473de", "user_7884", "4319b47cfa5b8c2d7ae1141f8f32eebc707a1676c16bac6d900ed4960438888ce6ed26bcfb53bb143b675160f8f1525aa10d73d65a50ecf137da9b5af938eeff", "S4dvRoGoGz2CGmFg", 0, "王军", "数据科学", 1843619154, "17acdda4-7b2f-48b7-956d-db59627e2d0a");
    insert_user_data("f511634f-1851-45c3-b95c-b6ad6b858fb9", "user_1977", "320fb08db7a50b7f4aec86918195bc24845675cddf636ac177b2cbc46a844bfb3a56bf7ad3af02ec77aae59b6f7630531d4f8fb0e99489087a977a2d5a7a143b", "6pX06XZOH7tfgkqL", 0, "周洋", "人工智能", 2265324013, "63cc2618-5cc7-45cb-bcb8-456432a23691");
    insert_user_data("7e21b765-d05e-44da-86a3-2f4a00cf1960", "user_2182", "36c6f4817b4b084d8d7a7b2c980e3d77b0f3d69eb053711f13c91cc4d4ed4aea29cbff6f6e357e5af261d020d89d76ffddc58d030ccfa1d5d003ce73b14ba324", "TBr47A8YapvB8hwO", 0, "赵军", "人工智能", 2861294394, "c21e94d5-20d8-44d0-8299-eeaf7339fb22");
    insert_user_data("2b6ad28d-9ea8-4733-bce2-4719d0d2794f", "user_4367", "a4b20a6d54f9549736d73a9b9f4c07880dbaa137632c2fe507312937bbc505ee159b746beaff374f5897a4a5c41ecdc9239857493169ed1b3e02fe06b8ffba2d", "Bm94B6sUZkmwiwxF", 0, "杨军", "人工智能", 1001604851, "c21e94d5-20d8-44d0-8299-eeaf7339fb22");
    system("pause");
    // 2. 插入新考试
    printf("2. 插入新考试...\n");
    insert_exam_data("9136eeab-7e50-4dec-ada4-4fe27fc83f1b", "2月月考", 1719539996, 1719546803, 1, 1);
    insert_exam_data("2bf8d1cc-635f-437a-afae-d4e701a1c111", "2月月考", 1704628540, 1704635143, 0, 0);
    insert_exam_data("0bd088fe-c089-468f-85ca-6e4878b54640", "2月月考", 1727290933, 1727296696, 1, 1);
    insert_exam_data("d5712e69-c137-4a30-b997-fb1f744b665a", "1月月考", 1723457350, 1723461705, 0, 0);
    insert_exam_data("f46a21cf-55d6-428a-8275-e126318dad9d", "1月月考", 1716456045, 1716462385, 0, 1);
    system("pause");
    // 3. 插入新问题
    printf("3. 插入新问题...\n");
    insert_question_data("476952e8-3aa3-48eb-a808-76b6726b3ed0", "9136eeab-7e50-4dec-ada4-4fe27fc83f1b", 23.4f, 3, 18.9f);
    insert_question_data("02ae76a6-f256-4f5a-a29d-72760d093d21", "9136eeab-7e50-4dec-ada4-4fe27fc83f1b", 56.4f, 1, 70.0f);
    insert_question_data("a5433bf8-2929-4e8d-933a-fce98186f42a", "9136eeab-7e50-4dec-ada4-4fe27fc83f1b", 85.8f, 0, 15.8f);
    insert_question_data("cda3df9f-5809-41bb-96a7-8bd51646b804", "9136eeab-7e50-4dec-ada4-4fe27fc83f1b", 52.9f, 2, 58.0f);
    insert_question_data("49d4c343-2fbb-4c46-896d-c91ca188473e", "9136eeab-7e50-4dec-ada4-4fe27fc83f1b", 27.1f, 1, 36.2f);
    insert_question_data("9878828a-4e98-411a-b90c-640c2f1bef8a", "2bf8d1cc-635f-437a-afae-d4e701a1c111", 69.1f, 0, 17.6f);
    insert_question_data("2b5455f6-4715-4f01-98c1-ec68306ce8ca", "2bf8d1cc-635f-437a-afae-d4e701a1c111", 10.1f, 2, 88.6f);
    insert_question_data("c963b484-0e6f-40b5-af4e-b05b56158d07", "2bf8d1cc-635f-437a-afae-d4e701a1c111", 23.3f, 2, 50.2f);
    insert_question_data("c565781d-3532-4dd1-9497-f6d0bca11555", "2bf8d1cc-635f-437a-afae-d4e701a1c111", 77.9f, 0, 81.3f);
    insert_question_data("1f8506cb-21a5-41d1-98d8-301c4ab24ece", "2bf8d1cc-635f-437a-afae-d4e701a1c111", 92.3f, 3, 68.5f);
    insert_question_data("25d4ee44-9e6a-40e9-834a-91a8c1528ab7", "0bd088fe-c089-468f-85ca-6e4878b54640", 14.5f, 2, 26.5f);
    insert_question_data("021f87b6-6911-4b6d-a293-360351082d3e", "0bd088fe-c089-468f-85ca-6e4878b54640", 33.5f, 0, 20.3f);
    insert_question_data("af886fd9-8f3d-4f45-b5df-480bd3f68f96", "0bd088fe-c089-468f-85ca-6e4878b54640", 75.2f, 0, 2.1f);
    insert_question_data("68ee012e-c0ad-49ad-a232-354a7550743e", "0bd088fe-c089-468f-85ca-6e4878b54640", 19.7f, 0, 34.5f);
    insert_question_data("0e3bb333-102e-4896-98e1-e9249d990603", "0bd088fe-c089-468f-85ca-6e4878b54640", 50.8f, 0, 75.1f);
    insert_question_data("c6d9e5a3-6e17-4a83-afaf-6ba88186fa0d", "d5712e69-c137-4a30-b997-fb1f744b665a", 56.4f, 2, 22.6f);
    insert_question_data("ddc09ff7-b397-4417-8489-d7efdc107be2", "d5712e69-c137-4a30-b997-fb1f744b665a", 62.4f, 3, 68.9f);
    insert_question_data("b7e7acf2-f858-40af-b204-a581e7e0926c", "d5712e69-c137-4a30-b997-fb1f744b665a", 28.3f, 1, 34.2f);
    insert_question_data("31ff6d10-3fe3-4db6-a017-068ee509e66d", "d5712e69-c137-4a30-b997-fb1f744b665a", 67.2f, 2, 3.0f);
    insert_question_data("cd58a92a-88a1-4a1a-9afb-5d4dce44e652", "d5712e69-c137-4a30-b997-fb1f744b665a", 93.8f, 1, 86.4f);
    insert_question_data("dcb4450d-305e-446b-b58f-c829b9517d68", "f46a21cf-55d6-428a-8275-e126318dad9d", 69.1f, 0, 21.9f);
    insert_question_data("5c47a40e-ca6d-4d52-add0-407fa723f322", "f46a21cf-55d6-428a-8275-e126318dad9d", 92.5f, 2, 66.6f);
    insert_question_data("be322f9a-8d61-4993-9fbe-aaeed446d85c", "f46a21cf-55d6-428a-8275-e126318dad9d", 29.3f, 1, 28.4f);
    insert_question_data("a4fb3f8a-978e-4a07-b047-9f0bd76160f6", "f46a21cf-55d6-428a-8275-e126318dad9d", 91.5f, 2, 29.2f);
    insert_question_data("405a3580-6af9-45ae-867a-004e17c957f3", "f46a21cf-55d6-428a-8275-e126318dad9d", 93.5f, 3, 29.8f);
    system("pause");
    // 4. 插入新成绩
    printf("4. 插入新成绩...\n");
    insert_score_data("f9409255-3a9a-470a-aeea-9b2b89c7e5bb", "d5712e69-c137-4a30-b997-fb1f744b665a", "7e21b765-d05e-44da-86a3-2f4a00cf1960", 74.9f, 1);
    insert_score_data("930ed5ee-ae24-43ea-b7b1-8110420074da", "2bf8d1cc-635f-437a-afae-d4e701a1c111", "473537fe-a158-4e77-8ade-2a228f12d65d", 91.1f, 0);
    insert_score_data("eb9683cd-8950-421e-b925-76a0f827dba4", "9136eeab-7e50-4dec-ada4-4fe27fc83f1b", "f2a3820f-f919-41ea-b399-4f2186d9081a", 62.8f, 0);
    insert_score_data("a3add7ba-90d2-4ff0-8d20-5f994a87bc07", "d5712e69-c137-4a30-b997-fb1f744b665a", "082367d8-03b6-4701-82fc-8feff6e473de", 71.2f, 1);
    insert_score_data("7aa09a74-d311-457d-9f13-873c2c7d73fb", "9136eeab-7e50-4dec-ada4-4fe27fc83f1b", "f511634f-1851-45c3-b95c-b6ad6b858fb9", 69.9f, 1);
    insert_score_data("93b492cc-71b8-4a3a-87a8-838ae6a6a855", "2bf8d1cc-635f-437a-afae-d4e701a1c111", "082367d8-03b6-4701-82fc-8feff6e473de", 64.6f, 1);
    insert_score_data("cbf64d43-8004-4d87-8b59-e4a8e993dd9b", "2bf8d1cc-635f-437a-afae-d4e701a1c111", "eac40007-86f7-43cc-9cc0-5a863f35fe51", 70.5f, 0);
    insert_score_data("217f4516-5d5e-4a25-a3f2-2cdb3e2756e7", "f46a21cf-55d6-428a-8275-e126318dad9d", "082367d8-03b6-4701-82fc-8feff6e473de", 60.8f, 0);
    insert_score_data("12ddf76b-55e9-4e50-bf8e-2c8422b4fe6e", "0bd088fe-c089-468f-85ca-6e4878b54640", "2b6ad28d-9ea8-4733-bce2-4719d0d2794f", 79.1f, 1);
    insert_score_data("04bb7cff-4608-40b1-ba2c-e887d3671154", "9136eeab-7e50-4dec-ada4-4fe27fc83f1b", "7e21b765-d05e-44da-86a3-2f4a00cf1960", 55.7f, 1);
    insert_score_data("85dbbd48-e8d2-4103-9175-ba5e6a754118", "d5712e69-c137-4a30-b997-fb1f744b665a", "2b6ad28d-9ea8-4733-bce2-4719d0d2794f", 93.8f, 1);
    insert_score_data("d8e23757-62d2-4446-94bc-94cb4bc2ff71", "d5712e69-c137-4a30-b997-fb1f744b665a", "eac40007-86f7-43cc-9cc0-5a863f35fe51", 91.7f, 1);
    insert_score_data("2fb0cd79-bdaf-485e-8515-d15c9e8ddf90", "f46a21cf-55d6-428a-8275-e126318dad9d", "f83415ca-9770-4405-b34e-7ad9cb3004b8", 57.7f, 0);
    insert_score_data("6d5471e0-dd9c-47cc-80f3-67d109a4cea3", "f46a21cf-55d6-428a-8275-e126318dad9d", "473537fe-a158-4e77-8ade-2a228f12d65d", 57.6f, 0);
    insert_score_data("7976b7e0-6722-4a28-9fac-c3df2dea4d11", "d5712e69-c137-4a30-b997-fb1f744b665a", "082367d8-03b6-4701-82fc-8feff6e473de", 68.7f, 1);
    system("pause");
    // 5. 查询单个用户
    printf("5. 查询单个用户...\n");
    struct User queried_user;
    if (query_user_info("id", "eac40007-86f7-43cc-9cc0-5a863f35fe51", &queried_user) == 0)
    {
        printf("查询到的用户信息：\n");
        printf("ID: %s\n", queried_user.id);
        printf("用户名: %s\n", queried_user.username);
        printf("角色: %d\n", queried_user.role);
        printf("姓名: %s\n", queried_user.name);
        printf("班级: %s\n", queried_user.class_name);
        printf("学号: %u\n", queried_user.number);
        printf("归属教师: %s\n", queried_user.belong_to);
        printf("权限：\n");
        printf("  答题权限: %d\n", queried_user.permission.stu_answer);
        printf("  查看个人信息权限: %d\n", queried_user.permission.stu_inspect_personal_info);
        printf("  查看考试信息权限: %d\n", queried_user.permission.stu_inspect_exam_info);
        printf("  管理考试权限: %d\n", queried_user.permission.tea_manage_exam);
        printf("  管理学生权限: %d\n", queried_user.permission.tea_manage_student);
        printf("  查看学生信息权限: %d\n", queried_user.permission.tea_inspect_student_info);
        printf("  查看成绩单权限: %d\n", queried_user.permission.tea_inspect_exam_scores);
        printf("  修改个人信息权限: %d\n\n", queried_user.permission.general_edit_info);
    }
    else
    {
        printf("查询用户失败。\n\n");
    }
    system("pause");
    // 6. 查询所有用户
    printf("6. 查询所有用户...\n");
    struct User all_users[10];
    if (query_users_info_all("张%%", "计算机%%", 0, "%%", "%%", all_users, 10) == 0)
    {
        printf("查询到的用户列表：\n");
        for (int i = 0; i < 10 && all_users[i].id[0] != '\0'; i++)
        {
            printf("用户 %d:\n", i + 1);
            printf("  ID: %s\n", all_users[i].id);
            printf("  用户名: %s\n", all_users[i].username);
            printf("  角色: %d\n", all_users[i].role);
            printf("  姓名: %s\n", all_users[i].name);
            printf("  班级: %s\n", all_users[i].class_name);
            printf("  学号: %u\n", all_users[i].number);
            printf("  归属教师: %s\n", all_users[i].belong_to);
            printf("  权限：\n");
            printf("    答题权限: %d\n", all_users[i].permission.stu_answer);
            printf("    查看个人信息权限: %d\n", all_users[i].permission.stu_inspect_personal_info);
            printf("    查看考试信息权限: %d\n", all_users[i].permission.stu_inspect_exam_info);
            printf("    管理考试权限: %d\n", all_users[i].permission.tea_manage_exam);
            printf("    管理学生权限: %d\n", all_users[i].permission.tea_manage_student);
            printf("    查看学生信息权限: %d\n", all_users[i].permission.tea_inspect_student_info);
            printf("    查看成绩单权限: %d\n", all_users[i].permission.tea_inspect_exam_scores);
            printf("    修改个人信息权限: %d\n\n", all_users[i].permission.general_edit_info);
        }
    }
    else
    {
        printf("查询所有用户失败。\n\n");
    }
    system("pause");
    // 7. 修改用户数据
    printf("7. 修改用户数据...\n");
    if (edit_user_data(
            "eac40007-86f7-43cc-9cc0-5a863f35fe51",
            "john_doe_updated",
            "new_hashed_password",
            "new_salt",
            1, // 修改为教师
            "李四",
            "软件工程",
            3124005678,
            "17acdda4-7b2f-48b7-956d-db59627e2d0a") == 0)
    {
        printf("用户数据修改成功。\n\n");
    }
    else
    {
        printf("用户数据修改失败。\n\n");
    }
    system("pause");
    // 8. 查询修改后的用户
    printf("8. 查询修改后的用户...\n");
    if (query_user_info("id", "eac40007-86f7-43cc-9cc0-5a863f35fe51", &queried_user) == 0)
    {
        printf("修改后的用户信息：\n");
        printf("ID: %s\n", queried_user.id);
        printf("用户名: %s\n", queried_user.username);
        printf("角色: %d\n", queried_user.role);
        printf("姓名: %s\n", queried_user.name);
        printf("班级: %s\n", queried_user.class_name);
        printf("学号: %u\n", queried_user.number);
        printf("归属教师: %s\n", queried_user.belong_to);
        printf("权限：\n");
        printf("  答题权限: %d\n", queried_user.permission.stu_answer);
        printf("  查看个人信息权限: %d\n", queried_user.permission.stu_inspect_personal_info);
        printf("  查看考试信息权限: %d\n", queried_user.permission.stu_inspect_exam_info);
        printf("  管理考试权限: %d\n", queried_user.permission.tea_manage_exam);
        printf("  管理学生权限: %d\n", queried_user.permission.tea_manage_student);
        printf("  查看学生信息权限: %d\n", queried_user.permission.tea_inspect_student_info);
        printf("  查看成绩单权限: %d\n", queried_user.permission.tea_inspect_exam_scores);
        printf("  修改个人信息权限: %d\n\n", queried_user.permission.general_edit_info);
    }
    else
    {
        printf("查询修改后的用户失败。\n\n");
    }
    system("pause");
    // 9. 删除用户数据
    printf("9. 删除用户数据...\n");
    if (del_user_data("eac40007-86f7-43cc-9cc0-5a863f35fe51") == 0)
    {
        printf("用户删除成功。\n\n");
    }
    else
    {
        printf("用户删除失败。\n\n");
    }
    system("pause");
    // 10. 查询删除后的用户
    printf("10. 查询删除后的用户...\n");
    if (query_user_info("id", "eac40007-86f7-43cc-9cc0-5a863f35fe51", &queried_user) == 0)
    {
        if (queried_user.id[0] != '\0')
        {
            printf("删除后的用户信息仍存在：\n");
            printf("ID: %s\n", queried_user.id);
            // 其他信息省略
        }
        else
        {
            printf("用户已成功删除，未找到相关信息。\n\n");
        }
    }
    else
    {
        printf("查询删除后的用户失败。\n\n");
    }
    system("pause");
    // 11. 查询单个考试
    printf("11. 查询单个考试...\n");
    struct SqlResponseExam queried_exam;
    if (query_exam_info("id", "9136eeab-7e50-4dec-ada4-4fe27fc83f1b", &queried_exam) == 0)
    {
        printf("查询到的考试信息：\n");
        printf("ID: %s\n", queried_exam.id);
        printf("名称: %s\n", queried_exam.name);
        printf("开始时间: %u\n", queried_exam.start_time);
        printf("结束时间: %u\n", queried_exam.end_time);
        printf("允许逾期作答: %d\n", queried_exam.allow_answer_when_expired);
        printf("随机问题顺序: %d\n\n", queried_exam.random_question);
    }
    else
    {
        printf("查询考试失败。\n\n");
    }
    system("pause");
    // 12. 查询所有考试
    printf("12. 查询所有考试...\n");
    struct SqlResponseExam all_exams[10];
    if (query_exams_info_all("2月%%", 1704620000, 1728000000, all_exams, 10) == 0)
    {
        printf("查询到的考试列表：\n");
        for (int i = 0; i < 10 && all_exams[i].id[0] != '\0'; i++)
        {
            printf("考试 %d:\n", i + 1);
            printf("  ID: %s\n", all_exams[i].id);
            printf("  名称: %s\n", all_exams[i].name);
            printf("  开始时间: %u\n", all_exams[i].start_time);
            printf("  结束时间: %u\n", all_exams[i].end_time);
            printf("  允许逾期作答: %d\n", all_exams[i].allow_answer_when_expired);
            printf("  随机问题顺序: %d\n\n", all_exams[i].random_question);
        }
    }
    else
    {
        printf("查询所有考试失败。\n\n");
    }
    system("pause");
    // 13. 修改考试数据
    printf("13. 修改考试数据...\n");
    if (edit_exam_data(
            "9136eeab-7e50-4dec-ada4-4fe27fc83f1b",
            "2月月考 - 更新版",
            1719539996,
            1719546803,
            0, // 不允许逾期作答
            0  // 关闭随机问题顺序
            ) == 0)
    {
        printf("考试数据修改成功。\n\n");
    }
    else
    {
        printf("考试数据修改失败。\n\n");
    }
    system("pause");
    // 14. 查询修改后的考试
    printf("14. 查询修改后的考试...\n");
    if (query_exam_info("id", "9136eeab-7e50-4dec-ada4-4fe27fc83f1b", &queried_exam) == 0)
    {
        printf("修改后的考试信息：\n");
        printf("ID: %s\n", queried_exam.id);
        printf("名称: %s\n", queried_exam.name);
        printf("开始时间: %u\n", queried_exam.start_time);
        printf("结束时间: %u\n", queried_exam.end_time);
        printf("允许逾期作答: %d\n", queried_exam.allow_answer_when_expired);
        printf("随机问题顺序: %d\n\n", queried_exam.random_question);
    }
    else
    {
        printf("查询修改后的考试失败。\n\n");
    }
    system("pause");
    // 15. 删除考试数据
    printf("15. 删除考试数据...\n");
    if (del_exam_data("9136eeab-7e50-4dec-ada4-4fe27fc83f1b") == 0)
    {
        printf("考试删除成功。\n\n");
    }
    else
    {
        printf("考试删除失败。\n\n");
    }
    system("pause");
    // 16. 查询删除后的考试
    printf("16. 查询删除后的考试...\n");
    if (query_exam_info("id", "9136eeab-7e50-4dec-ada4-4fe27fc83f1b", &queried_exam) == 0)
    {
        if (queried_exam.id[0] != '\0')
        {
            printf("删除后的考试信息仍存在：\n");
            printf("ID: %s\n", queried_exam.id);
            // 其他信息省略
        }
        else
        {
            printf("考试已成功删除，未找到相关信息。\n\n");
        }
    }
    else
    {
        printf("查询删除后的考试失败。\n\n");
    }
    system("pause");
    // 17. 查询单个问题
    printf("17. 查询单个问题...\n");
    struct SqlResponseQuestion queried_question;
    if (query_question_info("id", "476952e8-3aa3-48eb-a808-76b6726b3ed0", &queried_question) == 0)
    {
        printf("查询到的问题信息：\n");
        printf("ID: %s\n", queried_question.id);
        printf("考试ID: %s\n", queried_question.exam_id);
        printf("第一个操作数: %.2f\n", queried_question.num1);
        printf("运算符: %d\n", queried_question.op);
        printf("第二个操作数: %.2f\n\n", queried_question.num2);
    }
    else
    {
        printf("查询问题失败。\n\n");
    }
    system("pause");
    // 18. 查询所有问题
    printf("18. 查询所有问题...\n");
    struct SqlResponseQuestion all_questions[10];
    if (query_questions_info_all("2bf8d1cc-635f-437a-afae-d4e701a1c111", all_questions, 10) == 0)
    {
        printf("查询到的问题列表：\n");
        for (int i = 0; i < 10 && all_questions[i].id[0] != '\0'; i++)
        {
            printf("问题 %d:\n", i + 1);
            printf("  ID: %s\n", all_questions[i].id);
            printf("  考试ID: %s\n", all_questions[i].exam_id);
            printf("  第一个操作数: %.2f\n", all_questions[i].num1);
            printf("  运算符: %d\n", all_questions[i].op);
            printf("  第二个操作数: %.2f\n\n", all_questions[i].num2);
        }
    }
    else
    {
        printf("查询所有问题失败。\n\n");
    }
    system("pause");
    // 19. 修改问题数据
    printf("19. 修改问题数据...\n");
    if (edit_question_data(
            "476952e8-3aa3-48eb-a808-76b6726b3ed0",
            "2bf8d1cc-635f-437a-afae-d4e701a1c111",
            60.5f, // 新的第一个操作数
            0,     // 修改为运算符 '+'
            70.3f  // 新的第二个操作数
            ) == 0)
    {
        printf("问题数据修改成功。\n\n");
    }
    else
    {
        printf("问题数据修改失败。\n\n");
    }
    system("pause");
    // 20. 查询修改后的问题
    printf("20. 查询修改后的问题...\n");
    if (query_question_info("id", "476952e8-3aa3-48eb-a808-76b6726b3ed0", &queried_question) == 0)
    {
        printf("修改后的问题信息：\n");
        printf("ID: %s\n", queried_question.id);
        printf("考试ID: %s\n", queried_question.exam_id);
        printf("第一个操作数: %.2f\n", queried_question.num1);
        printf("运算符: %d\n", queried_question.op);
        printf("第二个操作数: %.2f\n\n", queried_question.num2);
    }
    else
    {
        printf("查询修改后的问题失败。\n\n");
    }
    system("pause");
    // 21. 删除问题数据
    printf("21. 删除问题数据...\n");
    if (del_question_data("476952e8-3aa3-48eb-a808-76b6726b3ed0") == 0)
    {
        printf("问题删除成功。\n\n");
    }
    else
    {
        printf("问题删除失败。\n\n");
    }
    system("pause");
    // 22. 查询删除后的问题
    printf("22. 查询删除后的问题...\n");
    if (query_question_info("id", "476952e8-3aa3-48eb-a808-76b6726b3ed0", &queried_question) == 0)
    {
        if (queried_question.id[0] != '\0')
        {
            printf("删除后的问题信息仍存在：\n");
            printf("ID: %s\n", queried_question.id);
            // 其他信息省略
        }
        else
        {
            printf("问题已成功删除，未找到相关信息。\n\n");
        }
    }
    else
    {
        printf("查询删除后的问题失败。\n\n");
    }
    system("pause");
    // 23. 查询单个成绩
    printf("23. 查询单个成绩...\n");
    struct SqlResponseScore queried_score;
    if (query_score_info("d5712e69-c137-4a30-b997-fb1f744b665a", "7e21b765-d05e-44da-86a3-2f4a00cf1960", &queried_score) == 0)
    {
        printf("查询到的成绩信息：\n");
        printf("ID: %s\n", queried_score.id);
        printf("考试ID: %s\n", queried_score.exam_id);
        printf("用户ID: %s\n", queried_score.user_id);
        printf("分数: %.2f\n", queried_score.score);
        printf("逾期标志: %d\n\n", queried_score.expired_flag);
    }
    else
    {
        printf("查询成绩失败。\n\n");
    }
    system("pause");
    // 24. 查询所有成绩
    printf("24. 查询所有成绩...\n");
    struct SqlResponseScore all_scores[10];
    if (query_scores_info_all("f46a21cf-55d6-428a-8275-e126318dad9d", "f2a3820f-f919-41ea-b399-4f2186d9081a", all_scores, 10) == 0)
    {
        printf("查询到的成绩列表：\n");
        for (int i = 0; i < 10 && all_scores[i].id[0] != '\0'; i++)
        {
            printf("成绩 %d:\n", i + 1);
            printf("  ID: %s\n", all_scores[i].id);
            printf("  考试ID: %s\n", all_scores[i].exam_id);
            printf("  用户ID: %s\n", all_scores[i].user_id);
            printf("  分数: %.2f\n", all_scores[i].score);
            printf("  逾期标志: %d\n\n", all_scores[i].expired_flag);
        }
    }
    else
    {
        printf("查询所有成绩失败。\n\n");
    }
    system("pause");
    // 25. 修改成绩数据
    printf("25. 修改成绩数据...\n");
    if (edit_score_data(
            "f9409255-3a9a-470a-aeea-9b2b89c7e5bb",
            "4715c3de-3b3b-4283-8787-eaa4b9d419b1",
            "7e21b765-d05e-44da-86a3-2f4a00cf1960",
            75.0f, // 新的分数
            1      // 修改为逾期
            ) == 0)
    {
        printf("成绩数据修改成功。\n\n");
    }
    else
    {
        printf("成绩数据修改失败。\n\n");
    }
    system("pause");
    // 26. 查询修改后的成绩
    printf("26. 查询修改后的成绩...\n");
    if (query_score_info("4715c3de-3b3b-4283-8787-eaa4b9d419b1", "7e21b765-d05e-44da-86a3-2f4a00cf1960", &queried_score) == 0)
    {
        printf("修改后的成绩信息：\n");
        printf("ID: %s\n", queried_score.id);
        printf("考试ID: %s\n", queried_score.exam_id);
        printf("用户ID: %s\n", queried_score.user_id);
        printf("分数: %.2f\n", queried_score.score);
        printf("逾期标志: %d\n\n", queried_score.expired_flag);
    }
    else
    {
        printf("查询修改后的成绩失败。\n\n");
    }
    system("pause");
    // 27. 删除成绩数据
    printf("27. 删除成绩数据...\n");
    if (del_score_data("f9409255-3a9a-470a-aeea-9b2b89c7e5bb") == 0)
    {
        printf("成绩删除成功。\n\n");
    }
    else
    {
        printf("成绩删除失败。\n\n");
    }
    system("pause");
    // 28. 查询删除后的成绩
    printf("28. 查询删除后的成绩...\n");
    if (query_score_info("4715c3de-3b3b-4283-8787-eaa4b9d419b1", "7e21b765-d05e-44da-86a3-2f4a00cf1960", &queried_score) == 0)
    {
        if (queried_score.id[0] != '\0')
        {
            printf("删除后的成绩信息仍存在：\n");
            printf("ID: %s\n", queried_score.id);
            // 其他信息省略
        }
        else
        {
            printf("成绩已成功删除，未找到相关信息。\n\n");
        }
    }
    else
    {
        printf("查询删除后的成绩失败。\n\n");
    }
    printf("===== 数据库操作测试完成 =====\n");
    system("pause");
    return 0;
}