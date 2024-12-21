/*
Copyright © GamerNoTitle 2024. All rights reserved.
File name: database.h
Author: 吴沛熹      ID: GamerNoTitle    Version: v1.0   Date: 2024/12/4
Description:    本文件用于声明预设的一些sql操作函数，包括成绩自定范围查询，学生的添加、修改、删除
                考试的查看、添加、删除，个人密码的修改等
Others:         暂无
History:        暂无
    1.  Date: 2024/12/6
        Author: 吴沛熹
        ID: GamerNoTitle
        Modification:   [+] 新建了本文件
                        [+] 进行了 query_user_info 函数的声明
    2.  Date: 2024/12/21
        Author: 吴沛熹
        ID: GamerNoTitle
        Modification:   [+] 添加了忘记添加的函数声明
                        [+] 添加了头文件包含保护
 */

#ifndef DATABASE_H
#define DATABASE_H

#include <stdio.h>
#include "model.h"
#include "../lib/sqlite3.h"

typedef enum BINDING
{
    BIND_TYPE_TEXT,
    BIND_TYPE_INT,
    BIND_TYPE_FLOAT
} BindType;

int open_database(const char *db_path, sqlite3 **db);
int query_user_info(const char key[], const char content[], struct User *user_to_return, FILE *log_file);
int query_exam_info(const char *key, const char *content, struct SqlResponseExam *exam_to_return);
int query_question_info(const char *key, const char *content, struct SqlResponseQuestion *question_to_return);
int query_score_info(const char *key, const char *content, struct SqlResponseScore *score_to_return);
int query_exams_info_all(struct SqlResponseExam *exams_to_return, int length, const char *key, const char *content);
int query_users_info_all(struct SqlResponseUser *users_to_return, int length, const char *key, const char *content);
int query_questions_info_all(struct SqlResponseQuestion *questions_to_return, int length, const char *key, const char *content);
int query_scores_info_all(struct SqlResponseScore *scores_to_return, int length, const char *key, const char *content);
int insert_data_to_db(const char *db_path, const char *sql, const void **bindings, const BindType *types, int num_bindings);
int insert_exam_data(const char *exam_id, const char *name, int start_time, int end_time, int allow_answer_when_expired, int random_question);
int insert_question_data(const char *question_id, const char *exam_id, int num1, int op, int num2);
int insert_score_data(const char *score_id, const char *exam_id, const char *user_id, float score, int expired_flag);
int insert_user_data(const char *user_id, const char *username, const char *hashpass, const char *salt, int role, const char *name, const char *class_name, int number, const char *belong_to);
int del_user_data(const char *user_id);
int del_exam_data(const char *exam_id);
int del_score_data(const char *score_id);
int del_question_data(const char *question_id);
int edit_user_data(const char *user_id, const char *username, const char *hashpass, const char *salt, int role, const char *name, const char *class_name, int number, const char *belong_to);
int edit_exam_data(const char *exam_id, const char *name, int start_time, int end_time, int allow_answer_when_expired, int random_question);
int edit_score_data(const char *score_id, const char *exam_id, const char *user_id, int score, int expired_flag);
int edit_score_data(const char *score_id, const char *exam_id, const char *user_id, int score, int expired_flag);


#endif