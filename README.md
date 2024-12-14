# MentalArithmeticApp

> [!important]
>
> 本仓库名称灵感的来源于我小学的时候做的龙门书局出版的一本口算练习册——《口算速算》，由[Google Translate 翻译](https://translate.google.com/?sl=auto&tl=en&text=%E5%8F%A3%E7%AE%97%E9%80%9F%E7%AE%97&op=translate)成`Mental arithmetic`，故在后面加上 App，表示这是一个程序，组合成了`MentalArithmeticApp`

## 更新日志

### 2024/12/14

- 修复了一处由跨语言调用引起的指针错误
- 将问题数据库中的两个操作数列改为INT类型存储

### 2024/12/12

- 完成了 Python 对 C 数据库部分函数的绑定
- 修复了模板中 js 部分的一个语法错误（这个错误会导致元素无法正确隐藏/显示）

### 2024/12/11

- 完成了部分的 flask 的鉴权和 JWT 的签发逻辑
- 完成了学生答题面板（未测试）

### 2024/12/10

- 完成了绝大多数当前阶段能够完成的前端，包括教师考试管理、学生管理；通用个人信息，并完成了对应的 js 代码

### 2024/12/9

- 完成了一部分的前端 html，并添加了学生成绩查看、教师考试管理的窗格

### 2024/12/8

- 完成了数据库的增删改函数，并添加了测试用例
- 将日志写入功能更改为一个函数，避免重复造轮子
- 将数据库 examination.db 内的`questions`表中的两个数字改为采用 REAL 类型进行存储
- 稍微摸了一点前端，最终决定为 Flask + jinja2 的方式渲染页面，如果需要，后期可以增加 Electron 打包

### 2024/12/7

- 完成了数据库查询函数`query_exam_info`, `query_question_info`, `query_score_info`, `query_exams_info_all`, `query_user_info_all`, `query_questions_info_all`, `query_scores_info_all`
- 修复了部分文件中缺失头文件引用的问题
- 对自己编写的头文件在编译过程中添加参数，使其动态加载
- 在初始化程序中，对 user.db 数据库的 users 表格添加了新的列`belong_to`，表示学生归属的老师，避免串班

### 2024/12/6

- 添加了新的 SQL 返回数据模型`SqlResponseExam`、`SqlResponseQuestion`、`SqlResponseScore`、`SqlResponseUser`，用于处理 sql 查询返回的各种数据
- 在 README 中添加了更新日志，并补全了之前的几次更新
- 对 SQL 数据返回模型的单复数问题进行了纠正
- 将错误归类的自定义函数放入新建的`app.c`中

### 2024/12/5

- 将所有函数的定义移入与头文件同名的 c 格式文件中，不再在头文件中定义函数
- 在头文件中对同名的 c 文件下的函数进行声明

### 2024/12/4

- 在`model.h`中定义了新的问题模型`Question`以及其附属数据模型`QuestionData`
- 修复了在`model.h`中出现的错误的`getPermission`的函数定义
- 创建并完成了初始化器`initializer`，并调试成功了整个初始化过程，定义了数据库的表和列
- 给初始化过程添加了写入 log 的功能，并创建`logs`文件夹来存放后续程序运行的各种 log
- 加入了函数`judge`来判断用户输入的结果与计算机计算的结果是否准确（精确到两位小数）

### 2024/12/3

- 在`model.h`中定义了新的权限管理模型`Permission`，用于定义用户的各种权限
- 删除了函数`isAdmin`，并放弃管理员`Admin`身份

### 2024/12/2

- 创建了本项目，开始进行功能的拆分
- 创建了基本的目录结构，并创建了`model.h`，用于定义各种模型
- 定义了用户模型（结构体`User`），用于存储用户的各种属性
- 添加了函数`isAdmin`，用于判断用户是否为管理员
