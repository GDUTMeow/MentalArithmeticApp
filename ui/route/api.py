from flask import Blueprint, request, jsonify, Response, make_response
from utils.database import (
    query_user_info,
    query_users_info_all,
    User,
    insert_user_data,
    edit_user_data,
    query_exam_info,
    query_exams_info_all,
    query_questions_info_all,
    insert_score_data
)
from utils.app import generate_question_list, randomize_question_list, traverse_question_list
from utils.tools import generate_salt, calculate_score
from hashlib import sha512
import jwt
import random
import uuid
import string
import time

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
            body = {"success": False, "msg": "用户名或密码不匹配！"}
        else:
            user = user[0]
            if (
                user.username == username.encode()
                and user.hashpass.decode()
                == sha512((user.salt.decode() + password).encode()).hexdigest()
            ):
                body = {"success": True, "msg": "登录成功"}
            else:
                body = {"success": False, "msg": "用户名或密码不匹配！"}
    else:
        body = {"success": False, "msg": "用户名或密码不能为空！"}
    response = make_response(body)
    if body.get("success"):
        cookie = jwt.encode(
            {"role": user.role, "id": user.id.decode()}, JWT_KEY, algorithm="HS256"
        )
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
    if name and number and username and password:
        user = query_user_info(key="username", content=username)
        if user.id:  # 当查到id不为空的时候，说明此用户存在
            body = {
                "success": False,
                "msg": f"当前使用的用户名 {username} 已经被使用了！",
            }
        else:
            if len(username) < 3:
                body = {"success": False, "msg": f"用户名 {username} 太短啦 😣"}
                return body
            if len(username) > 24:
                body = {"success": False, "msg": f"用户名 {username} 太长啦 😣"}
                return body
            for i in username:  # 检测用户名是否合法
                if i not in (string.ascii_letters + string.digits):
                    body = {
                        "success": False,
                        "msg": f"用户名 {username} 中包含非法字符 {i} 😦",
                    }
                    return body
            if int(number) > 4294967295:
                body = {
                    "success": False,
                    "msg": f"工号 {number} 太长了，看起来不是合法的工号 😦",
                }
                return body
            if int(number) <= 0:
                body = {
                    "success": False,
                    "msg": f"你输入了一个非法的工号 {number}，请不要尝试在这里玩栈溢出 😥",
                }
                return body
            if len(name.encode()) > 45:
                body = {
                    "success": False,
                    "msg": f"你的名字 {name} 太长啦，看起来不是合法的名字 😦",  # 按照公安部的规定，中文人名最长为15个汉字
                }
            else:
                user_id = str(uuid.uuid4())
                username = username
                salt = generate_salt()
                hashpass = sha512((salt + password).encode()).hexdigest()
                role = 1
                name = name
                class_name = ""
                number = number
                belong_to = ""
                if insert_user_data(
                    user_id,
                    username,
                    hashpass,
                    salt,
                    role,
                    name,
                    class_name,
                    number,
                    belong_to,
                ):
                    body = {"success": True, "msg": "注册成功"}
                else:
                    body = {"success": False, "msg": "请查看日志获取详细信息 😦"}

    else:
        body = {"success": False, "msg": "发送的数据中未正确填写各项信息 😦"}
    response = make_response(body)
    return response


@user_api_v1.route("/api/v1/user/modifyPassword", methods=["POST"])
def user_modify_password() -> Response:
    data = request.json
    user_id = data.get("userId")
    original_password = data.get("originalPassword")
    new_password = data.get("newPassword")
    user = query_users_info_all(1, key="id", content=user_id)
    if user:
        user = user[0]
        if (
            user.hashpass.decode()
            == sha512((user.salt.decode() + original_password).encode()).hexdigest()
        ):
            salt = generate_salt()
            hashpass = sha512((salt + new_password).encode()).hexdigest()
            if edit_user_data(
                user.id.decode(),
                user.username.decode(),
                hashpass,
                salt,
                user.role,
                user.name.decode(),
                user.class_name.decode(),
                user.number,
                user.belong_to.decode(),
            ):
                body = {"success": True, "msg": "已成功修改密码"}
            else:
                body = {
                    "success": False,
                    "msg": "未知原因修改失败！请查看 log 文件内容获取更多信息！",
                }
        else:
            body = {"success": False, "msg": "原密码不正确，请重新输入！"}
    else:
        body = {"success": False, "msg": "请求错误！请检查请求参数！"}
    return jsonify(body)


