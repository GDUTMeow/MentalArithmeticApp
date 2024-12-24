from flask import (
    Flask,
    render_template,
    send_from_directory,
    request,
    redirect,
    make_response,
    url_for,
    blueprints,
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
    teacher_get_all_exams
)
from utils.database import query_user_info, query_score_info, query_scores_info_all
from utils.tools import questions_xlsx_parse

app = Flask(__name__)
app.register_blueprint(general_api_v1)
app.register_blueprint(user_api_v1)
app.register_blueprint(student_api_v1)
app.register_blueprint(teacher_api_v1)

app.template_folder = "templates"
UPLOAD_FOLDER = 'uploads'
app.config['UPLOAD_FOLDER'] = UPLOAD_FOLDER
# 确保上传目录存在
if not os.path.exists(UPLOAD_FOLDER):
    os.makedirs(UPLOAD_FOLDER)

cookie_blacklist = (
    []
)  # 当用户登出的时候，把cookie丢进来，表示已经失效的cookie，因为JWT是没有状态的

key = "GamerNoTitle"

# 定义不需要身份验证的路径
EXEMPT_PATHS = [
    "/login",
    "/static/",
    "/",
]


def is_exempt_path(path):
    """检查请求路径是否在免验证路径中"""
    for exempt in EXEMPT_PATHS:
        if path.startswith(exempt):
            return True
    return False


@app.before_request
def before_request_func():
    # 如果请求路径在免验证路径中，跳过验证
    if is_exempt_path(request.path):
        return

    token = request.cookies.get("token")
    if token in cookie_blacklist:
        # 如果token在黑名单中，重定向到登录
        return redirect("/login")

    if token:
        try:
            data = jwt.decode(token, key, algorithms=["HS256"])
        except jwt.ExpiredSignatureError:
            # JWT过期
            return redirect("/login")
        except jwt.InvalidTokenError:
            # JWT无效
            return redirect("/login")

        role = data.get("role", -1)
        if role in [0, 1]:
            # 如果用户已经在访问/dashboard，无需重定向
            if request.path != "/dashboard":
                return redirect("/dashboard")
        elif role == -1:
            # 未登录用户，允许访问免验证路径
            pass
        else:
            return redirect("/login")
    else:
        # 没有token，设置一个默认的未登录token并重定向到登录
        response = redirect("/login")
        cookie = jwt.encode({"role": -1}, key, algorithm="HS256")
        cookie_age = 604800  # 7*24*60*60
        response.set_cookie("token", cookie, max_age=cookie_age)
        return response


@app.route("/static/<path:filename>")
def serve_static(filename):
    return send_from_directory("static", filename)


@app.route("/")
def home_handler():
    return redirect("/login")


@app.route("/login")
def login_handler():
    token = request.cookies.get("token")
    if token:
        try:
            data = jwt.decode(token, key, algorithms=["HS256"])
            if data.get("role", -1) in [0, 1]:
                return redirect("/dashboard")
        except jwt.InvalidTokenError:
            pass
    return render_template("login.html")


@app.route("/dashboard")
def render_dashboard():
    token = request.cookies.get("token")
    if token:
        try:
            data = jwt.decode(token, key, algorithms=["HS256"])
            if data.get("role", -1) in [0, 1]:
                if data.get("id", ""):
                    user = query_user_info(key="id", content=data.get("id"))
                    if user:
                        user_data = {
                            "name": user.name.decode(),
                            "username": user.username.decode(),
                            "number": user.number,
                            "class_name": (
                                user.class_name.decode() if user.class_name else ""
                            ),
                            "role": user.role,
                            "id": user.id.decode(),
                        }
                        print(user_data)
                        if user.role == 0:
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
                            return render_template(
                                "dashboard.html", user=user_data, exam=exam, score=score
                            )
                        else:
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
                            return render_template(
                                "dashboard.html", user=user_data, exam=exam, score=score, exams=exams
                            )
                    else:
                        # 当前查询的用户不存在，认为JWT_KEY遭到泄露，强制弹回登录页面
                        response = make_response(render_template("login.html"))
                        response.delete_cookie("token")
                        return response
                else:
                    return render_template("login.html")
            else:
                return render_template("login.html")
        except jwt.InvalidTokenError:
            pass
    return redirect("/login")


@app.route("/api/v1/user/logout", methods=["POST"])
def logout_handler():
    token = request.cookies.get("token")
    if token:
        cookie_blacklist.append(token)
    response_body = {"success": True, "msg": "Logout successfully."}
    response = make_response(json.dumps(response_body), 200)
    response.headers["Content-Type"] = "application/json"
    # 设置一个过期的token或者默认的未登录token
    response.set_cookie(
        "token", jwt.encode({"role": -1}, key, algorithm="HS256"), max_age=0
    )
    return response


if __name__ == "__main__":
    print("Student token:", jwt.encode({"role": 0}, key, algorithm="HS256"))
    print("Teacher token:", jwt.encode({"role": 1}, key, algorithm="HS256"))
    app.run(host="0.0.0.0", port=5000, debug=True)
