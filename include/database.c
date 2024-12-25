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
    4.  Date: 2024/12/10
        Author: 吴沛熹
        ID: GamerNoTitle
        Modification:   [*] 将 query_*_all 函数改为查询所有的内容，筛选通过中间件flask进行
    5.  Date: 2024/12/17
        Author: 吴沛熹
        ID: GamerNoTitle
        Modification:   [*] 更改了题目的操作数数据类型，从float改为int
                        [*] 现在允许在批量查询的过程中设置条件
    6.  Date: 2024/12/21
        Author: 吴沛熹
        ID: GamerNoTitle
        Modification:   [-] 删除了错误的初始化过程
                        [*] 将学生的分数全部改为int类型存储，不在引入小数点
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
    BIND_TYPE_UINT,
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
        printf("打开数据库失败了……");
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
        strncpy(user_to_return->id, (const char *)sqlite3_column_text(stmt, 0), 37);
        strncpy(user_to_return->username, (const char *)sqlite3_column_text(stmt, 1), 25);

        // hashpass和salt不在User结构体中使用，略过
        // sqlite3_column_text(stmt, 2) => hashpass
        // sqlite3_column_text(stmt, 3) => salt

        user_to_return->role = sqlite3_column_int(stmt, 4);
        strncpy(user_to_return->name, (const char *)sqlite3_column_text(stmt, 5), 46);
        strncpy(user_to_return->class_name, (const char *)sqlite3_column_text(stmt, 6), 31);
        user_to_return->number = (unsigned int)sqlite3_column_int(stmt, 7);
        strncpy(user_to_return->belong_to, (const char *)sqlite3_column_text(stmt, 8), 37);

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
 * @param question_to_return 查询结果返回的 SqlResponseQuestion 结构体类型变量
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
 * @brief 根据用户传递的key和content查询成绩信息，并将结果存储到 SqlResponseScore 结构体中
 *
 * @param key 查询的键，可能是exam_id、user_id或其他字段
 * @param content 查询的内容，具体的exam_id或user_id
 * @param score_to_return 查询结果返回的 SqlResponseScore 结构体类型变量
 *
 * @return int 函数执行成功与否，成功返回0，否则为1
 */
int query_score_info(const char *key, const char *content, struct SqlResponseScore *score_to_return)
{
    sqlite3 *db;
    sqlite3_stmt *stmt;
    int rc;

    // 定义SQL查询语句模板，允许根据 key 来动态选择字段进行查询
    char *sql = "SELECT id, exam_id, user_id, score, expired_flag FROM scores WHERE ";

    // 根据key值判断选择查询条件
    if (strcmp(key, "exam_id") == 0)
    {
        // 查询考试ID
        strcat(sql, "exam_id = ? LIMIT 1;");
    }
    else if (strcmp(key, "user_id") == 0)
    {
        // 查询用户ID
        strcat(sql, "user_id = ? LIMIT 1;");
    }
    else
    {
        // 如果key不是exam_id或者user_id，则查询失败
        log_message(LOGLEVEL_ERROR, "无效的查询条件 key: %s", key);
        return 1;
    }

    // 打开数据库
    if (open_database(SCORES_DB, &db))
    {
        return 1;
    }

    // 初始化传入的score_to_return结构体来避免出错
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

    // 绑定查询内容参数
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
        // 填充SqlResponseScore结构体
        strncpy(score_to_return->id, (const char *)sqlite3_column_text(stmt, 0), sizeof(score_to_return->id) - 1);
        strncpy(score_to_return->exam_id, (const char *)sqlite3_column_text(stmt, 1), sizeof(score_to_return->exam_id) - 1);
        strncpy(score_to_return->user_id, (const char *)sqlite3_column_text(stmt, 2), sizeof(score_to_return->user_id) - 1);
        score_to_return->score = sqlite3_column_int(stmt, 3);
        score_to_return->expired_flag = sqlite3_column_int(stmt, 4);

        log_message(LOGLEVEL_INFO, "成功查询到成绩信息，查询条件：%s = %s", key, content);
    }
    else
    {
        log_message(LOGLEVEL_INFO, "没有找到符合条件的成绩信息，查询条件：%s = %s", key, content);
    }

    // 清理和关闭数据库
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return 0;
}

