# 文件作用说明文档

```
MentalArithmeticApp/
├── include/                            # 自定义头文件
│   ├── app.c                           # 判题逻辑、分数计算逻辑以及题目链表生成逻辑
│   ├── app.h                           # 在 `app.c` 中定义的函数的声明
│   ├── database.c                      # 数据库操作逻辑，包括数据库的增删查改操作
│   ├── database.h                      # 在 `database.h` 中定义的数据库操作逻辑函数的声明
│   ├── model.c                         # 模型函数，主要是用户权限的获取函数
│   ├── model.h                         # 数据库返回模型、用户模型、权限模型和问题模型的定义，以及 `model.c` 中函数的声明
│   ├── utils.c                         # 工具函数，包括日志记录函数和时间获取及格式化函数
│   ├── utils.h                         # 在 `utils.c` 中定义的函数的声明
├── lib/                                # 第三方库
│   ├── sqlite3.c                       # 来自 sqlite 官方的 C 函数
│   ├── sqlite3.h                       # 来自 sqlite 官方的 C 头文件
├── ui/                                 # Flask 程序存放目录
│   ├── route/                          # 此文件夹存放了 Flask 的路由绑定关系
│   │   ├── __init__.py                 # 模块初始化文件（空）
│   │   ├── api.py                      # 所有的 API，包括通用模块调用 API、教师模块调用 API、学生模块调用 API
│   ├── static/                         # 静态文件（js、css、图片）存放目录
│   ├── templates/                      # 此文件夹存放了渲染并返回给前端的 HTML 模板
│   │   ├── public/                     # 通用模板
│   │   │   ├── base.html                # 包括了 HTML 的部分 `<head>` 的元素，主要用于引入必要的 JS 和 CSS，以及设定网站图标和网站标题
│   │   │   ├── cover.html               # 用户登录后进入仪表盘显示的欢迎信息的覆盖层模板，包含了欢迎信息中用户姓名和用户名的渲染占位
│   │   │   ├── nav.html                 # 用户在仪表盘页面看到的顶栏导航栏模板
│   │   │   ├── personal.html            # 用户在导航栏中点击“个人信息”按钮看到的页面模板
│   │   ├── student/                     # 学生视图
│   │   │   ├── achievement.html         # 学生点击导航栏中“查看成绩”按钮时看到的卡片式窗体成绩表单
│   │   │   ├── exam.html                # 学生点击前往测试以及学生正在进行测试时看到的窗口以及部分 JS 逻辑
│   │   ├── teacher/                     # 教师视图
│   │   │   ├── exam.html                # 教师在考试管理视图下看到的所有窗体，包括考试管理、考试添加、考试修改、考试删除，以及部分与导入 XLSX 文件相关的 JS 逻辑
│   │   │   ├── student.html             # 教师在学生管理视图下看到的所有窗体，包括学生管理、学生添加、学生修改、学生删除的页面
│   │   ├── dashboard.html               # 将仪表盘组合起来的引用其他模板的模板，也包括了部分 CSS 和淡出淡入动画帧
│   │   ├── login.html                   # 登录页面的所有元素和 JS、CSS，也是渲染登录页面时调用的模板
│   │   ├── script.html                  # 包含了绝大多数的仪表盘元素显示，以及筛选逻辑、请求发送、alert 弹窗弹出等
│   ├── utils/                           # Flask 下与 DLL 进行连接的连接器以及预定义的工具函数目录
│   │   ├── __init__.py                  # 进行模块的初始化的同时，进行 DLL 链接以及结构体、函数调用的参数绑定、函数调用的返回类型绑定以及 C 结构体的建立的相关逻辑
│   │   ├── app.py                       # 对问题链表的建立、问题链表的随机化以及考生答案的判定、考生成绩的计算相关 C 函数的调用封装
│   │   ├── database.py                  # 对数据库的增删查改的函数的调用进行的封装
│   │   ├── init.py                      # 对整个程序初始化的函数的调用进行的封装
│   │   ├── tools.py                     # 包含一些预定义的工具函数，包括哈希盐的生成、模拟 C 字符串长度计算、结构体数据的拆解等函数
│   ├── app.py                           # 整个程序的前端入口，调用此文件即可完成服务器的开启
├── utils/                              # Model 下的一些功能性程序
│   ├── initializer.c                    # 程序的初始化函数及数据库的初始化函数，进行必要的文件夹建立和数据库表的建立操作
├── build.ps1                           # 编译脚本，包括将 C 文件编译成 DLL、使用 nuitka 将 Flask 编译为 EXE 可执行文件的一系列操作
```