@student_api_v1.route("/api/v1/student/getExamInfo")
def student_get_exam_info(retJSON: int = 0) -> Response | dict:
    exams = query_exams_info_all(999)
    print(exams)
    if exams:
        # 按开始时间升序排序
        exams.sort(key=lambda x: x.start_time)
        now = time.time()

        active_exam = None
        next_exam = None

        # 选择的考试逻辑说明
        # 优先选择正在激活的考试（当前时间戳大于开始时间且小于结束时间），否则选择临近开始最近的考试
        # 如果没有临近开始的考试，则返回无考试
        for exam in exams:
            if exam.start_time <= now < exam.end_time:
                active_exam = exam
                break  # 找到激活的考试后退出循环
            elif exam.start_time > now and next_exam is None:
                next_exam = exam
                # 不中断循环，继续检查是否有激活的考试

        if active_exam:
            exam_to_return = active_exam
        elif next_exam:
            exam_to_return = next_exam
        else:
            exam_to_return = None

        if exam_to_return:
            body = {
                "success": True,
                "metadata": {
                    "id": exam_to_return.id.decode(),
                    "name": exam_to_return.name.decode(),
                    "start_time": exam_to_return.start_time,
                    "end_time": exam_to_return.end_time,
                    "allow_answer_when_expired": exam_to_return.allow_answer_when_expired,
                    "random_question": exam_to_return.random_question,
                },
                "data": [],
            }
        else:
            body = {
                "success": False,
                "metadata": {
                    "id": "",
                    "name": "没有即将进行的考试",
                    "start_time": -1,
                    "end_time": -1,
                    "allow_answer_when_expired": -1,
                    "random_question": -1,
                },
                "data": [],
            }
    else:
        body = {
            "success": False,
            "metadata": {
                "id": "",
                "name": "没有即将进行的考试",
                "start_time": -1,
                "end_time": -1,
                "allow_answer_when_expired": -1,
                "random_question": -1,
            },
            "data": [],
        }
    return body if retJSON else jsonify(body)


@student_api_v1.route("/api/v1/student/getExamData/<uuid:UUID>")
def student_get_exam_data(UUID: str) -> Response:
    exam = query_exam_info(key="id", content=str(UUID))
    if exam:
        body = {
            "success": True,
            "metadata": {
                "id": exam.id.decode(),
                "name": exam.name.decode(),
                "start_time": exam.start_time,
                "end_time": exam.end_time,
                "allow_answer_when_expired": exam.allow_answer_when_expired,
                "random_question": exam.random_question,
            },
            "data": [],
        }
        questions = [item for item in query_questions_info_all(999, key="exam_id", content=str(UUID)) if item.id.decode() != ""]
        original_question_list = traverse_question_list(questions)
        if exam.random_question:
            randomize_question_list = original_question_list.copy()
            seed = int(time.time())
            random.seed(int(seed))
            body["metadata"]["seed"] = seed
            for index in range(len(randomize_question_list))[::-1]: # Fisher-Yates
                random_index = random.randint(0, len(questions) - 1)    # randint是双端闭区间，所以要-1
                randomize_question_list[index], randomize_question_list[random_index] = randomize_question_list[random_index], randomize_question_list[index]
            body["data"] = randomize_question_list
        else:
            body["data"] = original_question_list
    else:
        body = {
            "success": False,
            "msg": f"错误！未找到id为 {str(UUID)} 的考试！",
            "metadata": {
                "id": "",
                "name": "没有即将进行的考试",
                "start_time": -1,
                "end_time": -1,
                "allow_answer_when_expired": -1,
                "random_question": -1,
                "seed": -1
            },
            "data": [],
        }
    return jsonify(body)

@student_api_v1.route("/api/v1/student/examSubmit", methods=["POST"])
def student_exam_submit():
    data = request.json
    exam_id: str = data.get("id")
    answers: list[str] = map(float, data.get("answers"))
    seed: int = data.get("seed", 0)
    user_id = jwt.decode(request.cookies.get("token"), JWT_KEY, algorithms=["HS256"]).get("id")
    exam = query_exam_info(key="id", content=exam_id)
    if seed:
        questions = [item for item in query_questions_info_all(999, key="exam_id", content=exam_id) if item.id.decode() != ""]
        original_question_list = traverse_question_list(questions)
        randomize_question_list = original_question_list.copy()
        for index in range(len(randomize_question_list))[::-1]: # Fisher-Yates
            random_index = random.randint(0, len(questions) - 1)    # randint是双端闭区间，所以要-1
            randomize_question_list[index], randomize_question_list[random_index] = randomize_question_list[random_index], randomize_question_list[index]
        question_list = randomize_question_list
    else:
        question_list = [item for item in query_questions_info_all(999, key="exam_id", content=exam_id) if item.id.decode() != ""]
    score = calculate_score(question_list, answers)
    insert_score_data(str(uuid.uuid4()), user_id, exam_id, score, 1 if time.time() > exam.end_time else 0)
    return jsonify({"success": True, "msg": "提交成功！", "score": score})

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
