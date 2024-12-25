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
    insert_score_data,
    query_scores_info_all,
    insert_question_data,
    insert_exam_data,
    delete_exam_data,
    edit_exam_data,
    delete_question_data,
)
from utils.app import (
    generate_question_list,
    randomize_question_list,
    traverse_question_list,
)
from utils.tools import (
    generate_salt,
    calculate_score,
    questions_xlsx_parse,
    students_xlsx_parser,
)
from hashlib import sha512
from datetime import datetime
import jwt
import random
import uuid
import string
import time
import json
from urllib.parse import unquote_to_bytes

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
        questions = [
            item
            for item in query_questions_info_all(999, key="exam_id", content=str(UUID))
            if item.id.decode() != ""
        ]
        original_question_list = traverse_question_list(questions)
        if exam.random_question:
            randomize_question_list = original_question_list.copy()
            seed = int(time.time())
            random.seed(int(seed))
            body["metadata"]["seed"] = seed
            for index in range(len(randomize_question_list))[::-1]:  # Fisher-Yates
                random_index = random.randint(
                    0, len(questions) - 1
                )  # randint是双端闭区间，所以要-1
                (
                    randomize_question_list[index],
                    randomize_question_list[random_index],
                ) = (
                    randomize_question_list[random_index],
                    randomize_question_list[index],
                )
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
                "seed": -1,
            },
            "data": [],
        }
    return jsonify(body)


@student_api_v1.route("/api/v1/student/examSubmit", methods=["POST"])
def student_exam_submit():
    try:
        data = request.json
        exam_id: str = data.get("id")
        answers: list[str] = map(float, data.get("answers"))
        seed: int = int(data.get("seed", 0))
        user_id = jwt.decode(
            request.cookies.get("token"), JWT_KEY, algorithms=["HS256"]
        ).get("id")
        exam = query_exam_info(key="id", content=exam_id)
        if seed:
            questions = [
                item
                for item in query_questions_info_all(
                    999, key="exam_id", content=exam_id
                )
                if item.id.decode() != ""
            ]
            original_question_list = traverse_question_list(questions)
            randomize_question_list = original_question_list.copy()
            random.seed(seed)
            for index in range(len(randomize_question_list))[::-1]:  # Fisher-Yates
                random_index = random.randint(
                    0, len(questions) - 1
                )  # randint是双端闭区间，所以要-1
                (
                    randomize_question_list[index],
                    randomize_question_list[random_index],
                ) = (
                    randomize_question_list[random_index],
                    randomize_question_list[index],
                )
            question_list = randomize_question_list
        else:
            question_list = [
                item
                for item in query_questions_info_all(
                    999, key="exam_id", content=exam_id
                )
                if item.id.decode() != ""
            ]
        score = calculate_score(question_list, answers)
        insert_score_data(
            str(uuid.uuid4()),
            exam_id,
            user_id,
            score,
            1 if time.time() > exam.end_time else 0,
        )
        body = {"success": True, "msg": "提交成功！", "score": score}
    except Exception as e:
        body = {"success": False, "msg": f"提交失败！{e}"}
    return jsonify(body)


