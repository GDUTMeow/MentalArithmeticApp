/*
Copyright © GamerNoTitle 2024. All rights reserved.
File name: model.h
Author: 吴沛熹      ID: GamerNoTitle    Version: v1.0   Date:
Description:    本头文件定义了三种不同的用户类型：学生、老师、管理员，并对它们的权限等级进行校验
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
 */

/**************************** 用户模型和权限部分 ****************************/

/**
 * @brief 定义变量类型Permission，作为用户的权限结构
 * 
 */
struct Permission
{
    int stuAnswer;  // 学生权限：答题
    int stuInspectPersonalInfo; // 学生权限：查看个人信息、成绩
    int stuInspectExamInfo; // 学生权限：查看考试信息
    int teaManageExam; // 教师权限：管理考试（包括新建、修改、删除）
    int teaManageStudent;   // 教师权限：管理学生（包括导入、修改、深处）
    int teaInspectStudentInfo;  // 教师权限：查看学生信息（包括个人信息和成绩）
    int teaInspectExamScores;   // 教师权限：查看成绩单
    int generalEditInfo;    // 通用：更改个人凭据（邮箱、密码）
};

/**
 * @brief 定义新的变量类型User，对用户内部的属性进行规范
 * 
 */
struct User
{
    char id[37];         // 以UUID4的格式：xxxxxxxx-xxxx-4xxx-yxxx-xxxxxxxxxxxx，其长度为36
                         // 但是字符串后面还要塞\0，所以定义成37
    char username[25];   // 用户名，范围为[a-zA-Z0-9]{3,24}
    int role;            // 用户类型标识符，0,1,2分别对应学生，老师
    char name[46];       // 按照公安部的规定，中文人名最长为15个汉字，而一个汉字在UTF-8中是三个字节
                         // 所以这里按照规定，得到最长人名的字节数为45，再塞一个\0得到46长度
    char class_name[31];      // 按照个人目前所见到的学校的班级名称，按照10个汉字算
    unsigned int number; // 因为按照我这一届的学号为312400XXXX，已经十位数字了
                         // 而且学号只有正数，所以在4294967295的范围内能存下
                         // 工号按照辅导员的五位来看，完全足够
    struct Permission permission;   // 使用上面定义的结构体Permission来决定用户的权限
};

/**
 * @brief 定义函数getPermission，用于获取用户的权限，返回包含用户权限数据的Permission结构体
 * 
 * Parameter:
 *  @user   User类型的结构体，通过获取User.role来决定用户的权限
 * 
 * Returns:
 *  @permission Permission类型的结构体，里面包含了用户的权限
 */
struct Permission getPermission(struct User user)
{
    struct Permission currentPermission;
    switch (user.role)  // 0为学生，1为老师
    {
        case 0: // 学生权限
            currentPermission.stuAnswer = 1;
            currentPermission.stuInspectPersonalInfo = 1;
            currentPermission.stuInspectExamInfo = 1;
            currentPermission.teaManageExam = 0;
            currentPermission.teaManageStudent = 0;
            currentPermission.teaInspectStudentInfo = 0;
            currentPermission.teaInspectExamScores = 0;
            currentPermission.generalEditInfo = 1;
            break;
        case 1: // 老师权限
            currentPermission.stuAnswer = 0;
            currentPermission.stuInspectPersonalInfo = 0;
            currentPermission.stuInspectExamInfo = 0;
            currentPermission.teaManageExam = 1;
            currentPermission.teaManageStudent = 1;
            currentPermission.teaInspectStudentInfo = 1;
            currentPermission.teaInspectExamScores = 1;
            currentPermission.generalEditInfo = 1;
            break;
        default:    // 未知用户类型，为了安全起见，除了更改个人信息的权限，其他权限一律不给
            currentPermission.stuAnswer = 0;
            currentPermission.stuInspectPersonalInfo = 0;
            currentPermission.stuInspectExamInfo = 0;
            currentPermission.teaManageExam = 0;
            currentPermission.teaManageStudent = 0;
            currentPermission.teaInspectStudentInfo = 0;
            currentPermission.teaInspectExamScores = 0;
            currentPermission.generalEditInfo = 1;
            break;
    }
    return currentPermission;
}

/**************************** 用户模型和权限部分结束 ****************************/

/**************************** 问题模型部分 ****************************/
/**
 * @brief 定义结构体Question，用于定义考试中的每个问题的节点
 * 
 * @details 因为考虑可以做到随机题目出现顺序，所以用链表来存储整个问题的列表
 *          所以在这里使用Question结构体，来作为问题的节点，便于后期进行随机化
 * 
 */
struct Question{
    /**
     * @brief 定义结构体QuestionData，用于存储问题
     * 
     * @details QuestionData包括 num1 (int), op (char), num2 (int)
     *          其中 num1 (int) 表示这个算式中的第一个数字
     *          op (char) 表示这个算式中的运算符，仅限 + - * /
     *          num2 (int) 是这个算式中的第二个数字
     * 
     */
    struct QuestionData
    {
        int num1;   // 第一个操作数
        char op;    // 运算符
        int num2;   // 第二个操作数
    };
    struct QuestionData data;   // 问题的数据部分，类型为上面定义的 QuestionData
    struct Question *nextQuestion;  // 问题链的下一个问题
};
/**************************** 问题模型部分结束 ****************************/