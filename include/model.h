/*
Copyright © GamerNoTitle 2024. All rights reserved.
File name: model.h
Author: 吴沛熹      ID: GamerNoTitle    Version: v1.0   Date: 2024/12/4
Description:    本头文件定义了两种不同的用户类型：学生、老师，并对它们的权限等级进行校验
                以及定义了问题模型，让问题以链表形式存储，便于后期进行随机排序
Others:         暂无
History:        暂无
    1.  Date: 2024/12/2
        Author: 吴沛熹
        ID: GamerNoTitle
        Modification:   [+] 新建了文件，并加入了头部文件注释，说明本头文件的功能
                        [+] 添加了新的结构体User，来定义用户，存储用户的各种属性
                        [+] 添加了新的函数isAdmin，用于判断用户是否是管理员（鉴权使用）
    2.  Date: 2024/12/3
        Author: 吴沛熹
        ID: GamerNoTitle
        Modification:   [-] 删除了函数isAdmin，并去掉了管理员类型用户
                        [+] 新增了权限管理结构体Permission，用于定义用户的各种权限
    3.  Date: 2024/12/4
        Author: 吴沛熹
        ID: GamerNoTitle
        Modification:   [+] 添加了新的函数getPermission，用于在初始化用户的时候快速获取用户权限
                        [+] 添加了问题模型Question及其附属模型QuestionData，以并采用链表节点格式
                        [+] 添加了计算函数calculate_result，函数内自带转换为float计算，并且采用了两位小数的比较方法
                        [*] 将函数的所有定义部分都移入同名的c文件
    4.  Date: 2024/12/6
        Author: 吴沛熹
        ID: GamerNoTitle
        Modification:   [+] 添加了新的数据库返回结构体 SqlResponseExam、SqlResponseQuestions、
                            SqlResponseScores、SqlResponseUser，以便于在database.c的函数使用
                            此四种结构体进行SQL查询返回数据的解析
 */

#include <math.h>

/**************************** 用户模型和权限部分 ****************************/

/**
 * @brief 定义变量类型Permission，作为用户的权限结构
 *
 */
struct Permission
{
    int stu_answer;                // 学生权限：答题
    int stu_inspect_personal_info; // 学生权限：查看个人信息、成绩
    int stu_inspect_exam_info;     // 学生权限：查看考试信息
    int tea_manage_exam;           // 教师权限：管理考试（包括新建、修改、删除）
    int tea_manage_student;        // 教师权限：管理学生（包括导入、修改、删除）
    int tea_inspect_student_info;  // 教师权限：查看学生信息（包括个人信息和成绩）
    int tea_inspect_exam_scores;   // 教师权限：查看成绩单
    int general_edit_info;         // 通用：更改个人凭据（邮箱、密码）
};

/**
 * @brief 定义新的变量类型User，对用户内部的属性进行规范
 *
 */
struct User
{
    char id[37];                  // 以UUID4的格式：xxxxxxxx-xxxx-4xxx-xxxx-xxxxxxxxxxxx，其长度为36
                                  // 但是字符串后面还要塞\0，所以定义成37
    char username[25];            // 用户名，范围为[a-zA-Z0-9]{3,24}
    int role;                     // 用户类型标识符，0,1,2分别对应学生，老师
    char name[46];                // 按照公安部的规定，中文人名最长为15个汉字，而一个汉字在UTF-8中是三个字节
                                  // 所以这里按照规定，得到最长人名的字节数为45，再塞一个\0得到46长度
    char class_name[31];          // 按照个人目前所见到的学校的班级名称，按照10个汉字算
    unsigned int number;          // 因为按照我这一届的学号为312400XXXX，已经十位数字了
                                  // 而且学号只有正数，所以在4294967295的范围内能存下
                                  // 工号按照辅导员的五位来看，完全足够
    struct Permission permission; // 使用上面定义的结构体Permission来决定用户的权限
    char belong_to[37];           // (仅学生) 属于哪一位老师，填入老师的UUID，可以为空
};