/**************************** 单条数据查询结束 ****************************/

/**************************** 多条数据查询开始 ****************************/

/**
 * @brief 查询数据库 db/examination.db 中所有考试信息（返回多条数据），可按指定键和值进行过滤
 *
 * @param exams_to_return 查询结果返回的 SqlResponseExam 结构体类型数组
 * @param length 查询结果数组的大小，同时也是查询结果返回的限制数量，类似于一个外置的LIMIT
 * @param key 可选的过滤键，如果为 NULL 或空字符串，则不进行过滤
 * @param content 可选的过滤值，与 key 对应
 *
 * @return int 函数执行成功与否，成功返回0，否则为1
 */
int query_exams_info_all(struct SqlResponseExam *exams_to_return, int length, const char *key, const char *content)
{
    sqlite3 *db;
    sqlite3_stmt *stmt;
    int rc;
    int count = 0;
    char sql[512];

    // 构建SQL语句
    if (key && strlen(key) > 0 && content && strlen(content) > 0)
    {
        // 使用 WHERE 子句进行过滤
        snprintf(sql, sizeof(sql),
                 "SELECT id, name, start_time, end_time, allow_answer_when_expired, random_question FROM examinations WHERE %s = ? LIMIT ?;",
                 key);
    }
    else
    {
        // 无条件查询
        snprintf(sql, sizeof(sql),
                 "SELECT id, name, start_time, end_time, allow_answer_when_expired, random_question FROM examinations LIMIT ?;");
    }

    // 初始化传入的 exams_to_return，避免返回出错
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
    int param_index = 1;
    if (key && strlen(key) > 0 && content && strlen(content) > 0)
    {
        rc = sqlite3_bind_text(stmt, param_index++, content, -1, SQLITE_STATIC);
        if (rc != SQLITE_OK)
        {
            log_message(LOGLEVEL_ERROR, "绑定 content 参数失败：%s", sqlite3_errmsg(db));
            sqlite3_finalize(stmt);
            sqlite3_close(db);
            return 1;
        }
    }

    // 绑定 LIMIT 参数
    rc = sqlite3_bind_int(stmt, param_index, length); // 绑定 LIMIT 参数
    if (rc != SQLITE_OK)
    {
        log_message(LOGLEVEL_ERROR, "绑定 LIMIT 参数失败：%s", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return 1;
    }

    // 执行查询
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW && count < length)
    {
        // 提取查询结果并确保字符串正确终止
        const unsigned char *id_text = sqlite3_column_text(stmt, 0);
        const unsigned char *name_text = sqlite3_column_text(stmt, 1);

        if (id_text && name_text)
        {
            strncpy(exams_to_return[count].id, (const char *)id_text, sizeof(exams_to_return[count].id) - 1);
            exams_to_return[count].id[sizeof(exams_to_return[count].id) - 1] = '\0'; // 确保字符串终止

            strncpy(exams_to_return[count].name, (const char *)name_text, sizeof(exams_to_return[count].name) - 1);
            exams_to_return[count].name[sizeof(exams_to_return[count].name) - 1] = '\0'; // 确保字符串终止

            exams_to_return[count].start_time = sqlite3_column_int(stmt, 2);
            exams_to_return[count].end_time = sqlite3_column_int(stmt, 3);
            exams_to_return[count].allow_answer_when_expired = sqlite3_column_int(stmt, 4);
            exams_to_return[count].random_question = sqlite3_column_int(stmt, 5);
            count++;
        }
    }

    if (count > 0)
    {
        log_message(LOGLEVEL_INFO, "成功查询到 %d 条考试信息", count);
    }
    else
    {
        log_message(LOGLEVEL_INFO, "没有找到任何考试信息");
    }

    // 清理和关闭数据库
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return 0;
}

