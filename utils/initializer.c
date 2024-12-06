/*
Copyright © GamerNoTitle 2024. All rights reserved.
File name: initializer.c
Author: 吴沛熹      ID: GamerNoTitle    Version: v1.0   Date: 2024/12/4
Description:    本文件用于初始化整个程序，当这个程序第一次运行的时候，运行本初始化程序
Others:         暂无
History:        暂无
    1.  Date: 2024/12/4
        Author: 吴沛熹
        ID: GamerNoTitle
        Modification: [+] 新建了文件，并加入了头部文件注释，说明本头文件的功能
                      [+] 新建了函数initialize()，用于进行程序的初始化
                      [+] 新建了函数initialize_database()用来进行数据库的初始化
                      [+] 添加了将日志保存到logs文件夹的特定文件的功能
 */

#include "../lib/sqlite3.h"
#include <io.h>
#include <direct.h>
#include <stdio.h>
#include <errno.h>
#include <windows.h>

#include "../include/utils.h" // 引入自己写的头文件utils.h，来调用里面已经写好的一些trick函数

/*** 文件读取等级 ***/
#define ACCESS_EXIST_MODE 0 // 访问模式，0为是否存在，在_access函数中使用
#define ACCESS_RW_MODE 6    // 访问模式，6代表是否有读写权限（读取为2，写入为4），在_access函数中使用

/*** 数据库部分 ***/
#define DB_FOLDER "db"  // 数据库保存文件夹名
#define EXAMINATION_DB "db/examination.db"  // 考试数据库
#define SCORES_DB "db/scores.db"    // 成绩数据库
#define USER_DB "db/user.db"    // 用户数据库

/*** 日志部分 ***/
#define LOG_FOLDER "logs"                  // 日志文件夹路径
#define LOG_FILE "logs/initialization.log" // 日志文件路径
// 日志等级
#define LOGLEVEL_INFO "INFO"
#define LOGLEVEL_ERROR "ERROR"

/**
 * @brief 数据库初始化函数，用于初始化不同的数据库
 *
 * @param db_filename 数据库文件路径
 * @param command   数据库初始化命令，只运行一次，如果有多条命令需要调用多次这个函数
 * @param log_file  日志文件，用于保存初始化日志情况
 */
void initialize_database(const char *db_filename, const char *command, FILE *log_file)
{
    // 定义变量存储时间
    char current_time[20];

    // 定义数据库指针变量
    sqlite3 *db;
    char *err_msg = 0;

    // 打开数据库
    int rc = sqlite3_open(db_filename, &db);
    if (rc)
    {
        get_current_time(current_time, sizeof(current_time));
        fprintf(log_file, "%s [%s]: 打开数据库 '%s' 失败：%s\n", current_time, LOGLEVEL_ERROR, db_filename, sqlite3_errmsg(db));
        sqlite3_close(db);
        return;
    }

    // 创建表
    rc = sqlite3_exec(db, command, 0, 0, &err_msg);
    if (rc != SQLITE_OK)
    {
        get_current_time(current_time, sizeof(current_time));
        fprintf(log_file, "%s [%s]: 创建表失败：%s\n", current_time, LOGLEVEL_ERROR, err_msg);
        sqlite3_free(err_msg); // 运行失败后释放占用的内存
    }
    else
    {
        get_current_time(current_time, sizeof(current_time));
        fprintf(log_file, "%s [%s]: 对数据库 '%s' 初始化命令 %s运行成功\n", current_time, LOGLEVEL_INFO, db_filename, command);
    }

    sqlite3_close(db); // 关闭数据库连接
}

/**
 * @brief 初始化函数，当程序运行时，执行初始化操作，且将日志保存到文件中
 */