- `MentalArithmeticApp/`        程序根目录
  - `include/`        自定义头文件
    - `app.c` 判题逻辑、分数计算逻辑以及题目链表生成逻辑
    - `app.h` 在`app.c`中定义的函数的声明
    - `database.c` 数据库操作逻辑，包括数据库的增删查改操作
    - `database.h` 在`database.h`中定义的数据库操作逻辑函数的声明
    - `model.c` 模型函数，主要是用户权限的获取函数
    - `model.h` 数据库返回模型、用户模型、权限模型和问题模型的定义，以及`model.c`中函数的声明
    - `utils.c` 工具函数，包括日志记录函数和时间获取及格式化函数
    - `utils.h` 在`utils.c`中定义的函数的声明
  - `lib/`        第三方库
    - `sqlite3.c` 来自sqlite官方的C函数
    - `sqlite3.h` 来自sqlite官方的C头文件
  - `ui/`        flask程序存放目录
    - `route/`	此文件夹存放了flask的路由绑定关系
      - `__init__.py` 模块初始化文件（空）
      - `api.py` 所有的api，包括通用模块调用API、教师模块调用API、学生模块调用API
    - `static/` 静态文件（js、css、图片）存放目录
    - `templates/ `       此文件夹存放了渲染并返回给前端的html模板
      - `public/`        通用模板
        - `base.html`         包括了html的部分`<head>`的元素，主要用于引入必要的js和css，以及设定网站图标和网站标题
        - `cover.html`       用户登录后进入仪表盘显示的欢迎信息的覆盖层模板，包含了欢迎信息中用户姓名和用户名的渲染占位
        - `nav.html`           用户在仪表盘页面看到的顶栏导航栏模板
        - `personal.html`  用户在导航栏中点击“个人信息”按钮看到的页面模板
      - `student/`      学生视图
        - `achievement.html`    学生点击导航栏中“查看成绩”按钮时看到的卡片式窗体成绩表单
        - `exam.html`         学生点击前往测试以及学生正在进行测试时看到的窗口以及部分js逻辑
      - `teacher/`      教师视图
        - `exam.html`        	教师在考试管理视图下看到的所有窗体，包括考试管理、考试添加、考试修改、考试删除，以及部分与导入xlsx文件相关的js逻辑
        - `student.html`          教师在学生管理视图下看到的所有窗体，包括学生管理、学生添加、学生修改、学生删除的页面
      - `dashboard.html`        将仪表盘组合起来的引用其他模板的模板，也包括了部分css和淡出淡入动画帧
      - `login.html`		登录页面的所有元素和js、css，也是渲染登录页面时调用的模板
      - `script.html`              包含了绝大多数的仪表盘元素显示，以及筛选逻辑、请求发送、alert弹窗弹出等
    - `utils/`        flask下与dll进行连接的连接器以及预定义的工具函数目录
      - `__init__.py`        进行模块的初始化的同时，进行dll链接以及结构体、函数调用的参数绑定、函数调用的返回类型绑定以及C结构体的建立的相关逻辑
      - `app.py`		  对问题链表的建立、问题链表的随机化以及考生答案的判定、考生成绩的计算相关C函数的调用封装
      - `database.py`       对数据库的增删查改的函数的调用进行的封装
      - `init.py`               对整个程序初始化的函数的调用进行的封装
      - `tools.py`             包含一些预定义的工具函数，包括哈希盐的生成、模拟C字符串长度计算、结构体数据的拆解等函数
    - `app.py`        整个程序的前端入口，调用此文件即可完成服务器的开启
  - `utils/`        Model下的一些功能性程序
    - `initializer.c` 程序的初始化函数及数据库的初始化函数，进行必要的文件夹建立和数据库表的建立操作
  - `build.ps1` 编译脚本，包括将C文件编译成dll、将flask编译为exe可执行文件的一系列操作


