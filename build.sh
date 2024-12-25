#!/bin/bash

# 设置日志文件路径
LOG_FILE="deploy.log"

# 清空日志文件
> "$LOG_FILE"

# 定义日志函数
write_log() {
    local LEVEL=$1
    local MESSAGE=$2
    local TIMESTAMP
    TIMESTAMP=$(date +"%Y-%m-%d %H-%M-%S")
    local FORMATTED_MESSAGE="$TIMESTAMP [$LEVEL]: $MESSAGE"
    
    case "$LEVEL" in
        INFO)
            echo -e "\e[32m$FORMATTED_MESSAGE\e[0m" | tee -a "$LOG_FILE"
            ;;
        WARN)
            echo -e "\e[33m$FORMATTED_MESSAGE\e[0m" | tee -a "$LOG_FILE"
            ;;
        ERROR)
            echo -e "\e[31m$FORMATTED_MESSAGE\e[0m" | tee -a "$LOG_FILE"
            ;;
        DEBUG)
            echo -e "\e[36m$FORMATTED_MESSAGE\e[0m" | tee -a "$LOG_FILE"
            ;;
        *)
            echo -e "$FORMATTED_MESSAGE" | tee -a "$LOG_FILE"
            ;;
    esac
}

# 定义步骤执行函数
execute_step() {
    local STEP_NAME=$1
    shift
    local STEP_COMMAND="$@"

    write_log "INFO" "开始步骤: $STEP_NAME"
    local STEP_START=$(date +%s)

    # 执行命令
    eval "$STEP_COMMAND"
    local STEP_STATUS=$?

    local STEP_END=$(date +%s)
    local STEP_DURATION=$((STEP_END - STEP_START))

    if [ $STEP_STATUS -eq 0 ]; then
        write_log "INFO" "步骤 '$STEP_NAME' 成功，耗时: ${STEP_DURATION}s"
    else
        write_log "ERROR" "步骤 '$STEP_NAME' 失败，耗时: ${STEP_DURATION}s"
        exit 1
    fi
}

# 记录脚本开始时间
SCRIPT_START=$(date +%s)

# 检查 Python 是否安装
execute_step "检查 Python3 是否安装" "
    if command -v python &> /dev/null; then
        write_log \"INFO\" \"Python 已经安装\"
        python --version | while read -r line; do write_log \"DEBUG\" \"\$line\"; done
    else
        write_log \"ERROR\" \"Python 没有安装，请安装后重试。\"
        exit 1
    fi
"

# 检查 GCC 是否安装
execute_step "检查 GCC 是否安装" "
    if command -v gcc &> /dev/null; then
        write_log \"INFO\" \"GCC 已经安装\"
        gcc --version | head -n1 | while read -r line; do write_log \"DEBUG\" \"\$line\"; done
    else
        write_log \"ERROR\" \"GCC 没有安装，请安装后重试。\"
        exit 1
    fi
"

# 检查虚拟环境是否存在
execute_step "检查或创建虚拟环境" "
    if [ -d \"venv\" ]; then
        write_log \"INFO\" \"检测到已创建的虚拟环境\"
    else
        write_log \"INFO\" \"正在创建虚拟环境 venv...\"
        python -m venv venv
        if [ \$? -eq 0 ]; then
            write_log \"INFO\" \"虚拟环境创建成功\"
        else
            write_log \"ERROR\" \"虚拟环境创建失败，请检查错误信息。\"
            exit 1
        fi
    fi
"

# 激活虚拟环境
execute_step "激活虚拟环境" "
    write_log \"INFO\" \"正在激活虚拟环境...\"
    # shellcheck source=/dev/null
    source venv/bin/activate
    if [ \$? -eq 0 ]; then
        write_log \"INFO\" \"虚拟环境激活成功\"
    else
        write_log \"ERROR\" \"虚拟环境激活失败，请检查错误信息。\"
        write_log \"WARN\" \"您可以尝试运行： Set-ExecutionPolicy RemoteSigned -Scope CurrentUser\"
        exit 1
    fi
"

# 安装依赖
execute_step "安装依赖" "
    write_log \"INFO\" \"正在安装必要的轮子...\"
    pip install -r ui/requirements.txt
    if [ \$? -eq 0 ]; then
        write_log \"INFO\" \"轮子安装成功\"
    else
        write_log \"ERROR\" \"轮子安装失败，请检查错误信息。\"
        exit 1
    fi
"

# 编译 C 文件
write_log "INFO" "开始编译 dll 文件..."

# 编译 initializer.dll
execute_step "编译 initializer.dll" "
    gcc -fdiagnostics-color=always -g -shared utils/initializer.c lib/sqlite3.c \
    include/utils.c include/database.c include/app.c include/model.c \
    -o initializer.dll
"

# 编译 database.dll
execute_step "编译 database.dll" "
    gcc -fdiagnostics-color=always -g -shared include/database.c lib/sqlite3.c \
    include/utils.c include/app.c include/model.c \
    -o database.dll
"

# 编译 app.dll
execute_step "编译 app.dll" "
    gcc -fdiagnostics-color=always -g -shared include/app.c lib/sqlite3.c \
    include/utils.c include/database.c include/model.c \
    -o app.dll
"

# 编译 Python 程序
execute_step "编译 Flask 服务器组件" "
    nuitka --standalone --include-data-dir=ui/templates=templates \
    --include-data-dir=ui/static=static \
    --include-data-file=database.dll=database.dll \
    --include-data-file=app.dll=app.dll \
    --include-data-file=initializer.dll=initializer.dll \
    --output-dir=build \
    --lto=yes --assume-yes-for-downloads ui/app.py
"

# 记录脚本结束时间
SCRIPT_END=$(date +%s)
SCRIPT_DURATION=$((SCRIPT_END - SCRIPT_START))

write_log "INFO" "脚本总执行时间: ${SCRIPT_DURATION}s"

# 完成
write_log "INFO" "编译完成！"

# 暂停（在 Linux 中通常不需要，但可以让用户查看日志）
read -rp "按任意键继续..." -n1 -s
echo