@student_api_v1.route("/api/v1/student/getScoreList")
def student_get_score_list(retJSON: int = 0):
    scores = [
        item
        for item in query_scores_info_all(
            999,
            key="user_id",
            content=jwt.decode(
                request.cookies.get("token"), JWT_KEY, algorithms=["HS256"]
            ).get("id"),
        )
        if item.id.decode() != ""
    ]
    exam_list = []
    for score in scores:
        exam_data = query_exam_info(key="id", content=score.exam_id.decode())
        if exam_data:
            exam_list.append(
                {
                    "id": score.id.decode(),
                    "exam_id": score.exam_id.decode(),
                    "exam_name": exam_data.name.decode(),
                    "score": score.score,
                    "expired": score.expired_flag,
                }
            )
    if not exam_list:  # 没有任何成绩
        body = {
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
    else:
        body = {"success": True, "msg": "成绩获取成功", "data": exam_list}
    return body if retJSON else jsonify(body)


@teacher_api_v1.route("/api/v1/teacher/getAllExams")
def teacher_get_all_exams(retJSON=0):
    exams = query_exams_info_all(999)
    if exams:
        body = {
            "success": True,
            "msg": "获取考试列表成功",
            "data": [
                {
                    "id": exam.id.decode(),
                    "name": exam.name.decode(),
                    "start_time": exam.start_time,
                    "end_time": exam.end_time,
                    "allow_answer_when_expired": exam.allow_answer_when_expired,
                    "random_question": exam.random_question,
                    "current_status": (
                        0
                        if time.time() < exam.start_time
                        else (
                            1
                            if time.time() > exam.start_time
                            and time.time() < exam.end_time
                            else -1
                        )
                    ),
                }
                for exam in exams
            ],
        }
    else:
        body = {
            "success": False,
            "msg": "获取考试列表失败",
            "data": [],
        }
    return body if retJSON else jsonify(body)


@teacher_api_v1.route("/api/v1/teacher/addExam", methods=["POST"])
def teacher_add_exam() -> Response:
    exam_data = request.form["examData"]
    exam_data = unquote_to_bytes(exam_data)
    exam_data = json.loads(exam_data)
    file = request.files["xlsxFile"]
    if file == None:
        body = {"success": False, "msg": "未上传文件！"}
        return body
    try:
        exam_uuid = str(uuid.uuid4())
        if insert_exam_data(
            exam_id=exam_uuid,
            name=exam_data.get("examName"),
            start_time=int(
                datetime.strptime(exam_data.get("startDate"), "%Y-%m-%d %H:%M")
                .replace(second=0)
                .timestamp()
            ),
            end_time=int(
                datetime.strptime(exam_data.get("endDate"), "%Y-%m-%d %H:%M")
                .replace(second=0)
                .timestamp()
            ),
            allow_answer_when_expired=int(exam_data.get("allowAnswerWhenExpired")),
            random_question=int(exam_data.get("randomQuestions")),
        ):
            questions = questions_xlsx_parse(file.read())
            for question in questions:
                question_uuid = str(uuid.uuid4())
                insert_question_data(
                    question_id=question_uuid,
                    exam_id=exam_uuid,
                    num1=question[0],
                    op=question[1],
                    num2=question[2],
                )
            body = {"success": True, "msg": "添加考试成功！"}
    except Exception as e:
        body = {"success": False, "msg": f"添加考试失败！{e}"}
    return jsonify(body)


@teacher_api_v1.route("/api/v1/teacher/getExam/<uuid:UUID>")
def teacher_get_exam(UUID: str) -> Response:
    exam = query_exam_info(key="id", content=str(UUID))
    if exam:
        body = {
            "success": True,
            "msg": "获取考试信息成功",
            "data": {
                "id": exam.id.decode(),
                "name": exam.name.decode(),
                "start_time": exam.start_time,
                "end_time": exam.end_time,
                "allow_answer_when_expired": exam.allow_answer_when_expired,
                "random_question": exam.allow_answer_when_expired,
            },
        }
    else:
        body = {"success": False, "msg": f"未找到ID为 {UUID} 的考试！", "data": {}}
    return body


@teacher_api_v1.route("/api/v1/teacher/deleteExams", methods=["POST"])
def teacher_delete_exam() -> Response:
    exams_to_delete = request.json.get("examIds")
    try:
        for exam in exams_to_delete:
            delete_exam_data(exam)
        body = {"success": True, "msg": "删除考试成功！"}
        return jsonify(body)
    except Exception as e:
        body = {"success": False, "msg": f"删除考试失败！{e}"}
        return jsonify(body)


@teacher_api_v1.route("/api/v1/teacher/modifyExam", methods=["POST"])
def teacher_modify_exam() -> Response:
    data = request.form
    exam_id = data.get("examId")
    current_exam = query_exam_info(key="id", content=exam_id)
    if current_exam:
        all_exams = query_exams_info_all(999)
        for exam in all_exams:
            if (
                exam.start_time
                <= current_exam.start_time  # 有一门考试的开始时间在本次修改的考试之前且结束时间在本次修改的考试之后，即出现重合时间段使得两场考试同时进行
                and current_exam.end_time <= exam.end_time
                and exam.id.decode() != exam_id
            ):
                body = {
                    "success": False,
                    "msg": f"无法进行修改！修改后的考试时间段与其他考试（{exam.name.decode()}）时间段重合！",
                }
                return jsonify(body)
        try:
            if edit_exam_data(
                exam_id,
                data.get("examName"),
                int(
                    datetime.strptime(data.get("startDate"), "%Y-%m-%d %H:%M")
                    .replace(second=0)
                    .timestamp()
                ),
                int(
                    datetime.strptime(data.get("endDate"), "%Y-%m-%d %H:%M")
                    .replace(second=0)
                    .timestamp()
                ),
                int(data.get("allowAnswerWhenExpired")),
                int(data.get("randomQuestions")),
            ):
                body = {"success": True, "msg": "修改考试信息成功！"}

                if "xlsxFile" in request.files:
                    file = request.files["xlsxFile"]
                    try:
                        new_questions = questions_xlsx_parse(file.read())
                        old_questions = [
                            item
                            for item in query_questions_info_all(
                                999, key="exam_id", content=exam_id
                            )
                            if item.id.decode() != ""
                        ]
                        for question in old_questions:
                            try:
                                if not delete_question_data(question.id.decode()):
                                    raise Exception(question.id.decode())
                            except Exception as e:
                                body = {
                                    "success": False,
                                    "msg": (
                                        f"修改考试信息成功，但是有题目删除出错了！{e}"
                                        if "但是有题目删除出错了" not in body.get("msg")
                                        else body.get("msg")
                                        + f" {question.id.decode()}"
                                    ),
                                }
                                continue
                        for question in new_questions:
                            question_uuid = str(uuid.uuid4())
                            insert_question_data(
                                question_id=question_uuid,
                                exam_id=exam_id,
                                num1=question[0],
                                op=question[1],
                                num2=question[2],
                            )
                        body = {
                            "success": True,
                            "msg": body.get("msg") + "题目添加成功！",
                        }
                    except Exception as e:
                        body = {
                            "success": False,
                            "msg": f"修改考试信息成功，但是题目添加失败！{e}",
                        }
            else:
                body = {"success": False, "msg": "修改考试信息失败！"}
        except Exception as e:
            body = {"success": False, "msg": f"修改考试信息失败！{e}"}
    else:
        body = {"success": False, "msg": f"未找到ID为 {exam_id} 的考试！"}
    return jsonify(body)


@teacher_api_v1.route("/api/v1/teacher/getExamScores/<uuid:UUID>")
def teacher_get_exam_scores(UUID: str) -> Response:
    try:
        scores = [
            score
            for score in query_scores_info_all(999, key="exam_id", content=str(UUID))
            if score.id.decode() != ""
        ]
        exam = query_exam_info(key="id", content=str(UUID))
        data = []
        for score in scores:
            user = query_user_info(key="id", content=score.user_id.decode())
            data.append(
                {
                    "id": score.id.decode(),
                    "user_id": score.user_id.decode(),
                    "score": score.score,
                    "expired": score.expired_flag,
                    "number": user.number,
                    "name": user.name.decode(),
                }
            )
        body = {
            "success": True,
            "msg": "获取成绩成功",
            "metadata": {
                "id": exam.id.decode(),
                "name": exam.name.decode(),
                "start_time": exam.start_time,
                "end_time": exam.end_time,
                "allow_answer_when_expired": exam.allow_answer_when_expired,
                "random_question": exam.allow_answer_when_expired,
            },
            "data": data,
        }
    except Exception as e:
        body = {"success": False, "msg": f"获取成绩失败！{str(e)}", "data": []}
    return jsonify(body)


@teacher_api_v1.route("/api/v1/teacher/addStudents", methods=["POST"])
def teacher_add_students(): ...


@teacher_api_v1.route("/api/v1/teacher/deleteStudents", methods=["POST"])
def teacher_delete_students():
    pass


@teacher_api_v1.route("/api/v1/teacher/getStudent/<UUID>")
def teacher_get_student(UUID: str):
    pass


@teacher_api_v1.route("/api/v1/teacher/modifyStudent", methods=["POST"])
def teacher_modify_student():
    pass
