from flask import (
    Flask,
    render_template,
    send_from_directory,
    request,
    redirect,
    make_response,
    url_for,
    blueprints,
    Response
)
import json
import jwt
import os
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

app = Flask(__name__)
app.register_blueprint(general_api_v1)
app.register_blueprint(user_api_v1)
app.register_blueprint(student_api_v1)
app.register_blueprint(teacher_api_v1)

app.template_folder = "templates"
UPLOAD_FOLDER = "uploads"
app.config["UPLOAD_FOLDER"] = UPLOAD_FOLDER
# 确保上传目录存在
if not os.path.exists(UPLOAD_FOLDER):
    os.makedirs(UPLOAD_FOLDER)

cookie_blacklist = (
    []
)  # 当用户登出的时候，把cookie丢进来，表示已经失效的cookie，因为JWT是没有状态的

JWT_KEY = "GamerNoTitle"

# 定义不需要身份验证的路径
EXEMPT_PATHS = [
    "/login",
    "/static/",
]


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


@app.before_request
def before_request_func():
    """
    在每次请求之前执行的函数：
    - 检查请求路径是否需要验证。
    - 验证JWT token的有效性和用户角色。
    - 根据用户角色和路径进行权限控制和重定向。
    """
    # 如果请求路径在免验证路径中，跳过验证
    if is_exempt_path(request.path):
        return
    # 获取请求中的token
    token = request.cookies.get("token")

    # 如果token在黑名单中，重定向到登录页面
    if token in cookie_blacklist:
        return redirect("/login")

    if token:
        try:
            # 尝试解码JWT token
            data = jwt.decode(token, JWT_KEY, algorithms=["HS256"])
        except jwt.ExpiredSignatureError:
            # 如果JWT过期，重定向到登录页面
            return redirect("/login")
        except jwt.InvalidTokenError:
            # 如果JWT无效，重定向到登录页面
            return redirect("/login")

        # 获取用户角色
        role = data.get("role", -1)

        # 获取用户请求的路径
        path = request.path.lower()
        # 角色与路径的权限控制
        if "student" in path:
            # 如果路径包含 'student'，要求角色为学生 (0)
            if role != 0:
                return "Permission Denied!"
        elif "teacher" in path:
            # 如果路径包含 'teacher'，要求角色为教师 (1)
            if role != 1:
                return "Permission Denied!"

        # 通用角色处理
        if role in [0, 1]:
            # 如果用户角色为学生(0)或教师(1)，并且请求路径不是/dashboard，则重定向到/dashboard
            if request.path != "/dashboard":
                return redirect("/dashboard")
        elif role == -1:
            # 如果用户未登录，允许访问免验证路径
            pass
        else:
            # 其他情况，重定向到登录页面
            return redirect("/login")
    else:
        # 如果没有token，设置一个默认的未登录token并重定向到登录页面
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
    return send_from_directory("static", filename)


@app.route("/")
def home_handler() -> Response:
    """
    根路径的处理函数：
    重定向到登录页面。
    
    返回：
        Response: 重定向响应。
    """
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
    # 获取请求中的token
    token = request.cookies.get("token")
    if token:
        try:
            # 尝试解码JWT token
            data = jwt.decode(token, JWT_KEY, algorithms=["HS256"])
            if data.get("role", -1) in [0, 1]:
                # 如果用户已登录，重定向到仪表板
                return redirect("/dashboard")
        except jwt.InvalidTokenError:
            # 如果JWT无效，忽略并渲染登录页面
            pass
    # 渲染登录页面
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
    # 获取请求中的token
    token = request.cookies.get("token")
    if token:
        try:
            # 尝试解码JWT token
            data = jwt.decode(token, JWT_KEY, algorithms=["HS256"])
            role = data.get("role", -1)
            user_id = data.get("id", "")
            
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
                    response = make_response(render_template("login.html"))
                    response.delete_cookie("token")
                    return response
            else:
                # 如果用户角色不在允许范围内，渲染登录页面
                return render_template("login.html")
        except jwt.InvalidTokenError:
            # 如果JWT无效，忽略并重定向到登录页面
            pass
    # 如果没有token，重定向到登录页面
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
    # 获取请求中的token
    token = request.cookies.get("token")
    if token:
        # 如果token存在，将其加入黑名单
        cookie_blacklist.append(token)
    
    # 构建响应体
    response_body = {"success": True, "msg": "Logout successfully."}
    response = make_response(json.dumps(response_body), 200)
    response.headers["Content-Type"] = "application/json"
    
    # 设置一个过期的token或者默认的未登录token
    response.set_cookie(
        "token", jwt.encode({"role": -1}, JWT_KEY, algorithm="HS256"), max_age=0
    )
    
    return response


if __name__ == "__main__":
    # 运行Flask应用，监听所有网络接口的5000端口，启用debug模式
    app.run(host="0.0.0.0", port=5000, debug=True)
