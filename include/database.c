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
 */

#include <stdio.h>
#include <string.h>
#include "model.h"
#include "../lib/sqlite3.h"
#include "utils.h"

#define LOGLEVEL_ERROR "ERROR"
#define LOGLEVEL_INFO "INFO"

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

        // 将查询结果复制到 user_to_return 结构体
        strncpy(user_to_return->id, response.id, sizeof(user_to_return->id));
        strncpy(user_to_return->username, response.username, sizeof(user_to_return->username));
        user_to_return->role = response.role;
        strncpy(user_to_return->name, response.name, sizeof(user_to_return->name));
        strncpy(user_to_return->class_name, response.class_name, sizeof(user_to_return->class_name));
        user_to_return->number = response.number;
        user_to_return->permission = get_permission(*user_to_return);

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