void initialize()
{
    // 检查是否存在 logs 文件夹
    if (_access(LOG_FOLDER, ACCESS_EXIST_MODE) != 0)
    {
        if (_mkdir(LOG_FOLDER) != 0)
        {
            printf("无法创建 logs 文件夹：错误码 %d，%s\n", errno, strerror(errno));
            exit(1); // 如果无法创建日志文件夹，初始化失败，退出程序
        }
    }

    // 打开日志文件，如果不存在则创建
    FILE *log_file = fopen(LOG_FILE, "a"); // 'a' 表示附加模式
    if (log_file == NULL)
    {
        printf("无法打开日志文件：%s\n", strerror(errno));
        return;
    }

    // 定义时间变量
    char current_time[20] = {'\0'};

    // 尝试创建 db 文件夹
    if (_access(DB_FOLDER, ACCESS_EXIST_MODE) != 0) // 当 db 文件夹不存在时
    {
        if (!_mkdir(DB_FOLDER)) // 创建文件夹
        {
            get_current_time(current_time, sizeof(current_time));
            fprintf(log_file, "%s [%s]: 文件夹 '%s' 创建成功。\n", current_time, LOGLEVEL_INFO, DB_FOLDER); // 创建成功的时候，写入日志
            fprintf(log_file, "%s [%s]: 正在尝试初始化数据库。\n", current_time, LOGLEVEL_INFO);
            fprintf(log_file, "%s [%s]: 正在初始化考试数据库。\n", current_time, LOGLEVEL_INFO);
            char examination_init_command[] = "CREATE TABLE examinations(\n"
                                              "id TEXT PRIMARY KEY   NOT NULL,\n"         // 考试ID，UUID，唯一键
                                              "name TEXT             NOT NULL,\n"         // 考次名称
                                              "start_time INT        NOT NULL,\n"         // 考试的开始时间，时间戳
                                              "end_time INT          NOT NULL,\n"         // 考试的结束时间，时间戳
                                              "allow_answer_when_expired INT NOT NULL,\n" // 是否允许逾期作答
                                              "random_question INT   NOT NULL\n"          // 是否开启问题乱序
                                              ");\n";
            initialize_database(EXAMINATION_DB, examination_init_command, log_file);
            char questions_init_command[] = "CREATE TABLE questions(\n"
                                            "id TEXT PRIMARY KEY    NOT NULL,\n" // 问题ID，UUID，唯一键
                                            "exam_id TEXT           NOT NULL,\n" // 问题作用的考试ID，对应上面考次的UUID
                                            "num1 INTEGER           NOT NULL,\n" // 第一个操作数字
                                            "op INTEGER             NOT NULL,\n" // 运算符，0123对应加减乘除
                                            "num2 INTEGER           NOT NULL\n"  // 第二个操作数字
                                            ");\n";
            initialize_database(EXAMINATION_DB, questions_init_command, log_file);
            fprintf(log_file, "%s [%s]: 正在初始化成绩数据库。\n", current_time, LOGLEVEL_INFO);
            char scores_init_command[] = "CREATE TABLE scores(\n"
                                         "id TEXT PRIMARY KEY    NOT NULL,\n" // 成绩ID，UUID，唯一键
                                         "exam_id TEXT           NOT NULL,\n" // 考试ID，对应上面考次的UUID
                                         "user_id TEXT           NOT NULL,\n" // 用户ID，成绩所对应的用户的UUID
                                         "score INTEGER          NOT NULL,\n" // 成绩
                                         "expired_flag INTEGER   NOT NULL\n"  // 是否逾期作答，01分别代表否、是
                                         ");\n";
            initialize_database(SCORES_DB, scores_init_command, log_file);
            fprintf(log_file, "%s [%s]: 正在初始化用户数据库。\n", current_time, LOGLEVEL_INFO);
            char users_init_command[] = "CREATE TABLE users(\n"
                                        "id TEXT PRIMARY KEY        NOT NULL,\n" // 用户ID，UUID，唯一键
                                        "username TEXT              NOT NULL,\n" // 用户名，范围为[a-zA-Z0-9]{3, 24}
                                        "hashpass TEXT              NOT NULL,\n" // 哈希后的密码
                                        "salt TEXT                  NOT NULL,\n" // 盐
                                        "role INTEGER               NOT NULL,\n" // 用户角色
                                        "name TEXT                  NOT NULL,\n" // 用户姓名
                                        "class TEXT,\n"                          // 用户班级
                                        "number INTEGER             NOT NULL\n"  // 用户学号/工号
                                        ");\n";
            initialize_database(USER_DB, users_init_command, log_file);
        }
        else
        {
            // 新建出错了，将出错信息写入日志并退出程序
            get_current_time(current_time, sizeof(current_time));
            fprintf(log_file, "%s [%s]: 初始化失败，创建文件夹 '%s' 时遇到问题：错误码 %d，%s\n", current_time, LOGLEVEL_ERROR, DB_FOLDER, errno, strerror(errno));
            exit(1);
        }
    }
    else
    {
        get_current_time(current_time, sizeof(current_time));
        fprintf(log_file, "%s [%s]: 文件夹 '%s' 已存在。\n", current_time, LOGLEVEL_INFO, DB_FOLDER); // 文件夹已经存在时记录日志
    }
    get_current_time(current_time, sizeof(current_time));
    fprintf(log_file, "%s [%s]: 初始化完成！\n", current_time, LOGLEVEL_INFO); // 文件夹已经存在时记录日志
    // 关闭日志文件
    fclose(log_file);
}

/**
 * @brief **仅用于调试**，用于调试初始化整个项目
 *
 * @return int 程序运行结束状态
 */
int main()
{
    SetConsoleOutputCP(65001);
    initialize();
    return 0;
}