from flask import Blueprint, request, jsonify, Response, make_response 
from utils.database import query_user_info, query_users_info_all, User, insert_user_data
from hashlib import sha512
import jwt
import random
import uuid
import string

JWT_KEY = "GamerNoTitle"

user_api_v1 = Blueprint("user", __name__)

student_api_v1 = Blueprint("student", __name__)

teacher_api_v1 = Blueprint("teacher", __name__)

general_api_v1 = Blueprint("general", __name__)

@general_api_v1.route("/api/v1/general/login", methods=["POST"])
def general_login() -> Response:
    """用户登录函数：因为在C里面既要进行哈希又要进行登录的计算等操作太麻烦了，所以统一移动到中间件来完成"""
    data = request.json
    body = {"success": False, "msg": "init"}
    username: str = data.get("username", "")
    password: str = data.get("password", "")
    if username and password:
        user = query_users_info_all(1, key="username", content=username)
        if len(user) == 0:
            # 查询失败，没有符合条件的条目
            body = {
                "success": False,
                "msg": "用户名或密码不匹配！"
            }
        else:
            user = user[0]
            if user.username == username.encode() and user.hashpass.decode() == sha512((user.salt.decode() + password).encode()).hexdigest():
                body = {
                    "success": True,
                    "msg": "登录成功"
                }
            else:
                body = {
                    "success": False,
                    "msg": "用户名或密码不匹配！"
                }
    else:
        body = {
            "success": False,
            "msg": "用户名或密码不能为空！"
        }
    response = make_response(body)
    if body.get("success"):
        cookie = jwt.encode({"role": user.role, "username": user.username.decode()}, JWT_KEY, algorithm="HS256")
        cookie_age = 604800  # 7*24*60*60
        response.set_cookie("token", cookie, max_age=cookie_age)
    return response


@general_api_v1.route("/api/v1/general/register", methods=["POST"])
def general_register() -> Response:
    data = request.json
    name: str = data.get("name", "")
    number: int = int(data.get("number", "0"))
    username: str = data.get("username", "")
    password: str = data.get("password", "")
    print(int(number))
    if name and number and username and password:
        user = query_user_info(key="username", content=username)
        if user.id: # 当查到id不为空的时候，说明此用户存在
            body = {
                "success": False,
                "msg": f"当前使用的用户名 {username} 已经被使用了！"
            }
        else:
            if len(username) < 3:
                body = {
                    "success": False,
                    "msg": f"用户名 {username} 太短啦！"
                }
                return body
            if len(username) > 24:
                body = {
                    "success": False,
                    "msg": f"用户名 {username} 太长啦！"
                }
                return body
            for i in username:  # 检测用户名是否合法
                if i not in (string.ascii_letters + string.digits):
                    body = {
                        "success": False,
                        "msg": f"用户名 {username} 中包含非法字符 {i}！"
                    }
                    return body
            if int(number) > 4294967295:
                body = {
                    "success": False,
                    "msg": f"工号 {number} 太长了，看起来不是合法的工号！"
                }
                return body
            if int(number) <= 0:
                body = {
                    "success": False,
                    "msg": f"你输入了一个非法的工号 {number}，请不要尝试在这里玩栈溢出！"
                }
                return body
            if len(name.encode()) > 45:
                body = {
                    "success": False,
                    "msg": f"你的名字 {name} 太长啦，看起来不是合法的名字！"    # 按照公安部的规定，中文人名最长为15个汉字
                }
            else:
                user_id = str(uuid.uuid4())
                username = username
                salt = "".join(random.choices(string.ascii_letters + string.digits, k=16))
                hashpass = sha512((salt + password).encode()).hexdigest()
                role = 1
                name = name
                class_name = ""
                number = number
                belong_to = ""
                if insert_user_data(user_id, username, hashpass, salt, role, name, class_name, number, belong_to):
                    body = {
                        "success": True,
                        "msg": "注册成功"
                    }
                else:
                    body = {
                        "success": False,
                        "msg": "请查看日志获取详细信息！"
                    }
            
    else:
        body = {
            "success": False,
            "msg": "发送的数据中未正确填写各项信息！"
        }
    response = make_response(body)
    return response

@user_api_v1.route("/api/v1/user/getInfo")
def user_get_info():
    pass


@user_api_v1.route("/api/v1/user/logout")
def user_logout():
    pass


@user_api_v1.route("/api/v1/user/changePassword", methods=["POST"])
def user_change_password():
    pass


@student_api_v1.route("/api/v1/student/getExamInfo")
def student_get_exam_info():
    pass


@student_api_v1.route("/api/v1/student/getExamData")
def student_get_exam_data():
    pass


@student_api_v1.route("/api/v1/student/getScoreList")
def student_get_score_list():
    pass


@teacher_api_v1.route("/api/v1/teacher/addExam", methods=["POST"])
def teacher_add_exam():
    pass


@teacher_api_v1.route("/api/v1/teacher/getExam/<UUID>")
def teracher_get_exam(UUID: str):
    pass


@teacher_api_v1.route("/api/v1/teacher/deleteExams", methods=["POST"])
def teacher_delete_exam():
    pass


@teacher_api_v1.route("/api/v1/teacher/getExamScores/<UUID>")
def teacher_get_exam_scores(UUID: str):
    pass


@teacher_api_v1.route("/api/v1/teacher/addStudents", methods=["POST"])
def teacher_add_students():
    pass


@teacher_api_v1.route("/api/v1/teacher/deleteStudents", methods=["POST"])
def teacher_delete_students():
    pass


@teacher_api_v1.route("/api/v1/teacher/getStudent/<UUID>")
def teacher_get_student(UUID: str):
    pass


@teacher_api_v1.route("/api/v1/teacher/modifyStudent", methods=["POST"])
def teacher_modify_student():
    pass
