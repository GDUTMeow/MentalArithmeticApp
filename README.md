# MentalArithmeticApp

> [!important]
>
> 本仓库名称灵感的来源于我小学的时候做的龙门书局出版的一本口算练习册——《口算速算》，由[Google Translate 翻译](https://translate.google.com/?sl=auto&tl=en&text=%E5%8F%A3%E7%AE%97%E9%80%9F%E7%AE%97&op=translate)成 `Mental arithmetic`，故在后面加上 App，表示这是一个程序，组合成了 `MentalArithmeticApp`

## 更新日志

### 2024/12/28

- 修改了分数的计算方式，现在它能够正确地计算出100分了
- 在用户添加页面下，添加了单个学生添加的默认密码提示
- 将默认密码设置为八个0（单个添加的时候）
- 修了修改考试的过程中，错误的考试时间重叠校验逻辑

### 2024/12/25

- 完成了教师视图下，对单次考试成绩的查询
- 完成了教师视图下，对学生的单个导入和批量导入
- 完成了教师对于学生的各种管理操作
- 在Cookie的token中添加了随机值 `token`，用于跟已经退出登录的令牌进行区分
- 将打包脚本改为 Powershell 脚本，以避免来自Command Prompt的编码问题
- 在 `app.c` 中添加了函数的声明，因为把释放内存的函数丢在下面了，不加就编译不通过了
- 将 `/api/v1/general/*` 加入免验证清单，避免出现注册和登录被拦住的问题
- 在部分位置的js中添加了 `location.reload()` 来达到刷新页面获取最新内容的效果
- 把仪表盘中的网站标题同一为 `口算速算 | MentalArithmeticApp`
- 在顶上的导航栏里添加了Github图标，指向本仓库
- 加入了程序图标，并在打包脚本中加入了更多的程序信息
- 添加了一个 `calculate_lines.ps1` 用于计算自己写的代码行数
- 修复了添加单个学生的时候，弹出错误的重复提示

### 2024/12/24

- 完成了教师视图下的新建考试、删除考试和修改考试的操作
- 添加了题目模板、学生导入模板
- 给模板下载添加了入口添加了nuitka的打包脚本，以便后期进行程序的编译
- 在 `requirements.txt`中添加了更多的轮子
- 在 `utils/tools`下添加了用于解析xlsx题目和学生信息的两个解析函数
- 修复了错误的 `display`设置
- 更改了模板中的前期用于占位的变量名为实际返回的变量名
- 给教师视图下添加考试的窗口中的输入框及按钮添加独特表示 `exam-add-` 以对学生答题页面的相关元素进行区分，保证唯一性
- 将原本预定的 `csv` 表格格式更改为比较 user-friendly 的 `xlsx` 文件
- 修复了连接器中对 `insert_question_data` 函数的错误的参数类型定义

### 2024/12/23

- 修复了特定条件下返回错误的模板的bug

### 2024/12/21

- 完成了学生对考试答案的提交路由
- 完成了学生初始化考试内容（获取考试题目）的路由，并添加了可选随机化，使用 `Fisher-Yates 算法` 进行问题数组的无偏随机打乱
- （其实是22号凌晨完成的）完成了几乎所有的学生路由和前端页面的显示，并且修复了部分由于变量类型不一致造成的bug

### 2024/12/20

- 完成了学生端进入仪表盘对考试名称及相关信息的渲染
- 对学生端的考试入口进行了JavaScript层面的时间戳转换以及剩余时间计算
- 添加了网站图标
- 完成了对 `student_api_v1` 和 `teacher_api_v1` 的注册
- 修复了 `query_exams_info_all` 的函数调用绑定变量数量不正确的问题

### 2024/12/18

- 修复了用户修改函数 `edit_user_data` 中错误的数据类型绑定导致的修改出错问题
- 修复了仪表盘页面个人信息页面的输入框与教师修改学生信息的输入框的 `id`一致导致的密码修改失败问题
- 完成了用户的密码修改功能绑定
- 修复了Python对C的User结构体绑定中，因为char类型的长度与C设定的不符，导致查询的数据乱套的问题
- 将生成哈希盐的函数写入  `utils/tools.py` 中，以便后期调用
- 修改了欢迎遮罩中信息显示为默认信息的问题
- 定义了两个与构成问题链表相关的函数

### 2024/12/17

- 修改了部分数据库的函数，现在查询所有内容的函数可以限制条件查询了
- 完成登录和注册功能对C函数的绑定
- 给登录表单添加了注册功能，同时使用js动态发送登录/注册请求
- 修复了部分Python包的导入问题
- （其他）写完了学者网要求的程序框架图、函数调用图和函数设计
- （补充）咕咕咕好多天了，彩六能直播了做完去打彩六了，果然鸽子就是会无限咕咕咕的呢~

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
- 将数据库 examination.db 内的 `questions`表中的两个数字改为采用 REAL 类型进行存储
- 稍微摸了一点前端，最终决定为 Flask + jinja2 的方式渲染页面，如果需要，后期可以增加 Electron 打包

### 2024/12/7

- 完成了数据库查询函数 `query_exam_info`, `query_question_info`, `query_score_info`, `query_exams_info_all`, `query_user_info_all`, `query_questions_info_all`, `query_scores_info_all`
- 修复了部分文件中缺失头文件引用的问题
- 对自己编写的头文件在编译过程中添加参数，使其动态加载
- 在初始化程序中，对 user.db 数据库的 users 表格添加了新的列 `belong_to`，表示学生归属的老师，避免串班

### 2024/12/6

- 添加了新的 SQL 返回数据模型 `SqlResponseExam`、`SqlResponseQuestion`、`SqlResponseScore`、`SqlResponseUser`，用于处理 sql 查询返回的各种数据
- 在 README 中添加了更新日志，并补全了之前的几次更新
- 对 SQL 数据返回模型的单复数问题进行了纠正
- 将错误归类的自定义函数放入新建的 `app.c`中

### 2024/12/5

- 将所有函数的定义移入与头文件同名的 c 格式文件中，不再在头文件中定义函数
- 在头文件中对同名的 c 文件下的函数进行声明

### 2024/12/4

- 在 `model.h`中定义了新的问题模型 `Question`以及其附属数据模型 `QuestionData`
- 修复了在 `model.h`中出现的错误的 `getPermission`的函数定义
- 创建并完成了初始化器 `initializer`，并调试成功了整个初始化过程，定义了数据库的表和列
- 给初始化过程添加了写入 log 的功能，并创建 `logs`文件夹来存放后续程序运行的各种 log
- 加入了函数 `judge`来判断用户输入的结果与计算机计算的结果是否准确（精确到两位小数）

### 2024/12/3

- 在 `model.h`中定义了新的权限管理模型 `Permission`，用于定义用户的各种权限
- 删除了函数 `isAdmin`，并放弃管理员 `Admin`身份

### 2024/12/2

- 创建了本项目，开始进行功能的拆分
- 创建了基本的目录结构，并创建了 `model.h`，用于定义各种模型
- 定义了用户模型（结构体 `User`），用于存储用户的各种属性
- 添加了函数 `isAdmin`，用于判断用户是否为管理员

## Credit

- [@ricky8955555 (Phrinky)](https://github.com/ricky8955555)
- [mdbootstrap/mdb-ui-kit: Bootstrap 5 &amp; Material Design UI KIT](https://github.com/mdbootstrap/mdb-ui-kit)
- [iconfont-阿里巴巴矢量图标库](https://www.iconfont.cn/)
- [SQL database engine](https://github.com/sqlite/sqlite)
- [commitizen/cz-conventional-changelog: A commitizen adapter for the angular preset of https://github.com/conventional-changelog/conventional-changelog](https://github.com/commitizen/cz-conventional-changelog)
- [pallets/flask: The Python micro framework for building web applications.](https://github.com/pallets/flask/)