int query_users_info_all(struct SqlResponseUser *users_to_return, int length, const char *key, const char *content)
{
    sqlite3 *db;
    sqlite3_stmt *stmt;
    int rc;
    int count = 0;
    char sql[512];

    // 构建SQL语句
    if (key && strlen(key) > 0 && content && strlen(content) > 0)
    {
        // 使用 WHERE 子句进行过滤
        snprintf(sql, sizeof(sql),
                 "SELECT id, username, hashpass, salt, role, name, class_name, number, belong_to FROM users WHERE %s = ? LIMIT ?;",
                 key);
    }
    else
    {
        // 无条件查询
        snprintf(sql, sizeof(sql),
                 "SELECT id, username, hashpass, salt, role, name, class_name, number, belong_to FROM users LIMIT ?;");
    }

    // 打开数据库
    if (open_database(USER_DB, &db))
    {
        log_message(LOGLEVEL_ERROR, "无法打开数据库 %s", USER_DB);
        return 1; // 打开数据库失败
    }

    // 启用外键支持
    sqlite3_exec(db, "PRAGMA foreign_keys = ON;", NULL, NULL, NULL);

    // 初始化传入的 users_to_return，避免返回出错
    for (int i = 0; i < length; i++)
    {
        strcpy(users_to_return[i].id, "");
    }

    // 准备查询语句
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK)
    {
        log_message(LOGLEVEL_ERROR, "准备 SQL 语句失败：%s", sqlite3_errmsg(db));
        sqlite3_close(db);
        return 1; // 执行失败
    }

    // 绑定参数
    int param_index = 1;
    if (key && strlen(key) > 0 && content && strlen(content) > 0)
    {
        rc = sqlite3_bind_text(stmt, param_index++, content, -1, SQLITE_STATIC);
        if (rc != SQLITE_OK)
        {
            log_message(LOGLEVEL_ERROR, "绑定 content 参数失败：%s", sqlite3_errmsg(db));
            sqlite3_finalize(stmt);
            sqlite3_close(db);
            return 1; // 执行失败
        }
    }

    // 绑定 LIMIT 参数
    rc = sqlite3_bind_int(stmt, param_index, length);
    if (rc != SQLITE_OK)
    {
        log_message(LOGLEVEL_ERROR, "绑定 LIMIT 参数失败：%s", sqlite3_errmsg(db));
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
        const unsigned char *hashpass_text = sqlite3_column_text(stmt, 2);
        const unsigned char *salt_text = sqlite3_column_text(stmt, 3);
        int role = sqlite3_column_int(stmt, 4);
        const unsigned char *name_text = sqlite3_column_text(stmt, 5);
        const unsigned char *class_name_text = sqlite3_column_text(stmt, 6);
        unsigned int number = (unsigned int)sqlite3_column_int(stmt, 7);
        const unsigned char *belong_to_text = sqlite3_column_text(stmt, 8);

        if (id_text && username_text && hashpass_text && salt_text && name_text && class_name_text && belong_to_text)
        {
            strncpy(users_to_return[count].id, (const char *)id_text, sizeof(users_to_return[count].id) - 1);
            users_to_return[count].id[sizeof(users_to_return[count].id) - 1] = '\0'; // 确保字符串终止

            strncpy(users_to_return[count].username, (const char *)username_text, sizeof(users_to_return[count].username) - 1);
            users_to_return[count].username[sizeof(users_to_return[count].username) - 1] = '\0';

            strncpy(users_to_return[count].hashpass, (const char *)hashpass_text, sizeof(users_to_return[count].hashpass) - 1);
            users_to_return[count].hashpass[sizeof(users_to_return[count].hashpass) - 1] = '\0';

            strncpy(users_to_return[count].salt, (const char *)salt_text, sizeof(users_to_return[count].salt) - 1);
            users_to_return[count].salt[sizeof(users_to_return[count].salt) - 1] = '\0';

            users_to_return[count].role = role;

            strncpy(users_to_return[count].name, (const char *)name_text, sizeof(users_to_return[count].name) - 1);
            users_to_return[count].name[sizeof(users_to_return[count].name) - 1] = '\0';

            strncpy(users_to_return[count].class_name, (const char *)class_name_text, sizeof(users_to_return[count].class_name) - 1);
            users_to_return[count].class_name[sizeof(users_to_return[count].class_name) - 1] = '\0';

            users_to_return[count].number = number;

            strncpy(users_to_return[count].belong_to, (const char *)belong_to_text, sizeof(users_to_return[count].belong_to) - 1);
            users_to_return[count].belong_to[sizeof(users_to_return[count].belong_to) - 1] = '\0';

            // 记录当前用户的信息到日志
            log_message(LOGLEVEL_INFO, "查询到用户 %d: id=%s, username=%s, hashpass=%s, salt=%s, role=%d, name=%s, class_name=%s, number=%u, belong_to=%s",
                        count + 1,
                        users_to_return[count].id,
                        users_to_return[count].username,
                        users_to_return[count].hashpass,
                        users_to_return[count].salt,
                        users_to_return[count].role,
                        users_to_return[count].name,
                        users_to_return[count].class_name,
                        users_to_return[count].number,
                        users_to_return[count].belong_to);

            count++;
        }
        else
        {
            log_message(LOGLEVEL_ERROR, "查询到的用户记录存在空字段，跳过该记录");
        }
    }

    if (count > 0)
    {
        log_message(LOGLEVEL_INFO, "成功查询到 %d 条用户信息", count);
    }
    else
    {
        log_message(LOGLEVEL_INFO, "没有找到任何用户信息");
    }

    // 清理和关闭数据库
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return 0; // 执行成功
}
/**
 * @brief 查询数据库 db/examination.db 中所有问题信息（返回多条数据），可按指定键和值进行过滤
 *
 * @param questions_to_return 查询结果返回的 SqlResponseQuestion 结构体类型数组
 * @param length 查询结果数组的大小，同时也是查询结果返回的限制数量，类似于一个外置的LIMIT
 * @param key 可选的过滤键，如果为 NULL 或空字符串，则不进行过滤
 * @param content 可选的过滤值，与 key 对应
 *
 * @return int 函数执行成功与否，成功返回0，否则为1
 */
