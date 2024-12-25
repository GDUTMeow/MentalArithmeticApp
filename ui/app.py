import os
import sys
import argparse
import json
import jwt
import logging
from logging.handlers import RotatingFileHandler
from flask import (
    Flask,
    render_template,
    send_from_directory,
    request,
    redirect,
    make_response,
    Response
)
import colorlog
import re
import datetime
from route.api import (
    general_api_v1,
    user_api_v1,
    teacher_api_v1,
    student_api_v1,
    student_get_exam_info,
    student_get_score_list,
    teacher_get_all_exams,
    teacher_get_all_students
)
from utils.database import query_user_info, query_score_info, query_scores_info_all
from utils.tools import questions_xlsx_parse
from utils.init import initialize

# 定义常量
LOG_DIR = 'logs'
LOG_FILE = os.path.join(LOG_DIR, 'ui.log')
EXEMPT_PATHS = [
    "/login",
    "/static/",
    "/api/v1/general/"
]
cookie_blacklist = []  # 初始化黑名单列表
JWT_KEY = "GamerNoTitle"  # 请替换为你的实际密钥

# 创建日志目录（如果不存在）
if not os.path.exists(LOG_DIR):
    os.makedirs(LOG_DIR)

class NoColorFilter(logging.Filter):
    """
    过滤器，用于移除日志消息中的 ANSI 转义序列。
    """
    ansi_escape = re.compile(r'\x1B[@-_][0-?]*[ -/]*[@-~]')

    def filter(self, record):
        record.msg = self.ansi_escape.sub('', record.msg)
        return True

def setup_logging():
    """
    配置日志记录器，使其同时输出到控制台和两个文件：
    - ui.log: 每次启动时清空。
    - ui-YYYY-MM-DD-HH-mm-SS.log: 带时间戳的日志文件，支持轮转。
    控制台使用带颜色的格式，文件使用纯文本格式。
    """
    logger = logging.getLogger()
    logger.setLevel(logging.INFO)  # 设置全局日志级别

    # 定义文件日志格式（不带颜色）
    file_formatter = logging.Formatter('%(asctime)s - %(name)s - %(levelname)s - %(message)s')

    # 定义控制台日志格式（带颜色）
    console_formatter = colorlog.ColoredFormatter(
        '%(log_color)s%(asctime)s - %(name)s - %(levelname)s - %(message)s',
        log_colors={
            'DEBUG': 'cyan',
            'INFO': 'green',
            'WARNING': 'yellow',
            'ERROR': 'red',
            'CRITICAL': 'bold_red',
        }
    )

    # 创建控制台处理器并设置格式
    console_handler = logging.StreamHandler()
    console_handler.setLevel(logging.INFO)
    console_handler.setFormatter(console_formatter)

    # 创建并清空 ui.log 文件处理器
    ui_log_file = os.path.join(LOG_DIR, 'ui.log')
    with open(ui_log_file, 'w'):
        pass  # 这将清空文件内容

    file_handler_ui = logging.FileHandler(ui_log_file, mode='a', encoding='utf-8')
    file_handler_ui.setLevel(logging.INFO)
    file_handler_ui.setFormatter(file_formatter)

    # 创建带时间戳的日志文件名
    timestamp = datetime.datetime.now().strftime("%Y-%m-%d-%H-%M-%S")
    timestamped_log_file = os.path.join(LOG_DIR, f'ui-{timestamp}.log')

    # 创建带时间戳的文件处理器（支持轮转）
    file_handler_timestamp = RotatingFileHandler(
        timestamped_log_file,
        maxBytes=10*1024*1024,  # 10MB
        backupCount=5,
        encoding='utf-8'
    )
    file_handler_timestamp.setLevel(logging.INFO)
    file_handler_timestamp.setFormatter(file_formatter)

    # 为带时间戳的文件处理器添加过滤器，移除 ANSI 转义序列
    file_handler_timestamp.addFilter(NoColorFilter())

    # 为 ui.log 文件处理器添加过滤器，移除 ANSI 转义序列
    file_handler_ui.addFilter(NoColorFilter())

    # 添加处理器到根记录器
    logger.addHandler(console_handler)
    logger.addHandler(file_handler_ui)
    logger.addHandler(file_handler_timestamp)

    # 配置 Flask 的内部日志（werkzeug）
    werkzeug_logger = logging.getLogger('werkzeug')
    werkzeug_logger.setLevel(logging.INFO)
    werkzeug_logger.propagate = True  # 让 werkzeug 的日志传播到根记录器
    werkzeug_logger.handlers = []  # 移除默认处理器，避免重复日志
    werkzeug_logger.addHandler(console_handler)
    werkzeug_logger.addHandler(file_handler_ui)
    werkzeug_logger.addHandler(file_handler_timestamp)
    
