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
 */

#include <stdio.h>
#include <string.h>
#include "model.h"
#include "utils.h"
#include "../lib/sqlite3.h"

#define LOGLEVEL_ERROR "ERROR"
#define LOGLEVEL_INFO "INFO"

/**************************** 单条数据查询开始 ****************************/

/**
 * @brief 本函数用于按照特定标准查询数据库 db/user.db 中符合条件的条目（只返回第一条）
 *
 * @param key 查询的标准，合法的范围为 id(用户唯一标识符), number(用户的学号/工号), name(用户的姓名), username(用户的用户名)
 * @param content 查询的内容，依据提供的查询标准，需要查询的内容需要填入content
 * @param user_to_return 查询结果返回的 User 结构体类型变量
 * @param log_file 日志写入的文件
 *
 * @returns int 函数执行成功与否，成功返回0，否则为1
 */
int query_user_info(const char key[], const char content[], struct User *user_to_return, FILE *log_file)
{
    struct SqlResponseUser response; // 定义User响应体，用于存放sql查询的数据
    char current_time[20];           // 时间保存变量
    sqlite3 *db;                     // 定义数据库
    sqlite3_stmt *stmt;              // 用于处理 SQL 语句
    int rc;                          // 用于存储执行结果
    char *errmsg = 0;                // 定义错误消息
    const char *cmd = "SELECT id, username, hashpass, salt, role, name, class_name, number FROM users WHERE ? = ?";

    if (strcmp(key, "id") && strcmp(key, "number") && strcmp(key, "name") && strcmp(key, "username")) // 当查询条件不合法
    {
        get_current_time(current_time, sizeof(current_time));
        fprintf(log_file, "%s [%s]: 查询条件 %s 不合法！\n", current_time, LOGLEVEL_ERROR, key);
        sqlite3_close(db);
        return 1;
    }

    // 打开数据库
    rc = sqlite3_open("db/user.db", &db);
    if (rc)
    {
        get_current_time(current_time, sizeof(current_time));
        fprintf(log_file, "%s [%s]: 打开数据库 'user.db' 失败：%s\n", current_time, LOGLEVEL_ERROR, sqlite3_errmsg(db));
        sqlite3_close(db);
        return 1; // 执行失败
    }

    // 准备查询语句
    rc = sqlite3_prepare_v2(db, cmd, -1, &stmt, 0);
    if (rc != SQLITE_OK)
    {
        get_current_time(current_time, sizeof(current_time));
        fprintf(log_file, "%s [%s]: 准备 SQL 语句失败：%s\n", current_time, LOGLEVEL_ERROR, sqlite3_errmsg(db));
        sqlite3_close(db);
        return 1; // 执行失败
    }

    // 绑定参数，替换问号占位符
    rc = sqlite3_bind_text(stmt, 1, key, -1, SQLITE_STATIC); // 绑定查询条件的字段名（如：id, number等）
    if (rc != SQLITE_OK)
    {
        get_current_time(current_time, sizeof(current_time));
        fprintf(log_file, "%s [%s]: 绑定字段名参数失败：%s\n", current_time, LOGLEVEL_ERROR, sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return 1; // 执行失败
    }

    rc = sqlite3_bind_text(stmt, 2, content, -1, SQLITE_STATIC); // 绑定查询内容
    if (rc != SQLITE_OK)
    {
        get_current_time(current_time, sizeof(current_time));
        fprintf(log_file, "%s [%s]: 绑定查询内容参数失败：%s\n", current_time, LOGLEVEL_ERROR, sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return 1; // 执行失败
    }

    // 执行查询并获取第一条结果
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW)
    {
        // 将查询结果填充到 response 结构体
        strncpy(response.id, (const char *)sqlite3_column_text(stmt, 0), sizeof(response.id));
        strncpy(response.username, (const char *)sqlite3_column_text(stmt, 1), sizeof(response.username));
        strncpy(response.hashpass, (const char *)sqlite3_column_text(stmt, 2), sizeof(response.hashpass));
        strncpy(response.salt, (const char *)sqlite3_column_text(stmt, 3), sizeof(response.salt));
        response.role = sqlite3_column_int(stmt, 4);
        strncpy(response.name, (const char *)sqlite3_column_text(stmt, 5), sizeof(response.name));
        strncpy(response.class_name, (const char *)sqlite3_column_text(stmt, 6), sizeof(response.class_name));
        response.number = sqlite3_column_int(stmt, 7);
        strncpy(response.belong_to, (const char *)sqlite3_column_text(stmt, 8), sizeof(response.belong_to));

        // 将查询结果复制到 user_to_return 结构体
        strncpy(user_to_return->id, response.id, sizeof(user_to_return->id));
        strncpy(user_to_return->username, response.username, sizeof(user_to_return->username));
        user_to_return->role = response.role;
        strncpy(user_to_return->name, response.name, sizeof(user_to_return->name));
        strncpy(user_to_return->class_name, response.class_name, sizeof(user_to_return->class_name));
        user_to_return->number = response.number;
        user_to_return->permission = get_permission(*user_to_return);
        strncpy(user_to_return->belong_to, response.belong_to, sizeof(user_to_return->belong_to));

        // 查询成功，记录日志
        get_current_time(current_time, sizeof(current_time));
        fprintf(log_file, "%s [%s]: 成功查询到用户信息，用户ID：%s\n", current_time, LOGLEVEL_INFO, user_to_return->id);
    }
    else
    {
        // 没有查询到结果
        get_current_time(current_time, sizeof(current_time));
        fprintf(log_file, "%s [%s]: 没有找到符合条件的用户信息\n", current_time, LOGLEVEL_INFO);
    }

    // 释放语句
    sqlite3_finalize(stmt);
    // 关闭数据库连接
    sqlite3_close(db);

    return 0; // 执行成功
}

/**
 * @brief 本函数用于按照特定标准查询数据库 db/exam.db 中符合条件的考试条目（只返回第一条）
 *
 * @param key 查询的标准，合法的范围为 id(考试唯一标识符), name(考试名称)
 * @param content 查询的内容，依据提供的查询标准，需要查询的内容需要填入content
 * @param exam_to_return 查询结果返回的 Exam 结构体类型变量
 * @param log_file 日志写入的文件
 *
 * @returns int 函数执行成功与否，成功返回0，否则为1
 */
int query_exam_info(const char key[], const char content[], struct SqlResponseExam *exam_to_return, FILE *log_file)
{
    struct SqlResponseExam response; // 定义Exam响应体，用于存放sql查询的数据
    char current_time[20];           // 时间保存变量
    sqlite3 *db;                     // 定义数据库
    sqlite3_stmt *stmt;              // 用于处理 SQL 语句
    int rc;                          // 用于存储执行结果
    char *errmsg = 0;                // 定义错误消息
    const char *cmd = "SELECT id, name, start_time, end_time, allow_answer_when_expired, random_question FROM exams WHERE ? = ?";

    if (strcmp(key, "id") && strcmp(key, "name")) // 当查询条件不合法
    {
        get_current_time(current_time, sizeof(current_time));
        fprintf(log_file, "%s [%s]: 查询条件 %s 不合法！\n", current_time, LOGLEVEL_ERROR, key);
        sqlite3_close(db);
        return 1;
    }

    // 打开数据库
    rc = sqlite3_open("db/exam.db", &db);
    if (rc)
    {
        get_current_time(current_time, sizeof(current_time));
        fprintf(log_file, "%s [%s]: 打开数据库 'exam.db' 失败：%s\n", current_time, LOGLEVEL_ERROR, sqlite3_errmsg(db));
        sqlite3_close(db);
        return 1;
    }

    // 准备查询语句
    rc = sqlite3_prepare_v2(db, cmd, -1, &stmt, 0);
    if (rc != SQLITE_OK)
    {
        get_current_time(current_time, sizeof(current_time));
        fprintf(log_file, "%s [%s]: 准备 SQL 语句失败：%s\n", current_time, LOGLEVEL_ERROR, sqlite3_errmsg(db));
        sqlite3_close(db);
        return 1;
    }

    // 绑定参数，替换问号占位符
    rc = sqlite3_bind_text(stmt, 1, key, -1, SQLITE_STATIC); // 绑定查询条件的字段名（如：id, name等）
    if (rc != SQLITE_OK)
    {
        get_current_time(current_time, sizeof(current_time));
        fprintf(log_file, "%s [%s]: 绑定字段名参数失败：%s\n", current_time, LOGLEVEL_ERROR, sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return 1;
    }

    rc = sqlite3_bind_text(stmt, 2, content, -1, SQLITE_STATIC); // 绑定查询内容
    if (rc != SQLITE_OK)
    {
        get_current_time(current_time, sizeof(current_time));
        fprintf(log_file, "%s [%s]: 绑定查询内容参数失败：%s\n", current_time, LOGLEVEL_ERROR, sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return 1;
    }

    // 执行查询并获取第一条结果
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW)
    {
        // 将查询结果填充到 response 结构体
        strncpy(response.id, (const char *)sqlite3_column_text(stmt, 0), sizeof(response.id));
        strncpy(response.name, (const char *)sqlite3_column_text(stmt, 1), sizeof(response.name));
        response.start_time = sqlite3_column_int(stmt, 2);
        response.end_time = sqlite3_column_int(stmt, 3);
        response.allow_answer_when_expired = sqlite3_column_int(stmt, 4);
        response.random_question = sqlite3_column_int(stmt, 5);

        // 将查询结果复制到 exam_to_return 结构体
        strncpy(exam_to_return->id, response.id, sizeof(exam_to_return->id));
        strncpy(exam_to_return->name, response.name, sizeof(exam_to_return->name));
        exam_to_return->start_time = response.start_time;
        exam_to_return->end_time = response.end_time;
        exam_to_return->allow_answer_when_expired = response.allow_answer_when_expired;
        exam_to_return->random_question = response.random_question;

        // 查询成功，记录日志
        get_current_time(current_time, sizeof(current_time));
        fprintf(log_file, "%s [%s]: 成功查询到考试信息，考试ID：%s\n", current_time, LOGLEVEL_INFO, exam_to_return->id);
    }
    else
    {
        // 没有查询到结果
        get_current_time(current_time, sizeof(current_time));
        fprintf(log_file, "%s [%s]: 没有找到符合条件的考试信息\n", current_time, LOGLEVEL_INFO);
    }

    // 释放语句
    sqlite3_finalize(stmt);
    // 关闭数据库连接
    sqlite3_close(db);

    return 0; // 执行成功
}

/**
 * @brief 本函数用于按照特定标准查询数据库 db/examination.db 中符合条件的题目信息（只返回第一条）
 *
 * @param key 查询的标准，合法的范围为 id(题目唯一标识符), exam_id(考试ID)
 * @param content 查询的内容，依据提供的查询标准，需要查询的内容需要填入content
 * @param question_to_return 查询结果返回的 Question 结构体类型变量
 * @param log_file 日志写入的文件
 *
 * @returns int 函数执行成功与否，成功返回0，否则为1
 */
int query_question_info(const char key[], const char content[], struct SqlResponseQuestion *question_to_return, FILE *log_file)
{
    struct SqlResponseQuestion response; // 定义Question响应体，用于存放sql查询的数据
    char current_time[20];               // 时间保存变量
    sqlite3 *db;                         // 定义数据库
    sqlite3_stmt *stmt;                  // 用于处理 SQL 语句
    int rc;                              // 用于存储执行结果
    char *errmsg = 0;                    // 定义错误消息
    const char *cmd = "SELECT id, exam_id, num1, op, num2 FROM questions WHERE ? = ?";

    if (strcmp(key, "id") && strcmp(key, "exam_id")) // 当查询条件不合法
    {
        get_current_time(current_time, sizeof(current_time));
        fprintf(log_file, "%s [%s]: 查询条件 %s 不合法！\n", current_time, LOGLEVEL_ERROR, key);
        sqlite3_close(db);
        return 1;
    }

    // 打开数据库
    rc = sqlite3_open("db/examination.db", &db);
    if (rc)
    {
        get_current_time(current_time, sizeof(current_time));
        fprintf(log_file, "%s [%s]: 打开数据库 'examination.db' 失败：%s\n", current_time, LOGLEVEL_ERROR, sqlite3_errmsg(db));
        sqlite3_close(db);
        return 1;
    }

    // 准备查询语句
    rc = sqlite3_prepare_v2(db, cmd, -1, &stmt, 0);
    if (rc != SQLITE_OK)
    {
        get_current_time(current_time, sizeof(current_time));
        fprintf(log_file, "%s [%s]: 准备 SQL 语句失败：%s\n", current_time, LOGLEVEL_ERROR, sqlite3_errmsg(db));
        sqlite3_close(db);
        return 1;
    }

    // 绑定参数，替换问号占位符
    rc = sqlite3_bind_text(stmt, 1, key, -1, SQLITE_STATIC); // 绑定查询条件的字段名
    if (rc != SQLITE_OK)
    {
        get_current_time(current_time, sizeof(current_time));
        fprintf(log_file, "%s [%s]: 绑定字段名参数失败：%s\n", current_time, LOGLEVEL_ERROR, sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return 1;
    }

    rc = sqlite3_bind_text(stmt, 2, content, -1, SQLITE_STATIC); // 绑定查询内容
    if (rc != SQLITE_OK)
    {
        get_current_time(current_time, sizeof(current_time));
        fprintf(log_file, "%s [%s]: 绑定查询内容参数失败：%s\n", current_time, LOGLEVEL_ERROR, sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return 1;
    }

    // 执行查询并获取第一条结果
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW)
    {
        // 将查询结果填充到 response 结构体
        strncpy(response.id, (const char *)sqlite3_column_text(stmt, 0), sizeof(response.id));
        strncpy(response.exam_id, (const char *)sqlite3_column_text(stmt, 1), sizeof(response.exam_id));
        response.num1 = sqlite3_column_double(stmt, 2);
        response.op = sqlite3_column_int(stmt, 3);
        response.num2 = sqlite3_column_double(stmt, 4);

        // 将查询结果复制到 question_to_return 结构体
        strncpy(question_to_return->id, response.id, sizeof(question_to_return->id));
        strncpy(question_to_return->exam_id, response.exam_id, sizeof(question_to_return->exam_id));
        question_to_return->num1 = response.num1;
        question_to_return->op = response.op;
        question_to_return->num2 = response.num2;

        // 查询成功，记录日志
        get_current_time(current_time, sizeof(current_time));
        fprintf(log_file, "%s [%s]: 成功查询到题目信息，题目ID：%s\n", current_time, LOGLEVEL_INFO, question_to_return->id);
    }
    else
    {
        // 没有查询到结果
        get_current_time(current_time, sizeof(current_time));
        fprintf(log_file, "%s [%s]: 没有找到符合条件的题目信息\n", current_time, LOGLEVEL_INFO);
    }

    // 释放语句
    sqlite3_finalize(stmt);
    // 关闭数据库连接
    sqlite3_close(db);

    return 0; // 执行成功
}

/**
 * @brief 本函数用于根据考试ID和用户ID查询成绩信息，并将结果存储到 SqlResponseScore 结构体中
 *
 * @param exam_id 考试ID，查询指定考试的成绩
 * @param user_id 用户ID，查询指定用户的成绩
 * @param score_to_return 查询结果返回的 SqlResponseScore 结构体类型变量
 * @param log_file 日志写入的文件
 *
 * @returns int 函数执行成功与否，成功返回0，否则为1
 */
int query_score_info(const char exam_id[], const char user_id[], struct SqlResponseScore *score_to_return, FILE *log_file)
{
    char current_time[20]; // 时间保存变量
    sqlite3 *db;           // 定义数据库
    sqlite3_stmt *stmt;    // 用于处理 SQL 语句
    int rc;                // 用于存储执行结果
    char *errmsg = 0;      // 定义错误消息
    const char *cmd = "SELECT id, exam_id, user_id, score, expired_flag FROM scores WHERE exam_id = ? AND user_id = ?";

    // 打开数据库
    rc = sqlite3_open("db/score.db", &db);
    if (rc)
    {
        get_current_time(current_time, sizeof(current_time));
        fprintf(log_file, "%s [%s]: 打开数据库 'score.db' 失败：%s\n", current_time, LOGLEVEL_ERROR, sqlite3_errmsg(db));
        sqlite3_close(db);
        return 1; // 执行失败
    }

    // 准备查询语句
    rc = sqlite3_prepare_v2(db, cmd, -1, &stmt, 0);
    if (rc != SQLITE_OK)
    {
        get_current_time(current_time, sizeof(current_time));
        fprintf(log_file, "%s [%s]: 准备 SQL 语句失败：%s\n", current_time, LOGLEVEL_ERROR, sqlite3_errmsg(db));
        sqlite3_close(db);
        return 1; // 执行失败
    }

    // 绑定查询条件，替换占位符
    rc = sqlite3_bind_text(stmt, 1, exam_id, -1, SQLITE_STATIC); // 绑定考试ID
    if (rc != SQLITE_OK)
    {
        get_current_time(current_time, sizeof(current_time));
        fprintf(log_file, "%s [%s]: 绑定考试ID参数失败：%s\n", current_time, LOGLEVEL_ERROR, sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return 1; // 执行失败
    }

    rc = sqlite3_bind_text(stmt, 2, user_id, -1, SQLITE_STATIC); // 绑定用户ID
    if (rc != SQLITE_OK)
    {
        get_current_time(current_time, sizeof(current_time));
        fprintf(log_file, "%s [%s]: 绑定用户ID参数失败：%s\n", current_time, LOGLEVEL_ERROR, sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return 1; // 执行失败
    }

    // 执行查询并获取第一条结果
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW)
    {
        // 将查询结果填充到 score 结构体
        strncpy(score_to_return->id, (const char *)sqlite3_column_text(stmt, 0), sizeof(score_to_return->id));
        strncpy(score_to_return->exam_id, (const char *)sqlite3_column_text(stmt, 1), sizeof(score_to_return->exam_id));
        strncpy(score_to_return->user_id, (const char *)sqlite3_column_text(stmt, 2), sizeof(score_to_return->user_id));
        score_to_return->score = (float)sqlite3_column_double(stmt, 3); // 强制转换为 float
        score_to_return->expired_flag = sqlite3_column_int(stmt, 4);    // 逾期标志

        // 查询成功，记录日志
        get_current_time(current_time, sizeof(current_time));
        fprintf(log_file, "%s [%s]: 成功查询到成绩信息，考试ID：%s，用户ID：%s\n", current_time, LOGLEVEL_INFO, exam_id, user_id);
    }
    else
    {
        // 没有查询到结果
        get_current_time(current_time, sizeof(current_time));
        fprintf(log_file, "%s [%s]: 没有找到符合条件的成绩信息，考试ID：%s，用户ID：%s\n", current_time, LOGLEVEL_INFO, exam_id, user_id);
    }

    // 释放语句
    sqlite3_finalize(stmt);
    // 关闭数据库连接
    sqlite3_close(db);

    return 0; // 执行成功
}

/**************************** 单条数据查询结束 ****************************/

/**************************** 多条数据查询开始 ****************************/

/**
 * @brief 本函数用于查询数据库 db/examination.db 中符合条件的所有考试信息（返回多条数据）
 *
 * @param name 考试名称，支持模糊查询，可以为空表示查询所有考试
 * @param start_time 查询起始时间的最小值，-1表示不限制
 * @param end_time 查询结束时间的最大值，-1表示不限制
 * @param exams_to_return 查询结果返回的 SqlResponseExam 结构体类型数组
 * @param length 查询结果数组的大小，同时也是查询结果返回的限制数量，类似于一个外置的LIMIT
 * @param log_file 日志写入的文件
 *
 * @returns int 函数执行成功与否，成功返回0，否则为1
 */
int query_exams_info_all(const char name[], int start_time, int end_time, struct SqlResponseExam *exams_to_return, int length, FILE *log_file)
{
    char current_time[20]; // 时间保存变量
    sqlite3 *db;           // 定义数据库
    sqlite3_stmt *stmt;    // 用于处理 SQL 语句
    int rc;                // 用于存储执行结果
    int count = 0;         // 记录查询到的记录数
    char *errmsg = 0;      // 定义错误消息
    const char *cmd = "SELECT id, name, start_time, end_time, allow_answer_when_expired, random_question FROM exams WHERE (name LIKE ?) AND (start_time >= ?) AND (end_time <= ?)";
    // 打开数据库
    rc = sqlite3_open("db/examination.db", &db);
    if (rc)
    {
        get_current_time(current_time, sizeof(current_time));
        fprintf(log_file, "%s [%s]: 打开数据库 'examination.db' 失败：%s\n", current_time, LOGLEVEL_ERROR, sqlite3_errmsg(db));
        sqlite3_close(db);
        return 1; // 执行失败
    }

    // 准备查询语句
    rc = sqlite3_prepare_v2(db, cmd, -1, &stmt, 0);
    if (rc != SQLITE_OK)
    {
        get_current_time(current_time, sizeof(current_time));
        fprintf(log_file, "%s [%s]: 准备 SQL 语句失败：%s\n", current_time, LOGLEVEL_ERROR, sqlite3_errmsg(db));
        sqlite3_close(db);
        return 1; // 执行失败
    }

    // 绑定查询条件，替换占位符
    rc = sqlite3_bind_text(stmt, 1, name, -1, SQLITE_STATIC); // 模糊查询名称
    if (rc != SQLITE_OK)
    {
        get_current_time(current_time, sizeof(current_time));
        fprintf(log_file, "%s [%s]: 绑定名称参数失败：%s\n", current_time, LOGLEVEL_ERROR, sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return 1; // 执行失败
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
        get_current_time(current_time, sizeof(current_time));
        fprintf(log_file, "%s [%s]: 绑定开始时间条件失败：%s\n", current_time, LOGLEVEL_ERROR, sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return 1; // 执行失败
    }

    if (end_time == -1)
    {
        rc = sqlite3_bind_int64(stmt, 3, (unsigned long long)(0 - 1)); // 不限制结束时间,设置为一个很大的数字
    }
    else
    {
        rc = sqlite3_bind_int(stmt, 3, end_time); // 绑定结束时间截止条件
    }
    if (rc != SQLITE_OK)
    {
        get_current_time(current_time, sizeof(current_time));
        fprintf(log_file, "%s [%s]: 绑定结束时间条件失败：%s\n", current_time, LOGLEVEL_ERROR, sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return 1; // 执行失败
    }

    // 执行查询
    while (sqlite3_step(stmt) == SQLITE_ROW && count < length)
    {
        // 将查询结果填充到 exams_to_return 数组中
        strncpy(exams_to_return[count].id, (const char *)sqlite3_column_text(stmt, 0), sizeof(exams_to_return[count].id));
        strncpy(exams_to_return[count].name, (const char *)sqlite3_column_text(stmt, 1), sizeof(exams_to_return[count].name));
        exams_to_return[count].start_time = sqlite3_column_int(stmt, 2);
        exams_to_return[count].end_time = sqlite3_column_int(stmt, 3);
        exams_to_return[count].allow_answer_when_expired = sqlite3_column_int(stmt, 4);
        exams_to_return[count].random_question = sqlite3_column_int(stmt, 5);
        count++;
    }

    if (count > 0)
    {
        get_current_time(current_time, sizeof(current_time));
        fprintf(log_file, "%s [%s]: 成功查询到 %d 条考试信息\n", current_time, LOGLEVEL_INFO, count);
    }
    else
    {
        get_current_time(current_time, sizeof(current_time));
        fprintf(log_file, "%s [%s]: 没有找到符合条件的考试信息\n", current_time, LOGLEVEL_INFO);
    }

    // 释放语句
    sqlite3_finalize(stmt);
    // 关闭数据库连接
    sqlite3_close(db);

    return 0; // 执行成功
}

/**
 * @brief 本函数用于查询数据库 db/user.db 中符合条件的所有用户信息（返回多条数据）
 *
 * @param name 用户姓名，支持模糊查询，可以为空表示查询所有用户
 * @param class_name 用户班级，支持模糊查询，可以为空表示查询所有班级
 * @param role 用户角色，需要精确查询，0表示学生，1表示老师
 * @param number 用户学号/工号，支持模糊查询，可以为空表示查询所有用户
 * @param belong_to (仅学生) 学生归属的老师，需要填入老师的UUID
 * @param users_to_return 查询结果返回的 SqlResponseUser 结构体类型数组
 * @param length 查询结果数组的大小
 * @param log_file 日志写入的文件
 *
 * @returns int 函数执行成功与否，成功返回0，否则为1
 */
int query_users_info_all(const char name[], const char class_name[], int role, const char number[], const char belong_to[], struct SqlResponseUser *users_to_return, int length, FILE *log_file)
{
    char current_time[20]; // 时间保存变量
    sqlite3 *db;           // 定义数据库
    sqlite3_stmt *stmt;    // 用于处理 SQL 语句
    int rc;                // 用于存储执行结果
    int count = 0;         // 记录查询到的记录数
    char *errmsg = 0;      // 定义错误消息
    const char *cmd = "SELECT id, username, hashpass, salt, role, name, class_name, number FROM users WHERE (name LIKE ?) AND (class_name LIKE ?) AND (role = ?) AND (number LIKE ?) AND (belong_to = ?)";

    // 打开数据库
    rc = sqlite3_open("db/user.db", &db);
    if (rc)
    {
        get_current_time(current_time, sizeof(current_time));
        fprintf(log_file, "%s [%s]: 打开数据库 'user.db' 失败：%s\n", current_time, LOGLEVEL_ERROR, sqlite3_errmsg(db));
        sqlite3_close(db);
        return 1; // 执行失败
    }

    // 准备查询语句
    rc = sqlite3_prepare_v2(db, cmd, -1, &stmt, 0);
    if (rc != SQLITE_OK)
    {
        get_current_time(current_time, sizeof(current_time));
        fprintf(log_file, "%s [%s]: 准备 SQL 语句失败：%s\n", current_time, LOGLEVEL_ERROR, sqlite3_errmsg(db));
        sqlite3_close(db);
        return 1; // 执行失败
    }

    // 绑定查询条件，替换占位符
    rc = sqlite3_bind_text(stmt, 1, name, -1, SQLITE_STATIC); // 模糊查询姓名
    if (rc != SQLITE_OK)
    {
        get_current_time(current_time, sizeof(current_time));
        fprintf(log_file, "%s [%s]: 绑定姓名参数失败：%s\n", current_time, LOGLEVEL_ERROR, sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return 1; // 执行失败
    }

    rc = sqlite3_bind_text(stmt, 2, class_name, -1, SQLITE_STATIC); // 模糊查询班级
    if (rc != SQLITE_OK)
    {
        get_current_time(current_time, sizeof(current_time));
        fprintf(log_file, "%s [%s]: 绑定班级参数失败：%s\n", current_time, LOGLEVEL_ERROR, sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return 1; // 执行失败
    }

    rc = sqlite3_bind_int(stmt, 3, role); // 精确查询角色
    if (rc != SQLITE_OK)
    {
        get_current_time(current_time, sizeof(current_time));
        fprintf(log_file, "%s [%s]: 绑定角色条件失败：%s\n", current_time, LOGLEVEL_ERROR, sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return 1; // 执行失败
    }

    rc = sqlite3_bind_text(stmt, 4, number, -1, SQLITE_STATIC); // 模糊查询学号/工号
    if (rc != SQLITE_OK)
    {
        get_current_time(current_time, sizeof(current_time));
        fprintf(log_file, "%s [%s]: 绑定学号/工号条件失败：%s\n", current_time, LOGLEVEL_ERROR, sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return 1; // 执行失败
    }

    rc = sqlite3_bind_text(stmt, 5, belong_to, -1, SQLITE_STATIC);
    {
        get_current_time(current_time, sizeof(current_time));
        fprintf(log_file, "%s [%s]: 绑定学号/工号条件失败：%s\n", current_time, LOGLEVEL_ERROR, sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return 1; // 执行失败
    }

    // 执行查询
    while (sqlite3_step(stmt) == SQLITE_ROW && count < length)
    {
        // 将查询结果填充到 users_to_return 数组中
        strncpy(users_to_return[count].id, (const char *)sqlite3_column_text(stmt, 0), sizeof(users_to_return[count].id));
        strncpy(users_to_return[count].username, (const char *)sqlite3_column_text(stmt, 1), sizeof(users_to_return[count].username));
        strncpy(users_to_return[count].hashpass, (const char *)sqlite3_column_text(stmt, 2), sizeof(users_to_return[count].hashpass));
        strncpy(users_to_return[count].salt, (const char *)sqlite3_column_text(stmt, 3), sizeof(users_to_return[count].salt));
        users_to_return[count].role = sqlite3_column_int(stmt, 4);
        strncpy(users_to_return[count].name, (const char *)sqlite3_column_text(stmt, 5), sizeof(users_to_return[count].name));
        strncpy(users_to_return[count].class_name, (const char *)sqlite3_column_text(stmt, 6), sizeof(users_to_return[count].class_name));
        users_to_return[count].number = sqlite3_column_int(stmt, 7);
        strncpy(users_to_return[count].belong_to, (const char *)sqlite3_column_text(stmt, 8), sizeof(users_to_return[count].belong_to));
        count++;
    }

    if (count > 0)
    {
        get_current_time(current_time, sizeof(current_time));
        fprintf(log_file, "%s [%s]: 成功查询到 %d 条用户信息\n", current_time, LOGLEVEL_INFO, count);
    }
    else
    {
        get_current_time(current_time, sizeof(current_time));
        fprintf(log_file, "%s [%s]: 没有找到符合条件的用户信息\n", current_time, LOGLEVEL_INFO);
    }

    // 释放语句
    sqlite3_finalize(stmt);
    // 关闭数据库连接
    sqlite3_close(db);

    return 0; // 执行成功
}

/**
 * @brief 本函数用于查询数据库 db/examination.db 中符合条件的所有问题信息（返回多条数据）
 *
 * @param exam_id 所属考试的ID，只允许此方式查询，用来构建问题列表
 * @param questions_to_return 查询结果返回的 SqlResponseQuestion 结构体类型数组
 * @param length 查询结果数组的大小
 * @param log_file 日志写入的文件
 *
 * @returns int 函数执行成功与否，成功返回0，否则为1
 */
int query_questions_info_all(const char exam_id[], struct SqlResponseQuestion *questions_to_return, int length, FILE *log_file)
{
    char current_time[20]; // 时间保存变量
    sqlite3 *db;           // 定义数据库
    sqlite3_stmt *stmt;    // 用于处理 SQL 语句
    int rc;                // 用于存储执行结果
    int count = 0;         // 记录查询到的记录数
    char *errmsg = 0;      // 定义错误消息
    const char *cmd = "SELECT id, exam_id, num1, op, num2 FROM questions WHERE (exam_id = ?)";

    // 打开数据库
    rc = sqlite3_open("db/examination.db", &db);
    if (rc)
    {
        get_current_time(current_time, sizeof(current_time));
        fprintf(log_file, "%s [%s]: 打开数据库 'examination.db' 失败：%s\n", current_time, LOGLEVEL_ERROR, sqlite3_errmsg(db));
        sqlite3_close(db);
        return 1; // 执行失败
    }

    // 准备查询语句
    rc = sqlite3_prepare_v2(db, cmd, -1, &stmt, 0);
    if (rc != SQLITE_OK)
    {
        get_current_time(current_time, sizeof(current_time));
        fprintf(log_file, "%s [%s]: 准备 SQL 语句失败：%s\n", current_time, LOGLEVEL_ERROR, sqlite3_errmsg(db));
        sqlite3_close(db);
        return 1; // 执行失败
    }

    // 绑定查询条件，替换占位符
    rc = sqlite3_bind_text(stmt, 1, exam_id, -1, SQLITE_STATIC); // 精确查询所属考试ID
    if (rc != SQLITE_OK)
    {
        get_current_time(current_time, sizeof(current_time));
        fprintf(log_file, "%s [%s]: 绑定考试ID参数失败：%s\n", current_time, LOGLEVEL_ERROR, sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return 1; // 执行失败
    }

    // 执行查询
    while (sqlite3_step(stmt) == SQLITE_ROW && count < length)
    {
        // 将查询结果填充到 questions_to_return 数组中
        strncpy(questions_to_return[count].id, (const char *)sqlite3_column_text(stmt, 0), sizeof(questions_to_return[count].id));
        strncpy(questions_to_return[count].exam_id, (const char *)sqlite3_column_text(stmt, 1), sizeof(questions_to_return[count].exam_id));
        questions_to_return[count].num1 = sqlite3_column_double(stmt, 3);
        questions_to_return[count].op = sqlite3_column_int(stmt, 4);
        questions_to_return[count].num2 = sqlite3_column_double(stmt, 5);
        count++;
    }

    if (count > 0)
    {
        get_current_time(current_time, sizeof(current_time));
        fprintf(log_file, "%s [%s]: 成功查询到 %d 条问题信息\n", current_time, LOGLEVEL_INFO, count);
    }
    else
    {
        get_current_time(current_time, sizeof(current_time));
        fprintf(log_file, "%s [%s]: 没有找到符合条件的问题信息\n", current_time, LOGLEVEL_INFO);
    }

    // 释放语句
    sqlite3_finalize(stmt);
    // 关闭数据库连接
    sqlite3_close(db);

    return 0; // 执行成功
}

/**
 * @brief 本函数用于查询数据库 db/examination.db 中符合条件的所有成绩信息（返回多条数据）
 *
 * @param exam_id 考试ID，支持精确查询，可以为空表示查询所有考试的成绩
 * @param student_number 学号，支持精确查询，可以为空表示查询所有学生的成绩
 * @param scores_to_return 查询结果返回的 SqlResponseScore 结构体类型数组
 * @param length 查询结果数组的大小
 * @param log_file 日志写入的文件
 *
 * @returns int 函数执行成功与否，成功返回0，否则为1
 */
int query_scores_info_all(const char exam_id[], const char student_number[], struct SqlResponseScore *scores_to_return, int length, FILE *log_file)
{
    char current_time[20]; // 时间保存变量
    sqlite3 *db;           // 定义数据库
    sqlite3_stmt *stmt;    // 用于处理 SQL 语句
    int rc;                // 用于存储执行结果
    int count = 0;         // 记录查询到的记录数
    char *errmsg = 0;      // 定义错误消息
    const char *cmd = "SELECT exam_id, student_number, score FROM scores WHERE (exam_id LIKE ?) AND (student_number LIKE ?)";

    // 打开数据库
    rc = sqlite3_open("db/examination.db", &db);
    if (rc)
    {
        get_current_time(current_time, sizeof(current_time));
        fprintf(log_file, "%s [%s]: 打开数据库 'examination.db' 失败：%s\n", current_time, LOGLEVEL_ERROR, sqlite3_errmsg(db));
        sqlite3_close(db);
        return 1; // 执行失败
    }

    // 准备查询语句
    rc = sqlite3_prepare_v2(db, cmd, -1, &stmt, 0);
    if (rc != SQLITE_OK)
    {
        get_current_time(current_time, sizeof(current_time));
        fprintf(log_file, "%s [%s]: 准备 SQL 语句失败：%s\n", current_time, LOGLEVEL_ERROR, sqlite3_errmsg(db));
        sqlite3_close(db);
        return 1; // 执行失败
    }

    // 绑定查询条件，替换占位符
    rc = sqlite3_bind_text(stmt, 1, exam_id, -1, SQLITE_STATIC); // 精确查询考试ID
    if (rc != SQLITE_OK)
    {
        get_current_time(current_time, sizeof(current_time));
        fprintf(log_file, "%s [%s]: 绑定考试ID参数失败：%s\n", current_time, LOGLEVEL_ERROR, sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return 1; // 执行失败
    }

    rc = sqlite3_bind_text(stmt, 2, student_number, -1, SQLITE_STATIC); // 精确查询学号
    if (rc != SQLITE_OK)
    {
        get_current_time(current_time, sizeof(current_time));
        fprintf(log_file, "%s [%s]: 绑定学号条件失败：%s\n", current_time, LOGLEVEL_ERROR, sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return 1; // 执行失败
    }

    // 执行查询
    while (sqlite3_step(stmt) == SQLITE_ROW && count < length)
    {
        // 将查询结果填充到 scores_to_return 数组中
        strncpy(scores_to_return[count].id, (const char *)sqlite3_column_text(stmt, 0), sizeof(scores_to_return[count].id));
        strncpy(scores_to_return[count].exam_id, (const char *)sqlite3_column_text(stmt, 1), sizeof(scores_to_return[count].exam_id));
        strncpy(scores_to_return[count].user_id, (const char *)sqlite3_column_text(stmt, 2), sizeof(scores_to_return[count].user_id));
        scores_to_return[count].score = sqlite3_column_double(stmt, 3);
        scores_to_return[count].expired_flag = sqlite3_column_int(stmt, 4);
        count++;
    }

    if (count > 0)
    {
        get_current_time(current_time, sizeof(current_time));
        fprintf(log_file, "%s [%s]: 成功查询到 %d 条成绩信息\n", current_time, LOGLEVEL_INFO, count);
    }
    else
    {
        get_current_time(current_time, sizeof(current_time));
        fprintf(log_file, "%s [%s]: 没有找到符合条件的成绩信息\n", current_time, LOGLEVEL_INFO);
    }

    // 释放语句
    sqlite3_finalize(stmt);
    // 关闭数据库连接
    sqlite3_close(db);

    return 0; // 执行成功
}

/**************************** 多条数据查询结束 ****************************/