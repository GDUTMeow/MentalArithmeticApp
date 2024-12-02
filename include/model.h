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
        Modification:   新建了文件，并加入了头部文件注释，说明本头文件的功能
                        [+] 添加了新的结构体User，来定义用户，存储用户的各种属性
                        [+] 添加了新的函数isAdmin，用于判断用户是否是管理员（鉴权使用）
 */


/**
 * @brief 定义新的变量类型User，对用户内部的属性进行规范
 * 
 */
struct User
{
    char id[37];         // 以UUID4的格式：xxxxxxxx-xxxx-4xxx-yxxx-xxxxxxxxxxxx，其长度为36
                         // 但是字符串后面还要塞\0，所以定义成37
    char username[25];   // 用户名，范围为[a-zA-Z0-9]{3,24}
    int role;            // 用户类型标识符，0,1,2分别对应学生，老师，管理员
    char name[46];       // 按照公安部的规定，中文人名最长为15个汉字，而一个汉字在UTF-8中是三个字节
                         // 所以这里按照规定，得到最长人名的字节数为45，再塞一个\0得到46长度
    char class[31];      // 按照个人目前所见到的学校的班级名称，按照10个汉字算
    unsigned int number; // 因为按照我这一届的学号为312400XXXX，已经十位数字了
                         // 而且学号只有正数，所以在4294967295的范围内能存下
                         // 工号按照辅导员的五位来看，完全足够
};

/**
 * @brief 判断某用户是否是管理员
 *
 * @return int
 *         管理员：1，非管理员：0
 */
int isAdmin(struct User user)
{
    if (user.role == 0)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}