class StreamToLogger:
    """
    将标准输出或错误流重定向到日志记录器。
    """
    def __init__(self, logger, log_level=logging.INFO):
        self.logger = logger
        self.log_level = log_level
        self.linebuf = ''

    def write(self, buf):
        if isinstance(buf, bytes):
            try:
                buf = buf.decode('utf-8')  # 解码字节字符串
            except UnicodeDecodeError:
                buf = buf.decode('latin-1')  # 回退到其他编码
        buf = buf.strip()
        if buf.startswith('b"') and buf.endswith('"'):
            buf = buf[2:-1]  # 移除 b" 和 " 前缀和后缀
        for line in buf.splitlines():
            self.logger.log(self.log_level, line.rstrip())

    def flush(self):
        pass  # 无需实现

def redirect_print_to_logging():
    """
    将 sys.stdout 和 sys.stderr 重定向到日志记录器。
    """
    stdout_logger = logging.getLogger('STDOUT')
    stderr_logger = logging.getLogger('STDERR')

    sys.stdout = StreamToLogger(stdout_logger, logging.INFO)
    sys.stderr = StreamToLogger(stderr_logger, logging.ERROR)

def is_exempt_path(path: str) -> bool:
    """
    检查请求路径是否在免验证路径中。

    参数：
        path (str): 请求的路径。

    返回：
        bool: 如果路径在免验证路径列表中，返回True；否则返回False。
    """
    for exempt in EXEMPT_PATHS:
        if path.startswith(exempt):
            return True
    return False

# 初始化 Flask 应用
app = Flask(__name__)
app.register_blueprint(general_api_v1)
app.register_blueprint(user_api_v1)
app.register_blueprint(student_api_v1)
app.register_blueprint(teacher_api_v1)
app.template_folder = "templates"

@app.before_request
def before_request_func():
    """
    在每次请求之前执行的函数：
    - 检查请求路径是否需要验证。
    - 验证JWT token的有效性和用户角色。
    - 根据用户角色和路径进行权限控制和重定向。
    """
    logger = logging.getLogger(__name__)
    logger.info(f"Processing request path: {request.path}")

    # 如果请求路径在免验证路径中，跳过验证
    if is_exempt_path(request.path):
        logger.info(f"Path {request.path} is exempt from authentication.")
        return

    # 获取请求中的token
    token = request.cookies.get("token")
    logger.info(f"Token retrieved: {token}")

    # 如果token在黑名单中，重定向到登录页面
    if token in cookie_blacklist:
        logger.warning(f"Token {token} is in blacklist. Redirecting to /login.")
        return redirect("/login")

    if token:
        try:
            # 尝试解码JWT token
            data = jwt.decode(token, JWT_KEY, algorithms=["HS256"])
            logger.info(f"JWT decoded successfully: {data}")
        except jwt.ExpiredSignatureError:
            # 如果JWT过期，重定向到登录页面
            logger.warning("JWT has expired. Redirecting to /login.")
            return redirect("/login")
        except jwt.InvalidTokenError:
            # 如果JWT无效，重定向到登录页面
            logger.warning("Invalid JWT. Redirecting to /login.")
            return redirect("/login")

        # 获取用户角色
        role = data.get("role", -1)
        logger.info(f"User role: {role}")

        # 获取用户请求的路径（小写）
        path = request.path.lower()

        # 角色与路径的权限控制
        if "/student/" in path.lower():
            # 如果路径包含 'student'，要求角色为学生 (0)
            if role != 0:
                logger.warning(f"Access denied for role {role} on path {path}.")
                return "Permission Denied!", 403
        elif "/teacher/" in path.lower():
            # 如果路径包含 'teacher'，要求角色为教师 (1)
            if role != 1:
                logger.warning(f"Access denied for role {role} on path {path}.")
                return "Permission Denied!", 403

        # 通用角色处理
        if role in [0, 1]:
            # 如果用户角色为学生(0)或教师(1)，并且请求路径不是/dashboard，则重定向到/dashboard
            if request.path != "/dashboard" and "/api" not in request.path:
                logger.info(f"Redirecting user role {role} to /dashboard.")
                return redirect("/dashboard")
        elif role == -1:
            # 如果用户未登录，允许访问免验证路径
            logger.info("User is not logged in (role -1).")
            pass
        else:
            # 其他情况，重定向到登录页面
            logger.warning(f"Unknown role {role}. Redirecting to /login.")
            return redirect("/login")
    else:
        # 如果没有token，设置一个默认的未登录token并重定向到登录页面
        logger.info("No token found. Setting default token and redirecting to /login.")
        response = redirect("/login")
        cookie = jwt.encode({"role": -1}, JWT_KEY, algorithm="HS256")
        cookie_age = 604800  # 设置cookie有效期为7天（7*24*60*60秒）
        response.set_cookie("token", cookie, max_age=cookie_age)
        return response