int query_questions_info_all(struct SqlResponseQuestion *questions_to_return, int length, const char *key, const char *content)
{
    sqlite3 *db;
    sqlite3_stmt *stmt;
    int rc;
    int count = 0;
    char sql[512];

    // 构建SQL语句
    if (key && strlen(key) > 0 && content && strlen(content) > 0)
    {
        // 使用 WHERE 子句进行过滤
        snprintf(sql, sizeof(sql),
                 "SELECT id, exam_id, num1, op, num2 FROM questions WHERE %s = ? LIMIT ?;",
                 key);
    }
    else
    {
        // 无条件查询
        snprintf(sql, sizeof(sql),
                 "SELECT id, exam_id, num1, op, num2 FROM questions LIMIT ?;");
    }

    // 打开数据库
    if (open_database(EXAMINATION_DB, &db))
    {
        return 1;
    }

    // 启用外键支持
    sqlite3_exec(db, "PRAGMA foreign_keys = ON;", NULL, NULL, NULL);

    // 准备查询语句
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK)
    {
        log_message(LOGLEVEL_ERROR, "准备 SQL 语句失败：%s", sqlite3_errmsg(db));
        sqlite3_close(db);
        return 1;
    }

    // 绑定参数
    int param_index = 1;
    if (key && strlen(key) > 0 && content && strlen(content) > 0)
    {
        rc = sqlite3_bind_text(stmt, param_index++, content, -1, SQLITE_STATIC);
        if (rc != SQLITE_OK)
        {
            log_message(LOGLEVEL_ERROR, "绑定 content 参数失败：%s", sqlite3_errmsg(db));
            sqlite3_finalize(stmt);
            sqlite3_close(db);
            return 1;
        }
    }

    // 绑定 LIMIT 参数
    rc = sqlite3_bind_int(stmt, param_index, length); // 绑定 LIMIT 参数
    if (rc != SQLITE_OK)
    {
        log_message(LOGLEVEL_ERROR, "绑定 LIMIT 参数失败：%s", sqlite3_errmsg(db));
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

            questions_to_return[count].num1 = sqlite3_column_int(stmt, 2);
            questions_to_return[count].op = sqlite3_column_int(stmt, 3);
            questions_to_return[count].num2 = sqlite3_column_int(stmt, 4);
            count++;
        }
    }

    if (count > 0)
    {
        log_message(LOGLEVEL_INFO, "成功查询到 %d 条问题信息", count);
    }
    else
    {
        log_message(LOGLEVEL_INFO, "没有找到任何问题信息");
    }

    // 清理和关闭数据库
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return 0;
}

