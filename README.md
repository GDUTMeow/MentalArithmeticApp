# MentalArithmeticApp

> [!important]
>
> 本仓库名称灵感的来源于我小学的时候做的龙门书局出版的一本口算练习册——《口算速算》，由[Google Translate翻译](https://translate.google.com/?sl=auto&tl=en&text=%E5%8F%A3%E7%AE%97%E9%80%9F%E7%AE%97&op=translate)成`Mental arithmetic`，故在后面加上App，表示这是一个程序，组合成了`MentalArithmeticApp`

## 更新日志

### 2024/12/6

- 添加了新的SQL返回数据模型`SqlResponseExam`、`SqlResponseQuestion`、`SqlResponseScore`、`SqlResponseUser`，用于处理sql查询返回的各种数据
- 在README中添加了更新日志，并补全了之前的几次更新
- 对SQL数据返回模型的单复数问题进行了纠正

### 2024/12/5

- 将所有函数的定义移入与头文件同名的c格式文件中，不再在头文件中定义函数
- 在头文件中对同名的c文件下的函数进行声明

### 2024/12/4

- 在`model.h`中定义了新的问题模型`Question`以及其附属数据模型`QuestionData`
- 修复了在`model.h`中出现的错误的`getPermission`的函数定义
- 创建并完成了初始化器`initializer`，并调试成功了整个初始化过程，定义了数据库的表和列
- 给初始化过程添加了写入log的功能，并创建`logs`文件夹来存放后续程序运行的各种log
- 加入了函数`judge`来判断用户输入的结果与计算机计算的结果是否准确（精确到两位小数）

### 2024/12/3

- 在`model.h`中定义了新的权限管理模型`Permission`，用于定义用户的各种权限
- 删除了函数`isAdmin`，并放弃管理员`Admin`身份

### 2024/12/2

- 创建了本项目，开始进行功能的拆分
- 创建了基本的目录结构，并创建了`model.h`，用于定义各种模型
- 定义了用户模型（结构体`User`），用于存储用户的各种属性
- 添加了函数`isAdmin`，用于判断用户是否为管理员