@app.route("/static/<path:filename>")
def serve_static(filename: str) -> Response:
    """
    提供静态文件的路由。

    参数：
        filename (str): 静态文件的路径。

    返回：
        Response: 返回指定的静态文件。
    """
    logger = logging.getLogger(__name__)
    logger.info(f"Serving static file: {filename}")
    return send_from_directory("static", filename)

@app.route("/")
def home_handler() -> Response:
    """
    根路径的处理函数：
    重定向到登录页面。

    返回：
        Response: 重定向响应。
    """
    logger = logging.getLogger(__name__)
    logger.info("Home route accessed. Redirecting to /login.")
    return redirect("/login")

@app.route("/login")
def login_handler() -> Response:
    """
    登录页面的处理函数：
    - 检查用户是否已登录。
    - 如果已登录，重定向到仪表板。
    - 否则，渲染登录页面。

    返回：
        Response: 渲染的登录页面或重定向响应。
    """
    logger = logging.getLogger(__name__)
    # 获取请求中的token
    token = request.cookies.get("token")
    logger.info(f"Login handler accessed. Token: {token}")
    if token:
        try:
            # 尝试解码JWT token
            data = jwt.decode(token, JWT_KEY, algorithms=["HS256"])
            if data.get("role", -1) in [0, 1]:
                # 如果用户已登录，重定向到仪表板
                logger.info(f"User with role {data.get('role')} already logged in. Redirecting to /dashboard.")
                return redirect("/dashboard")
        except jwt.InvalidTokenError:
            # 如果JWT无效，忽略并渲染登录页面
            logger.warning("Invalid JWT during login check.")
            pass
    # 渲染登录页面
    logger.info("Rendering login page.")
    return render_template("login.html")