struct Permission get_permission(struct User user);

/**************************** 用户模型和权限部分结束 ****************************/

/**************************** 问题模型部分 ****************************/

/**
 * @brief 定义结构体Question，用于定义考试中的每个问题的节点
 *
 * @details 因为考虑可以做到随机题目出现顺序，所以用链表来存储整个问题的列表
 *          所以在这里使用Question结构体，来作为问题的节点，便于后期进行随机化
 *
 */
struct Question
{
    /**
     * @brief 定义结构体QuestionData，用于存储问题
     *
     * @details QuestionData包括 num1 (int), op (char), num2 (int)
     *          其中 num1 (int) 表示这个算式中的第一个数字
     *          op (int) 表示这个算式中的运算符，仅限 + - * /，分别对应数字0123
     *          num2 (int) 是这个算式中的第二个数字
     *
     */
    struct QuestionData
    {
        float num1; // 第一个操作数
        int op;     // 运算符
        float num2; // 第二个操作数
    };
    struct QuestionData data;       // 问题的数据部分，类型为上面定义的 QuestionData
    struct Question *next_question; // 问题链的下一个问题
};

/**************************** 问题模型部分结束 ****************************/

/**************************** 数据库结果返回开始 ****************************/

/**
 * @brief 通过SQL查询考次表得到的结果，采用下面这个模型进行存储，便于后期进行数据的读取
 *
 */
struct SqlResponseExam
{
    char id[37];                   // 考试的唯一ID，采用UUID4格式
    char name[91];                 // 考试的名称，预留30个汉字的空位，加多一个位置放入\0
    int start_time;                // 考试的开始时间
    int end_time;                  // 考试的结束时间
    int allow_answer_when_expired; // 是否允许逾期作答，只允许0（不允许）和1（允许）
    int random_question;           // 是否开启随机题目顺序，只允许0（不允许）和1（允许）
};

/**
 * @brief 通过SQL查询问题表得到的结果，采用下面这个模型进行存储，便于后期进行题目的装载
 *
 */
struct SqlResponseQuestion
{
    char id[37];      // 题目的唯一ID，采用UUID4格式
    char exam_id[37]; // 题目对应的考试ID，仍然是UUID4
    float num1;       // 第一个操作数
    int op;           // 运算符，0123对应加减乘除
    float num2;       // 第二个操作数
};

/**
 * @brief 通过SQL查询成绩表得到的结果，采用下面这个模型进行存储，便于后期进行成绩的引用
 *
 */
struct SqlResponseScore
{
    char id[37];      // 成绩的唯一ID，采用UUID4格式
    char exam_id[37]; // 成绩对应的考试ID，UUID4
    char user_id[37]; // 成绩对应的用户ID，UUID4
    float score;      // 用户的成绩
    int expired_flag; // 用户是否逾期作答，只允许0（正常作答）和1（逾期作答）
};

/**
 * @brief 通过SQL查询用户表得到的结果，采用下面这个模型进行存储，便于后期转换为User类
 *
 */
struct SqlResponseUser
{
    char id[37];         // 用户的唯一ID，采用UUID4格式
    char username[25];   // 用户自行设定的用户名，可用于登录，范围为[a-zA-Z0-9]{3,24}，只允许字母数字
    char hashpass[129];  // 用户密码采用 sha512(salt + passwd) 计算后的值
                         // sha512计算结果长度为128，所以定义129，保留\0的位置
    char salt[17];       // 本程序中，盐采用16字节（128位）的规范，同时此长度不会占用过多的空间
    int role;            // 用户角色
    char name[46];       // 用户的真实姓名
    char class_name[31]; // 用户的班级名字
    unsigned int number; // 用户的学号/工号
    char belong_to[37]; // 用户归属的老师（如果有）
};

/**************************** 数据库结果返回结束 ****************************/