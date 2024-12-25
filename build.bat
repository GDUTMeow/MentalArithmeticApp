@echo off
chcp 65001
cls

:: 编译和部署 Flask 服务器
:: 检查 Python 和 GCC 是否安装，创建虚拟环境，安装轮子，编译 C 文件，编译 Python 程序

:: 检查 Python 是否安装
echo 正在检查 Python3 是否安装...
python --version > nul 2>&1
if %errorlevel% == 0 (
    echo Python 已经安装
    python --version
) else (
    echo Python 没有安装，请安装后重试。
    exit /b 1
)

:: 检查 GCC 是否安装
echo 正在检查 GCC 是否安装...
gcc --version > nul 2>&1
if %errorlevel% == 0 (
    echo GCC 已经安装
    gcc --version
) else (
    echo GCC 没有安装，请安装后重试。
    exit /b 1
)

:: 检查虚拟环境是否存在
if exist venv (
    echo 检测到已创建的虚拟环境
) else (
    echo 正在创建虚拟环境 venv...
    python -m venv venv > nul 2>&1
    if %errorlevel% == 0 (
        echo 虚拟环境创建成功
    ) else (
        echo 虚拟环境创建失败，请检查错误信息。
        exit /b 1
    )
)

:: 激活虚拟环境
echo 正在激活虚拟环境...
call venv\Scripts\activate
if %errorlevel% == 0 (
    echo 虚拟环境激活成功
) else (
    echo 虚拟环境激活失败，请检查错误信息。
    exit /b 1
)

:: 安装轮子
echo 正在安装必要的轮子...
pip install -r ui/requirements.txt > nul 2>&1
if %errorlevel% == 0 (
    echo 轮子安装成功
) else (
    echo 轮子安装失败，请检查错误信息。
    exit /b 1
)

:: 编译 C 文件
echo 开始编译 dll 文件...
gcc -fdiagnostics-color=always -g -shared utils/initializer.c lib/sqlite3.c \
include/utils.c include/database.c include/app.c include/model.c \
-o initializer.dll > nul 2>&1
if %errorlevel% == 0 (
    echo 编译初始化链接库 initializer.dll 成功！
) else (
    echo 编译初始化链接库 initializer.dll 失败，请检查错误信息。
    exit /b 1
)

gcc -fdiagnostics-color=always -g -shared include/database.c lib/sqlite3.c \
include/utils.c include/app.c include/model.c \
-o database.dll > nul 2>&1
if %errorlevel% == 0 (
    echo 编译数据库链接库 database.dll 成功！
) else (
    echo 编译数据库链接库 database.dll 失败，请检查错误信息。
    exit /b 1
)

gcc -fdiagnostics-color=always -g -shared include/app.c lib/sqlite3.c \
include/utils.c  include/database.c include/model.c \
-o app.dll > nul 2>&1
if %errorlevel% == 0 (
    echo 编译应用链接库 app.dll 成功！
) else (
    echo 编译应用链接库 app.dll 失败，请检查错误信息。
    exit /b 1
)

:: 编译 Python 程序
echo 正在编译 Flask 服务器组件...
nuitka --standalone --include-data-dir=ui/templates=templates ^
--include-data-dir=ui/static=static ^
--include-data-file=database.dll ^
--include-data-file=app.dll ^
--include-data-file=initializer.dll ^
--output-dir=build ^
--lto=yes --assume-yes-for-downloads ui/app.py

:: 完成
echo 编译完成！
pause