@app.route("/dashboard")
def render_dashboard() -> Response:
    """
    仪表板页面的处理函数：
    - 根据用户角色显示不同的信息。
    - 学生用户显示考试和成绩信息。
    - 教师用户显示所有考试和学生信息。

    返回：
        Response: 渲染的仪表板页面或重定向响应。
    """
    logger = logging.getLogger(__name__)
    # 获取请求中的token
    token = request.cookies.get("token")
    logger.info(f"Dashboard accessed. Token: {token}")
    if token:
        try:
            # 尝试解码JWT token
            data = jwt.decode(token, JWT_KEY, algorithms=["HS256"])
            role = data.get("role", -1)
            user_id = data.get("id", "")
            logger.info(f"Decoded JWT: role={role}, user_id={user_id}")

            if role in [0, 1] and user_id:
                # 查询用户信息
                user = query_user_info(key="id", content=user_id)
                if user:
                    # 构建用户数据字典
                    user_data = {
                        "name": user.name.decode(),
                        "username": user.username.decode(),
                        "number": user.number,
                        "class_name": user.class_name.decode() if user.class_name else "",
                        "role": user.role,
                        "id": user.id.decode(),
                    }
                    logger.info(f"User data retrieved: {user_data}")

                    if user.role == 0:
                        # 如果用户是学生，获取考试和成绩信息
                        exam = student_get_exam_info(retJSON=1)
                        scores = [
                            item
                            for item in query_scores_info_all(
                                999,
                                key="exam_id",
                                content=exam.get("metadata", {}).get("id"),
                            )
                            if item.id.decode() != ""
                        ]
                        for score in scores:
                            if score.user_id == user.id:
                                exam["done"] = True
                                break
                            exam["done"] = False
                        score = student_get_score_list(retJSON=1)
                        # 渲染学生仪表板页面
                        logger.info(f"Rendering dashboard for student: {user_data['name']}")
                        return render_template(
                            "dashboard.html", user=user_data, exam=exam, score=score
                        )
                    else:
                        # 如果用户是教师，获取所有考试和学生信息
                        exam = {"metadata": {"allow_answer_when_expired": False}}
                        score = {
                            "success": True,
                            "msg": "成绩获取成功",
                            "data": [
                                {
                                    "id": "score_id",
                                    "exam_id": "exam_id",
                                    "exam_name": "exam_name",
                                    "score": -1,
                                    "expired": False,
                                }
                            ],
                        }
                        exams = teacher_get_all_exams(retJSON=1)
                        students = teacher_get_all_students(retJSON=1)
                        # 渲染教师仪表板页面
                        logger.info(f"Rendering dashboard for teacher: {user_data['name']}")
                        return render_template(
                            "dashboard.html",
                            user=user_data,
                            exam=exam,
                            score=score,
                            exams=exams,
                            students=students,
                        )
                else:
                    # 当前查询的用户不存在，认为JWT_KEY遭到泄露，强制弹回登录页面
                    logger.warning(f"User with ID {user_id} not found. Clearing token.")
                    response = make_response(render_template("login.html"))
                    response.delete_cookie("token")
                    return response
            else:
                # 如果用户角色不在允许范围内，渲染登录页面
                logger.warning(f"Invalid role {role}. Redirecting to /login.")
                return render_template("login.html")
        except jwt.InvalidTokenError:
            # 如果JWT无效，忽略并重定向到登录页面
            logger.warning("Invalid JWT during dashboard rendering. Redirecting to /login.")
            pass
    # 如果没有token，重定向到登录页面
    logger.info("No token found. Redirecting to /login.")
    return redirect("/login")

@app.route("/api/v1/user/logout", methods=["POST"])
def logout_handler() -> Response:
    """
    注销用户的处理函数：
    - 将用户的token加入黑名单。
    - 设置一个过期的token或默认的未登录token。
    - 返回注销成功的响应。

    返回：
        Response: JSON格式的响应。
    """
    logger = logging.getLogger(__name__)
    # 获取请求中的token
    token = request.cookies.get("token")
    logger.info(f"Logout handler accessed. Token: {token}")
    if token:
        # 如果token存在，将其加入黑名单
        cookie_blacklist.append(token)
        logger.info(f"Token {token} added to blacklist.")

    # 构建响应体
    response_body = {"success": True, "msg": "Logout successfully."}
    response = make_response(json.dumps(response_body), 200)
    response.headers["Content-Type"] = "application/json"

    # 设置一个过期的token或者默认的未登录token
    response.set_cookie(
        "token", jwt.encode({"role": -1}, JWT_KEY, algorithm="HS256"), max_age=0
    )
    logger.info("Token cookie cleared and default token set.")

    return response

def initialize_application():
    """
    初始化函数：
    - 进行必要的初始化操作，如数据库连接、加载配置等。
    """
    logger = logging.getLogger(__name__)
    logger.info("Initializing the Flask application...")
    initialize()  # 调用你在 utils.init 中定义的初始化函数
    # 这里可以添加更多的初始化操作
    pass

if __name__ == "__main__":
    # 解析命令行参数
    parser = argparse.ArgumentParser(description="运行 Flask 应用，支持自定义主机、端口和调试模式。")
    parser.add_argument('--host', '-H', default='0.0.0.0', help='监听的主机地址 (默认: 0.0.0.0)')
    parser.add_argument('--port', '-p', type=int, default=5000, help='监听的端口号 (默认: 5000)')
    parser.add_argument('--debug', '-d', action='store_true', help='启用调试模式')
    args = parser.parse_args()

    # 配置日志
    setup_logging()
    redirect_print_to_logging()

    # 初始化应用
    initialize_application()

    # 运行 Flask 应用，禁用重新加载器以防止日志重复
    # app.run(host=args.host, port=args.port, debug=args.debug, use_reloader=False)
    app.run(host=args.host, port=args.port, debug=args.debug, use_reloader=True)