/**
 * @brief 查询数据库 db/scores.db 中所有成绩信息（返回多条数据），可按指定键和值进行过滤
 *
 * @param scores_to_return 查询结果返回的 SqlResponseScore 结构体类型数组
 * @param length 查询结果数组的大小，同时也是查询结果返回的限制数量，类似于一个外置的 LIMIT
 * @param key 可选的过滤键，如果为 NULL 或空字符串，则不进行过滤
 * @param content 可选的过滤值，与 key 对应
 *
 * @return int 函数执行成功与否，成功返回0，否则为1
 */
int query_scores_info_all(struct SqlResponseScore *scores_to_return, int length, const char *key, const char *content)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    int rc;
    int count = 0;
    char sql[512];

    // 定义允许的过滤键，以防止 SQL 注入
    const char *allowed_keys[] = {"id", "exam_id", "user_id", "score", "expired_flag"};
    int num_allowed_keys = sizeof(allowed_keys) / sizeof(allowed_keys[0]);
    int is_valid_key = 0;

    // 验证 key 是否为允许的列名
    if (key && strlen(key) > 0)
    {
        for (int i = 0; i < num_allowed_keys; i++)
        {
            if (strcmp(key, allowed_keys[i]) == 0)
            {
                is_valid_key = 1;
                break;
            }
        }
        if (!is_valid_key)
        {
            log_message(LOGLEVEL_ERROR, "无效的查询键：%s", key);
            return 1;
        }
    }

    // 构建SQL语句
    if (is_valid_key && content && strlen(content) > 0)
    {
        // 使用 WHERE 子句进行过滤
        snprintf(sql, sizeof(sql),
                 "SELECT id, exam_id, user_id, score, expired_flag FROM scores WHERE %s = ? LIMIT ?;",
                 key);
    }
    else
    {
        // 无条件查询
        snprintf(sql, sizeof(sql),
                 "SELECT id, exam_id, user_id, score, expired_flag FROM scores LIMIT ?;");
    }

    // 打开数据库
    if (open_database(SCORES_DB, &db))
    {
        log_message(LOGLEVEL_ERROR, "无法打开数据库：%s", SCORES_DB);
        return 1;
    }

    // 启用外键支持
    rc = sqlite3_exec(db, "PRAGMA foreign_keys = ON;", NULL, NULL, NULL);
    if (rc != SQLITE_OK)
    {
        log_message(LOGLEVEL_ERROR, "启用外键支持失败：%s", sqlite3_errmsg(db));
        sqlite3_close(db);
        return 1;
    }

    // 初始化传入的 scores_to_return，避免返回出错
    for (int i = 0; i < length; i++)
    {
        strcpy(scores_to_return[i].id, "");
        strcpy(scores_to_return[i].exam_id, "");
        strcpy(scores_to_return[i].user_id, "");
        scores_to_return[i].score = 0.0f;
        scores_to_return[i].expired_flag = 0;
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
    int param_index = 1;
    if (is_valid_key && content && strlen(content) > 0)
    {
        // 根据 key 的数据类型选择绑定函数
        if (strcmp(key, "score") == 0)
        {
            // 假设 content 是浮点数
            float float_content = atof(content);
            rc = sqlite3_bind_double(stmt, param_index++, (double)float_content);
        }
        else if (strcmp(key, "expired_flag") == 0)
        {
            // 假设 expired_flag 是整数
            int int_content = atoi(content);
            rc = sqlite3_bind_int(stmt, param_index++, int_content);
        }
        else
        {
            // 其他字段作为字符串处理
            rc = sqlite3_bind_text(stmt, param_index++, content, -1, SQLITE_STATIC);
        }

        if (rc != SQLITE_OK)
        {
            log_message(LOGLEVEL_ERROR, "绑定 content 参数失败：%s", sqlite3_errmsg(db));
            sqlite3_finalize(stmt);
            sqlite3_close(db);
            return 1;
        }
    }

    // 绑定 LIMIT 参数
    rc = sqlite3_bind_int(stmt, param_index, length);
    if (rc != SQLITE_OK)
    {
        log_message(LOGLEVEL_ERROR, "绑定 LIMIT 参数失败：%s", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return 1;
    }

    // 执行查询并填充结果
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

    if (rc != SQLITE_DONE && rc != SQLITE_ROW)
    {
        log_message(LOGLEVEL_ERROR, "执行查询失败：%s", sqlite3_errmsg(db));
    }

    if (count > 0)
    {
        log_message(LOGLEVEL_INFO, "成功查询到 %d 条成绩信息", count);
    }
    else
    {
        log_message(LOGLEVEL_INFO, "没有找到任何成绩信息");
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
        case BIND_TYPE_UINT:
            rc = sqlite3_bind_int64(stmt, i + 1, *(const unsigned int *)bindings[i]);
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
int insert_question_data(const char *question_id, const char *exam_id, int num1, int op, int num2)
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
    const BindType types[] = {BIND_TYPE_TEXT, BIND_TYPE_TEXT, BIND_TYPE_INT, BIND_TYPE_INT, BIND_TYPE_INT};

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
int insert_score_data(const char *score_id, const char *exam_id, const char *user_id, int score, int expired_flag)
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
    const BindType types[] = {BIND_TYPE_TEXT, BIND_TYPE_TEXT, BIND_TYPE_TEXT, BIND_TYPE_INT, BIND_TYPE_INT};

    // 调用通用插入函数
    int result = insert_data_to_db(SCORES_DB, sql, bindings, types, 5);
    log_message(LOGLEVEL_INFO, "成功插入了用户id为 %s 的成绩 %d\n", user_id, score);
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
    const BindType types[] = {BIND_TYPE_TEXT, BIND_TYPE_TEXT, BIND_TYPE_TEXT, BIND_TYPE_TEXT, BIND_TYPE_INT, BIND_TYPE_TEXT, BIND_TYPE_TEXT, BIND_TYPE_UINT, BIND_TYPE_TEXT};

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

    rc = sqlite3_bind_int(stmt, 4, role);
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

    rc = sqlite3_bind_int(stmt, 7, number);
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

// int main()
// {
//     SetConsoleOutputCP(65001);
//     // 查询单个用户测试
//     struct Permission permission = {-1, -1, -1, -1, -1, -1, -1, -1};
//     struct User user = {
//         "",
//         "",
//         -1,
//         "",
//         "",
//         -1,
//         permission,
//         ""};
//     query_user_info("id", "abcdefgh-ijkl-4mno-pqrs-tuvwxyz12345", &user);
//     log_message(LOGLEVEL_INFO, "id=%s, username=%s, role=%d, name=%s, class_name=%s, number=%u, belong_to=%s", user.id, user.username, user.role, user.name, user.class_name, user.number, user.belong_to);
//     // 修改单个用户测试
//     edit_user_data(
//         "438de9e3-3180-44a3-b205-53514076f334",
//         "STUDENT",
//         "660e5568eef2348d55214737232baa7968f4d2a9037904ff20500f4b525d354cd46e65bcc898227aad8539bb13d07d2ca23756ecfa085d948b3e209cdd989af8",
//         "THISISASALTSTRIN",
//         0,
//         "学生",
//         "班级",
//         1234567890,
//         "abcdefgh-ijkl-4mno-pqrs-tuvwxyz12345"
//     );
//     return 0;
// }