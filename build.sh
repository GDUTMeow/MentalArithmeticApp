#!/bin/bash

# 编译和部署 Flask 服务器
# 检查 Python 和 GCC 是否安装，创建虚拟环境，安装轮子，编译 C 文件，编译 Python 程序

# 检查 Python 是否安装
echo "正在检查 Python3 是否安装..."
python3 --version > /dev/null 2>&1
if [ $? -eq 0 ]; then
    echo "Python 已经安装"
    python3 --version
else
    echo "Python 没有安装，请安装后重试。"
    exit 1
fi

# 检查 GCC 是否安装
echo "正在检查 GCC 是否安装..."
gcc --version > /dev/null 2>&1
if [ $? -eq 0 ]; then
    echo "GCC 已经安装"
    gcc --version
else
    echo "GCC 没有安装，请安装后重试。"
    exit 1
fi

# 检查虚拟环境是否存在
if [ -d venv ]; then
    echo "检测到已创建的虚拟环境"
else
    echo "正在创建虚拟环境 venv..."
    python3 -m venv venv > /dev/null 2>&1
    if [ $? -eq 0 ]; then
        echo "虚拟环境创建成功"
    else
        echo "虚拟环境创建失败，请检查错误信息。"
        exit 1
    fi
fi

# 激活虚拟环境
echo "正在激活虚拟环境..."
source venv/bin/activate
if [ $? -eq 0 ]; then
    echo "虚拟环境激活成功"
else
    echo "虚拟环境激活失败，请检查错误信息。"
    exit 1
fi

# 安装轮子
echo "正在安装必要的轮子..."
pip install -r ui/requirements.txt > /dev/null 2>&1
if [ $? -eq 0 ]; then
    echo "轮子安装成功"
else
    echo "轮子安装失败，请检查错误信息。"
    exit 1
fi

# 编译 C 文件
echo "开始编译 dll 文件..."
gcc -fdiagnostics-color=always -g -shared utils/initializer.c lib/sqlite3.c \
include/utils.c include/database.c include/app.c include/model.c \
-o initializer.so > /dev/null 2>&1
if [ $? -eq 0 ]; then
    echo "编译初始化链接库 initializer.so 成功！"
else
    echo "编译初始化链接库 initializer.so 失败，请检查错误信息。"
    exit 1
fi

gcc -fdiagnostics-color=always -g -shared include/database.c lib/sqlite3.c \
include/utils.c include/app.c include/model.c \
-o database.so > /dev/null 2>&1
if [ $? -eq 0 ]; then
    echo "编译数据库链接库 database.so 成功！"
else
    echo "编译数据库链接库 database.so 失败，请检查错误信息。"
    exit 1
fi

gcc -fdiagnostics-color=always -g -shared include/app.c lib/sqlite3.c \
include/utils.c  include/database.c include/model.c \
-o app.so > /dev/null 2>&1
if [ $? -eq 0 ]; then
    echo "编译应用链接库 app.so 成功！"
else
    echo "编译应用链接库 app.so 失败，请检查错误信息。"
    exit 1
fi

# 编译 Python 程序
echo "正在编译 Flask 服务器组件..."
nuitka --standalone --include-data-dir=ui/templates=templates \
--include-data-dir=ui/static=static \
--include-data-file=database.so \
--include-data-file=app.so \
--include-data-file=initializer.so \
--output-dir=build \
--lto=yes --assume-yes-for-downloads ui/app.py

# 完成
echo "编译完成！"
