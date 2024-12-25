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
    delete_user_data,
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
    """
    用户登录函数：
    处理用户的登录请求，验证用户名和密码是否匹配。
    由于在C语言中处理哈希和登录计算较为复杂，因此将这些操作移至中间件完成。
    """
    # 获取请求中的JSON数据
    data = request.json
    # 初始化响应体，默认登录失败
    body = {"success": False, "msg": "init"}
    # 从请求数据中提取用户名和密码
    username: str = data.get("username", "")
    password: str = data.get("password", "")

    # 检查用户名和密码是否都提供
    if username and password:
        # 查询数据库中是否存在该用户名的用户
        user = query_users_info_all(1, key="username", content=username)
        if len(user) == 0:
            # 如果查询结果为空，说明用户名不存在
            body = {"success": False, "msg": "用户名或密码不匹配！"}
        else:
            # 获取查询到的第一个用户记录
            user = user[0]
            # 验证用户名和密码哈希是否匹配
            if (
                user.username == username.encode()
                and user.hashpass.decode()
                == sha512((user.salt.decode() + password).encode()).hexdigest()
            ):
                # 如果验证成功，设置成功消息
                body = {"success": True, "msg": "登录成功"}
            else:
                # 如果验证失败，设置错误消息
                body = {"success": False, "msg": "用户名或密码不匹配！"}
    else:
        # 如果用户名或密码为空，设置错误消息
        body = {"success": False, "msg": "用户名或密码不能为空！"}

    # 创建响应对象
    response = make_response(body)

    # 如果登录成功，生成JWT并设置到cookie中
    if body.get("success"):
        # 生成JWT，包含用户角色和ID
        cookie = jwt.encode(
            {"role": user.role, "id": user.id.decode(), "token": generate_salt()},
            JWT_KEY,
            algorithm="HS256",
        )
        cookie_age = 604800  # 设置cookie有效期为7天（7*24*60*60秒）
        response.set_cookie("token", cookie, max_age=cookie_age)

    # 返回响应
    return response


@general_api_v1.route("/api/v1/general/register", methods=["POST"])
def general_register() -> Response:
    """
    用户注册函数：
    处理用户的注册请求，验证输入信息的合法性，并将新用户信息存入数据库。
    """
    # 获取请求中的JSON数据
    data = request.json
    # 从请求数据中提取各项注册信息
    name: str = data.get("name", "")
    number: int = int(data.get("number", "0"))
    username: str = data.get("username", "")
    password: str = data.get("password", "")

    # 检查所有必填字段是否提供
    if name and number and username and password:
        # 查询数据库中是否已存在该用户名的用户
        user = query_user_info(key="username", content=username)
        if user.id:  # 如果查询到用户ID，说明用户名已被使用
            body = {
                "success": False,
                "msg": f"当前使用的用户名 {username} 已经被使用了！",
            }
        else:
            # 验证用户名长度是否符合要求（3到24个字符）
            if len(username) < 3:
                body = {"success": False, "msg": f"用户名 {username} 太短啦 😣"}
                return make_response(body)
            if len(username) > 24:
                body = {"success": False, "msg": f"用户名 {username} 太长啦 😣"}
                return make_response(body)

            # 检查用户名是否仅包含字母和数字
            for i in username:
                if i not in (string.ascii_letters + string.digits):
                    body = {
                        "success": False,
                        "msg": f"用户名 {username} 中包含非法字符 {i} 😦",
                    }
                    return make_response(body)

            # 验证工号是否在合法范围内
            if int(number) > 4294967295:
                body = {
                    "success": False,
                    "msg": f"工号 {number} 太长了，看起来不是合法的工号 😦",
                }
                return make_response(body)
            if int(number) <= 0:
                body = {
                    "success": False,
                    "msg": f"你输入了一个非法的工号 {number}，请不要尝试在这里玩栈溢出 😥",
                }
                return make_response(body)

            # 检查姓名的字节长度是否符合公安部规定（最长15个汉字，约45字节）
            if len(name.encode()) > 45:
                body = {
                    "success": False,
                    "msg": f"你的名字 {name} 太长啦，看起来不是合法的名字 😦",
                }
            else:
                # 生成新用户的UUID
                user_id = str(uuid.uuid4())
                # 生成随机盐值
                salt = generate_salt()
                # 计算密码的哈希值
                hashpass = sha512((salt + password).encode()).hexdigest()
                # 设置用户角色，默认角色为1
                role = 1
                # 初始化其他用户信息字段
                class_name = ""
                belong_to = ""

                # 将新用户数据插入数据库
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
                    # 如果插入成功，设置成功消息
                    body = {"success": True, "msg": "注册成功"}
                else:
                    # 如果插入失败，设置错误消息
                    body = {"success": False, "msg": "请查看日志获取详细信息 😦"}
    else:
        # 如果缺少必填字段，设置错误消息
        body = {"success": False, "msg": "发送的数据中未正确填写各项信息 😦"}

    # 创建并返回响应对象
    response = make_response(body)
    return response


@user_api_v1.route("/api/v1/user/modifyPassword", methods=["POST"])
def user_modify_password() -> Response:
    """
    用户修改密码函数：
    允许用户修改其密码，需验证原密码的正确性，并更新为新密码。
    """
    # 获取请求中的JSON数据
    data = request.json
    # 从请求数据中提取用户ID、原密码和新密码
    user_id = data.get("userId")
    original_password = data.get("originalPassword")
    new_password = data.get("newPassword")

    # 查询数据库中是否存在该用户ID的用户
    user = query_users_info_all(1, key="id", content=user_id)
    if user:
        # 获取查询到的第一个用户记录
        user = user[0]
        # 验证原密码是否正确
        if (
            user.hashpass.decode()
            == sha512((user.salt.decode() + original_password).encode()).hexdigest()
        ):
            # 生成新的盐值
            salt = generate_salt()
            # 计算新密码的哈希值
            hashpass = sha512((salt + new_password).encode()).hexdigest()
            # 更新用户数据，包括新的哈希密码和盐值
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
                # 如果更新成功，设置成功消息
                body = {"success": True, "msg": "已成功修改密码"}
            else:
                # 如果更新失败，设置错误消息
                body = {
                    "success": False,
                    "msg": "未知原因修改失败！请查看 log 文件内容获取更多信息！",
                }
        else:
            # 如果原密码不正确，设置错误消息
            body = {"success": False, "msg": "原密码不正确，请重新输入！"}
    else:
        # 如果未找到用户，设置错误消息
        body = {"success": False, "msg": "请求错误！请检查请求参数！"}

    # 返回JSON格式的响应
    return jsonify(body)


@student_api_v1.route("/api/v1/student/getExamInfo")
def student_get_exam_info(retJSON: int = 0) -> Response | dict:
    """
    获取考试信息函数：
    返回当前激活的考试或即将开始的最近考试的信息。
    如果参数 retJSON 为 1，则返回字典；否则返回 JSON 响应。
    """
    # 查询所有考试信息，限制返回数量为999
    exams = query_exams_info_all(999)
    if exams:
        # 按考试开始时间升序排序
        exams.sort(key=lambda x: x.start_time)
        now = time.time()

        active_exam = None
        next_exam = None

        # 选择考试逻辑：
        # 优先选择当前正在进行的考试（当前时间在开始和结束时间之间）
        # 如果没有进行中的考试，则选择下一个即将开始的考试
        # 如果没有下一个即将开始的考试，则返回最后一个过期的考试
        for exam in exams:
            if exam.start_time <= now < exam.end_time:
                active_exam = exam
                break  # 找到正在进行的考试后退出循环
            elif exam.start_time > now and next_exam is None:
                next_exam = exam
            elif exam.end_time <= now:
                last_past_exam = exam

        # 确定要返回的考试信息
        if active_exam:
            exam_to_return = active_exam
        elif next_exam:
            exam_to_return = next_exam
        elif last_past_exam:
            exam_to_return = last_past_exam
        else:
            exam_to_return = None

        if exam_to_return:
            # 构建成功的响应体，包含考试的元数据
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
            # 如果没有找到符合条件的考试，返回默认的错误信息
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
        # 如果查询不到任何考试信息，返回默认的错误信息
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
    # 根据 retJSON 参数决定返回字典还是 JSON 响应
    return body if retJSON else jsonify(body)


@student_api_v1.route("/api/v1/student/getExamData/<uuid:UUID>")
def student_get_exam_data(UUID: str) -> Response:
    """
    获取考试数据函数：
    根据考试的 UUID 获取考试的详细信息和相关问题列表。
    如果考试允许随机问题，则对问题进行随机排序。
    """
    # 查询指定 UUID 的考试信息
    exam = query_exam_info(key="id", content=str(UUID))
    if exam:
        # 构建成功的响应体，包含考试的元数据
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
        # 查询与考试相关的所有问题，排除 ID 为空的条目
        questions = [
            item
            for item in query_questions_info_all(999, key="exam_id", content=str(UUID))
            if item.id.decode() != ""
        ]
        # 遍历问题列表，生成原始问题列表
        original_question_list = traverse_question_list(questions)
        if exam.random_question:
            # 如果考试允许随机问题，进行 Fisher-Yates 随机洗牌
            randomize_question_list = original_question_list.copy()
            seed = int(time.time())
            random.seed(seed)
            body["metadata"]["seed"] = seed  # 将随机种子添加到元数据中
            for index in range(len(randomize_question_list))[::-1]:
                random_index = random.randint(0, len(questions) - 1)  # 生成随机索引
                # 交换当前索引和随机索引的元素
                (
                    randomize_question_list[index],
                    randomize_question_list[random_index],
                ) = (
                    randomize_question_list[random_index],
                    randomize_question_list[index],
                )
            body["data"] = randomize_question_list
        else:
            # 如果不允许随机问题，直接使用原始问题列表
            body["data"] = original_question_list
    else:
        # 如果未找到指定 UUID 的考试，返回错误信息
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
    # 返回 JSON 格式的响应
    return jsonify(body)


@student_api_v1.route("/api/v1/student/examSubmit", methods=["POST"])
def student_exam_submit() -> Response:
    """
    提交考试答案函数：
    接收学生提交的考试答案，计算得分，并将成绩存入数据库。
    """
    try:
        # 获取请求中的 JSON 数据
        data = request.json
        # 提取考试 ID、答案列表和随机种子
        exam_id: str = data.get("id")
        answers: list[str] = map(float, data.get("answers"))
        seed: int = int(data.get("seed", 0))
        # 从 JWT token 中解码获取用户 ID
        user_id = jwt.decode(
            request.cookies.get("token"), JWT_KEY, algorithms=["HS256"]
        ).get("id")
        # 查询考试信息
        exam = query_exam_info(key="id", content=exam_id)
        if seed:
            # 如果提供了随机种子，重新生成问题列表的顺序
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
            for index in range(len(randomize_question_list))[::-1]:
                random_index = random.randint(0, len(questions) - 1)  # 生成随机索引
                # 交换当前索引和随机索引的元素
                (
                    randomize_question_list[index],
                    randomize_question_list[random_index],
                ) = (
                    randomize_question_list[random_index],
                    randomize_question_list[index],
                )
            question_list = randomize_question_list
        else:
            # 如果没有提供随机种子，直接使用原始问题列表
            question_list = [
                {
                    "num1": item.num1,
                    "op": item.op,
                    "num2": item.num2,
                }
                for item in query_questions_info_all(
                    999, key="exam_id", content=exam_id
                )
                if item.id.decode() != ""
            ]
        # 计算得分
        score = calculate_score(question_list, answers)
        # 将成绩数据插入数据库
        insert_score_data(
            str(uuid.uuid4()),
            exam_id,
            user_id,
            score,
            1 if time.time() > exam.end_time else 0,  # 判断考试是否过期
        )
        # 构建成功的响应体，包含得分信息
        body = {"success": True, "msg": "提交成功！", "score": score}
    except Exception as e:
        # 如果发生异常，构建失败的响应体，包含错误信息
        body = {"success": False, "msg": f"提交失败！{e}"}
    # 返回 JSON 格式的响应
    return jsonify(body)


@student_api_v1.route("/api/v1/student/getScoreList")
def student_get_score_list(retJSON: int = 0):
    """
    获取成绩列表函数：
    返回当前用户的所有考试成绩列表。
    如果参数 retJSON 为 1，则返回字典；否则返回 JSON 响应。
    """
    # 从 JWT token 中解码获取用户 ID
    user_id = jwt.decode(
        request.cookies.get("token"), JWT_KEY, algorithms=["HS256"]
    ).get("id")
    # 查询当前用户的所有成绩记录，排除 ID 为空的条目
    scores = [
        item
        for item in query_scores_info_all(
            999,
            key="user_id",
            content=user_id,
        )
        if item.id.decode() != ""
    ]
    exam_list = []
    for score in scores:
        # 查询与成绩记录相关的考试信息
        exam_data = query_exam_info(key="id", content=score.exam_id.decode())
        if exam_data:
            # 将成绩和考试信息整合到列表中
            exam_list.append(
                {
                    "id": score.id.decode(),
                    "exam_id": score.exam_id.decode(),
                    "exam_name": exam_data.name.decode(),
                    "score": score.score,
                    "expired": score.expired_flag,
                }
            )
    if not exam_list:
        # 如果没有任何成绩记录，返回默认的提示信息
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
        # 如果存在成绩记录，返回整合后的成绩列表
        body = {"success": True, "msg": "成绩获取成功", "data": exam_list}
    # 根据 retJSON 参数决定返回字典还是 JSON 响应
    return body if retJSON else jsonify(body)


@teacher_api_v1.route("/api/v1/teacher/getAllExams")
def teacher_get_all_exams(retJSON: int = 0) -> Response | dict:
    """
    获取所有考试信息函数：
    返回所有考试的列表，包括考试的基本信息和当前状态。
    如果参数 retJSON 为1，则返回字典；否则返回 JSON 响应。
    """
    # 查询数据库中所有考试信息，限制返回数量为999条
    exams = query_exams_info_all(999)
    if exams:
        # 构建成功的响应体，包含考试列表
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
                            1 if exam.start_time <= time.time() < exam.end_time else -1
                        )
                    ),  # 计算考试的当前状态：0-未开始，1-进行中，-1-已结束
                }
                for exam in exams
            ],
        }
    else:
        # 如果未查询到任何考试信息，构建失败的响应体
        body = {
            "success": False,
            "msg": "获取考试列表失败",
            "data": [],
        }
    # 根据 retJSON 参数决定返回字典还是 JSON 响应
    return body if retJSON else jsonify(body)


@teacher_api_v1.route("/api/v1/teacher/addExam", methods=["POST"])
def teacher_add_exam() -> Response:
    """
    添加考试函数：
    处理教师提交的新考试信息和试题文件，添加新的考试及其相关试题到数据库。
    """
    # 从表单数据中获取考试信息和上传的文件
    exam_data = request.form["examData"]
    exam_data = unquote_to_bytes(exam_data)  # 对URL编码的考试数据进行解码
    current_exam = json.loads(exam_data)  # 将JSON字符串解析为字典
    if not all(
        [
            current_exam.get("examName"),
            current_exam.get("startDate"),
            current_exam.get("endDate"),
            current_exam.get("allowAnswerWhenExpired"),
            current_exam.get("randomQuestions"),
        ]
    ):
        body = {
            "success": False,
            "msg": "当前要添加的考试信息有内容未填写，请检查所有窗格的填写情况！",
        }
    if int(
        datetime.strptime(current_exam["startDate"], "%Y-%m-%d %H:%M")
        .replace(second=0)
        .timestamp()
    ) > int(
        datetime.strptime(current_exam["endDate"], "%Y-%m-%d %H:%M")
        .replace(second=0)
        .timestamp()
    ):
        body = {
            "success": False,
            "msg": f"考试的开始时间（{current_exam.get('startDate')}）不能大于结束时间（{current_exam.get('endDate')}）"
        }
        return jsonify(body)
    # 查询所有考试信息，限制返回数量为999条
    all_exams = query_exams_info_all(999)
    for exam in all_exams:
        # 检查修改后的考试时间是否与其他考试时间重叠
        # 当数据库中的考试的结束时间小于当前考试的开始时间
        # 或者数据库中的考试的开始时间大于当前考试的结束时间
        # 类似于 ====(exam1)===========(current_exam)=========(exam2)====> 的时间线
        #                 ↑ endTime   ↑ startTime  ↑ endTime ↑ startTime
        if not (
            exam.start_time
            > int(
                datetime.strptime(current_exam["endDate"], "%Y-%m-%d %H:%M")
                .replace(second=0)
                .timestamp()
            )
            or exam.end_time
            < int(
                datetime.strptime(current_exam["startDate"], "%Y-%m-%d %H:%M")
                .replace(second=0)
                .timestamp()
            )
        ):
            # 如果存在时间重叠，返回错误消息
            body = {
                "success": False,
                "msg": f"无法进行添加，要添加的考试占用的考试时间段与其他考试（{exam.name.decode()}）时间段重合（{datetime.fromtimestamp(exam.start_time)} ~ {datetime.fromtimestamp(exam.end_time)}）！",
            }
            return jsonify(body)
    file = request.files.get("xlsxFile")  # 获取上传的Excel文件
    if file is None:
        # 如果未上传文件，返回错误消息
        body = {"success": False, "msg": "未上传文件！"}
        return jsonify(body)
    # try:
    # 生成新的考试UUID
    exam_uuid = str(uuid.uuid4())
    # 插入新的考试数据到数据库
    if insert_exam_data(
        exam_id=exam_uuid,
        name=current_exam.get("examName"),
        start_time=int(
            datetime.strptime(current_exam.get("startDate"), "%Y-%m-%d %H:%M")
            .replace(second=0)
            .timestamp()
        ),  # 将开始日期转换为时间戳
        end_time=int(
            datetime.strptime(current_exam.get("endDate"), "%Y-%m-%d %H:%M")
            .replace(second=0)
            .timestamp()
        ),  # 将结束日期转换为时间戳
        allow_answer_when_expired=int(current_exam.get("allowAnswerWhenExpired")),
        random_question=int(current_exam.get("randomQuestions")),
    ):
        # 解析上传的Excel文件中的试题
        questions = questions_xlsx_parse(file.read())
        for question in questions:
            # 为每个试题生成唯一的UUID
            question_uuid = str(uuid.uuid4())
            # 插入试题数据到数据库
            insert_question_data(
                question_id=question_uuid,
                exam_id=exam_uuid,
                num1=question[0],
                op=question[1],
                num2=question[2],
            )
        # 如果所有操作成功，构建成功的响应体
        body = {"success": True, "msg": "添加考试成功！"}
    else:
        body = {"success": False, "msg": "添加考试失败！"}
    # except Exception as e:
    #     # 如果发生异常，构建失败的响应体并包含错误信息
    #     body = {"success": False, "msg": f"添加考试失败！{e}"}
    # 返回 JSON 格式的响应
    return jsonify(body)


@teacher_api_v1.route("/api/v1/teacher/getExam/<uuid:UUID>")
def teacher_get_exam(UUID: str) -> Response:
    """
    获取特定考试信息函数：
    根据考试的 UUID 获取该考试的详细信息。
    """
    # 查询指定 UUID 的考试信息
    exam = query_exam_info(key="id", content=str(UUID))
    if exam:
        # 如果考试存在，构建成功的响应体，包含考试的详细信息
        body = {
            "success": True,
            "msg": "获取考试信息成功",
            "data": {
                "id": exam.id.decode(),
                "name": exam.name.decode(),
                "start_time": exam.start_time,
                "end_time": exam.end_time,
                "allow_answer_when_expired": exam.allow_answer_when_expired,
                "random_question": exam.random_question,  # 修正错误：原代码重复使用 allow_answer_when_expired
            },
        }
    else:
        # 如果未找到指定 UUID 的考试，构建失败的响应体
        body = {"success": False, "msg": f"未找到ID为 {UUID} 的考试！", "data": {}}
    # 返回 JSON 格式的响应
    return jsonify(body)


@teacher_api_v1.route("/api/v1/teacher/deleteExams", methods=["POST"])
def teacher_delete_exam() -> Response:
    """
    删除考试函数：
    处理教师提交的考试ID列表，删除对应的考试及其相关数据。
    """
    # 从请求的JSON数据中获取要删除的考试ID列表
    exams_to_delete = request.json.get("examIds")
    try:
        for exam_id in exams_to_delete:
            # 遍历每个考试ID并执行删除操作
            delete_exam_data(exam_id)
        # 如果所有删除操作成功，构建成功的响应体
        body = {"success": True, "msg": "删除考试成功！"}
        return jsonify(body)
    except Exception as e:
        # 如果发生异常，构建失败的响应体并包含错误信息
        body = {"success": False, "msg": f"删除考试失败！{e}"}
        return jsonify(body)


@teacher_api_v1.route("/api/v1/teacher/modifyExam", methods=["POST"])
def teacher_modify_exam() -> Response:
    """
    修改考试信息函数：
    处理教师提交的考试修改请求，更新考试的基本信息和相关试题。
    确保修改后的考试时间不与其他考试时间重叠。
    如果上传了新的试题文件，删除旧试题并添加新试题。
    """
    # 获取请求中的表单数据
    data = request.form
    exam_id = data.get("examId")  # 获取要修改的考试ID
    if not all([data.get("examId"), data.get("examName"), data.get("startDate"), data.get("endDate"), data.get("allowAnswerWhenExpired"), data.get("randomQuesitons")]):
        body = {
            "success": False,
            "msg": "传入的修改信息不能为空！"
        }
    current_exam_start_time_from_front = int(
        datetime.strptime(data["startDate"], "%Y-%m-%d %H:%M")
        .replace(second=0)
        .timestamp()
    )
    current_exam_end_time_from_front = int(
        datetime.strptime(data["endDate"], "%Y-%m-%d %H:%M")
        .replace(second=0)
        .timestamp()
    )
    if current_exam_start_time_from_front >= current_exam_end_time_from_front:
        body = {
            "success": False,
            "msg": "考试的开始时间不能大于结束时间！"
        }
        return jsonify(body)
    # 查询当前考试的信息
    current_exam = query_exam_info(key="id", content=exam_id)

    if current_exam:
        # 查询所有考试信息，限制返回数量为999条
        all_exams = query_exams_info_all(999)
        for exam in all_exams:
                # 检查修改后的考试时间是否与其他考试时间重叠
                # 当数据库中的考试的结束时间小于当前考试的开始时间
                # 或者数据库中的考试的开始时间大于当前考试的结束时间
                # 类似于 ====(exam1)===========(current_exam)=========(exam2)====> 的时间线
                #                 ↑ endTime   ↑ startTime  ↑ endTime ↑ startTime
            if not (
                exam.end_time < current_exam_start_time_from_front or exam.start_time > current_exam_end_time_from_front
            ):
                # 如果存在时间重叠，返回错误消息
                body = {
                    "success": False,
                    "msg": f"无法进行修改！修改后的考试时间段与其他考试（{exam.name.decode()}）时间段（{datetime.fromtimestamp(exam.start_time)} ~ {datetime.fromtimestamp(exam.end_time)}）重合！",
                }
                return jsonify(body)
        try:
            # 尝试更新考试的基本信息
            if edit_exam_data(
                exam_id,
                data.get("examName"),
                int(
                    datetime.strptime(data.get("startDate"), "%Y-%m-%d %H:%M")
                    .replace(second=0)
                    .timestamp()
                ),  # 将开始日期转换为时间戳
                int(
                    datetime.strptime(data.get("endDate"), "%Y-%m-%d %H:%M")
                    .replace(second=0)
                    .timestamp()
                ),  # 将结束日期转换为时间戳
                int(data.get("allowAnswerWhenExpired")),
                int(data.get("randomQuestions")),
            ):
                # 如果基本信息修改成功，设置成功消息
                body = {"success": True, "msg": "修改考试信息成功！"}

                # 检查是否上传了新的试题文件
                if "xlsxFile" in request.files:
                    file = request.files["xlsxFile"]
                    try:
                        # 解析上传的Excel文件中的新试题
                        new_questions = questions_xlsx_parse(file.read())
                        # 查询当前考试的所有旧试题，排除ID为空的条目
                        old_questions = [
                            item
                            for item in query_questions_info_all(
                                999, key="exam_id", content=exam_id
                            )
                            if item.id.decode() != ""
                        ]
                        # 删除所有旧试题
                        for question in old_questions:
                            try:
                                if not delete_question_data(question.id.decode()):
                                    raise Exception(question.id.decode())
                            except Exception as e:
                                # 如果删除试题失败，记录错误消息
                                body = {
                                    "success": False,
                                    "msg": (
                                        f"修改考试信息成功，但是有题目删除出错了！{e}"
                                        if "但是有题目删除出错了" not in body.get("msg")
                                        else body.get("msg")
                                        + f" {question.id.decode()}"
                                    ),
                                }
                                continue  # 继续删除其他试题
                        # 插入新的试题数据
                        for question in new_questions:
                            question_uuid = str(uuid.uuid4())
                            insert_question_data(
                                question_id=question_uuid,
                                exam_id=exam_id,
                                num1=question[0],
                                op=question[1],
                                num2=question[2],
                            )
                        # 更新成功消息，包含试题添加成功的信息
                        body = {
                            "success": True,
                            "msg": body.get("msg") + "题目添加成功！",
                        }
                    except Exception as e:
                        # 如果添加试题失败，更新错误消息
                        body = {
                            "success": False,
                            "msg": f"修改考试信息成功，但是题目添加失败！{e}",
                        }
            else:
                # 如果基本信息修改失败，返回错误消息
                body = {"success": False, "msg": "修改考试信息失败！"}
        except Exception as e:
            # 如果修改考试信息过程中发生异常，返回错误消息
            body = {"success": False, "msg": f"修改考试信息失败！{e}"}
    else:
        # 如果未找到指定ID的考试，返回错误消息
        body = {"success": False, "msg": f"未找到ID为 {exam_id} 的考试！"}

    # 返回JSON格式的响应
    return jsonify(body)


@teacher_api_v1.route("/api/v1/teacher/getExamScores/<uuid:UUID>")
def teacher_get_exam_scores(UUID: str) -> Response:
    """
    获取考试成绩函数：
    根据考试的UUID获取该考试的所有成绩记录，包括学生信息。
    返回成绩列表和考试的基本信息。
    """
    try:
        # 查询指定考试ID的所有成绩记录，排除ID为空的条目
        scores = [
            score
            for score in query_scores_info_all(999, key="exam_id", content=str(UUID))
            if score.id.decode() != ""
        ]
        # 查询考试的基本信息
        exam = query_exam_info(key="id", content=str(UUID))
        data = []
        for score in scores:
            # 查询每个成绩记录对应的学生信息
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
        # 构建成功的响应体，包含考试的元数据和成绩数据
        body = {
            "success": True,
            "msg": "获取成绩成功",
            "metadata": {
                "id": exam.id.decode(),
                "name": exam.name.decode(),
                "start_time": exam.start_time,
                "end_time": exam.end_time,
                "allow_answer_when_expired": exam.allow_answer_when_expired,
                "random_question": exam.random_question,  # 修正错误：原代码重复使用 allow_answer_when_expired
            },
            "data": data,
        }
    except Exception as e:
        # 如果发生异常，构建失败的响应体并包含错误信息
        body = {"success": False, "msg": f"获取成绩失败！{str(e)}", "data": []}
    # 返回JSON格式的响应
    return jsonify(body)


@teacher_api_v1.route("/api/v1/teacher/addStudents", methods=["POST"])
def teacher_add_students() -> Response:
    """
    添加学生函数：
    处理教师提交的学生信息，支持单个添加和批量导入。
    验证学生信息的完整性和唯一性（学号）。
    将有效的学生信息插入数据库，并返回操作结果。
    """
    # 获取请求中的表单数据
    student = request.form
    # 获取上传的学生文件（如果有）
    student_file = request.files["xlsxFile"] if "xlsxFile" in request.files else None
    # 获取教师的JWT token
    teacher_cookie = request.cookies
    token = teacher_cookie.get("token")
    # 解码JWT token以获取教师ID
    token_data = jwt.decode(token, JWT_KEY, algorithms=["HS256"])
    teacher_id = token_data.get("id")

    if not teacher_id:
        # 如果未找到教师ID，返回错误消息
        body = {"success": False, "msg": "未找到教师信息！请重新登录！"}
        return jsonify(body)

    if student_file is None:
        # 如果未上传文件，认为是单个学生添加
        try:
            # 检查是否提供了所有必填字段
            if not all(
                [
                    student.get("studentName"),
                    student.get("className"),
                    student.get("number"),
                ]
            ):
                body = {"success": False, "msg": "请填写完整的学生信息！"}
                return jsonify(body)
            # 验证学号是否在合法范围内
            if (
                int(student.get("number")) < 0
                or int(student.get("number")) > 4294967295
            ):
                body = {
                    "success": False,
                    "msg": "学号不合法！请填写正确的学号！",
                }
                return jsonify(body)
            # 检查学号是否已存在，避免重复
            if query_user_info(key="number", content=student.get("number")):
                body = {
                    "success": False,
                    "msg": "学号与已有数据重复！请检查学号是否填写正确！",
                }
                return jsonify(body)
            # 生成新的用户ID和盐值
            user_id = str(uuid.uuid4())
            salt = generate_salt()
            # 生成随机密码
            password = generate_salt(32)  # 使用盐生成函数生成随机密码
            # 计算密码的哈希值
            hashpass = sha512((salt + password).encode()).hexdigest()
            # 插入新的学生数据到数据库
            if insert_user_data(
                user_id,
                str(student.get("number")),  # 以学号作为学生的登录依据
                hashpass,
                salt,
                0,  # 角色设为0，表示学生
                student.get("studentName"),
                student.get("className"),
                int(student.get("number")),
                teacher_id,
            ):
                # 如果插入成功，返回成功消息
                body = {"success": True, "msg": "添加学生成功！"}
            else:
                # 如果插入失败，返回错误消息
                body = {
                    "success": False,
                    "msg": "添加学生失败！请查看日志文件获取更多信息！",
                }
            return jsonify(body)
        except Exception as e:
            # 如果发生异常，返回错误消息
            body = {"success": False, "msg": f"添加学生失败！{e}"}
            return jsonify(body)
    else:
        # 如果上传了文件，进行批量添加
        msg = """导入成功 {success_count} 个学生\n导入失败 {failed_count} 个学生\n{failed_students}"""
        success_count = 0  # 成功添加的学生数量
        failed_count = 0  # 失败添加的学生数量
        failed_students_list = []  # 记录添加失败的学生信息
        try:
            # 解析上传的Excel文件中的学生信息
            students = students_xlsx_parser(student_file.read())
            for (
                student
            ) in students:  # 每个学生的结构为：[number, name, class_name, password]
                # 检查学号是否已存在，避免重复
                tmp_user = query_user_info(key="number", content=str(student[0]))
                user = tmp_user if tmp_user.id.decode() != "" else None
                if user:
                    # 如果学号重复，记录失败原因
                    del tmp_user, user
                    failed_students_list.append((student[1], "与已有数据学号重复"))
                    failed_count += 1
                    continue
                del tmp_user, user
                # 生成新的用户ID和盐值
                user_id = str(uuid.uuid4())
                salt = generate_salt()
                # 计算密码的哈希值
                hashpass = sha512((salt + student[3]).encode()).hexdigest()
                # 尝试插入新的学生数据到数据库
                if insert_user_data(
                    user_id,
                    str(student[0]),
                    hashpass,
                    salt,
                    0,  # 角色设为0，表示学生
                    student[1],
                    student[2],
                    student[0],
                    teacher_id,
                ):
                    # 如果插入成功，增加成功计数
                    success_count += 1
                else:
                    # 如果插入失败，记录失败原因
                    failed_students_list.append((student[1], "未知原因"))
                    failed_count += 1
        except Exception as e:
            # 如果解析或插入过程中发生异常，返回错误消息
            body = {"success": False, "msg": f"批量添加学生失败！{e}"}
            return jsonify(body)

        # 将失败的学生信息转换为字符串格式
        if failed_students_list:
            failed_students_str = "\n".join(
                f"{name}：{reason}" for name, reason in failed_students_list
            )
        else:
            failed_students_str = ""

        # 构建最终的响应消息
        body = {
            "success": True,
            "msg": msg.format(
                success_count=success_count,
                failed_count=failed_count,
                failed_students=failed_students_str,
            ),
        }
        return jsonify(body)


@teacher_api_v1.route("/api/v1/teacher/deleteStudents", methods=["POST"])
def teacher_delete_students() -> Response:
    """
    删除学生函数：
    处理教师提交的学生ID列表，删除对应的学生用户数据。
    """
    # 获取请求中的JSON数据
    data = request.json
    students_to_delete = data.get("studentIds")  # 获取要删除的学生ID列表

    try:
        # 遍历每个学生ID并执行删除操作
        for student_id in students_to_delete:
            if not delete_user_data(student_id):
                # 如果删除操作失败，抛出异常并包含失败的学生ID
                raise Exception(f"{student_id} 删除失败！")

        # 如果所有删除操作成功，构建成功的响应体
        body = {"success": True, "msg": "删除学生成功！"}
        return jsonify(body)
    except Exception as e:
        # 如果发生异常，构建失败的响应体并包含错误信息
        body = {"success": False, "msg": f"删除学生失败！{e}"}
        return jsonify(body)


@teacher_api_v1.route("/api/v1/teacher/getAllStudents")
def teacher_get_all_students(retJSON: int = 0) -> Response | dict:
    """
    获取所有学生信息函数：
    返回当前教师所属的所有学生的列表，包括学生的基本信息。
    如果参数 retJSON 为1，则返回字典；否则返回 JSON 响应。
    """
    try:
        # 从JWT token中解码获取教师ID
        token = request.cookies.get("token")
        token_data = jwt.decode(token, JWT_KEY, algorithms=["HS256"])
        teacher_id = token_data.get("id")

        # 查询数据库中所有学生信息，限制返回数量为999条
        students = [
            item
            for item in query_users_info_all(999)
            if item.id.decode() != ""
            and item.role == 0  # 角色为0表示学生
            and item.belong_to.decode() == teacher_id
        ]
    except Exception as e:
        # 如果解码JWT或查询数据库时发生异常，返回失败响应
        body = {"success": False, "msg": f"获取学生列表失败！{e}", "data": []}
        return body if retJSON else jsonify(body)

    if students:
        # 构建成功的响应体，包含学生列表
        body = {
            "success": True,
            "msg": "获取学生列表成功",
            "data": [
                {
                    "id": student.id.decode(),
                    "number": student.number,
                    "name": student.name.decode(),
                    "class_name": student.class_name.decode(),
                    "username": student.username.decode(),
                }
                for student in students
            ],
        }
    else:
        # 如果未查询到任何学生信息，构建失败的响应体
        body = {
            "success": False,
            "msg": "获取学生列表失败",
            "data": [],
        }
    # 根据 retJSON 参数决定返回字典还是 JSON 响应
    return body if retJSON else jsonify(body)


@teacher_api_v1.route("/api/v1/teacher/getStudent/<uuid:UUID>")
def teacher_get_student(UUID: str) -> Response:
    """
    获取单个学生信息函数：
    根据学生的UUID获取该学生的详细信息。
    """
    student_id = str(UUID)  # 将UUID转换为字符串格式
    # 查询指定UUID的学生信息
    student = query_user_info(key="id", content=student_id)

    if student:
        # 如果学生存在，构建成功的响应体，包含学生的详细信息
        body = {
            "success": True,
            "msg": "获取学生信息成功",
            "data": {
                "id": student.id.decode(),
                "number": student.number,
                "name": student.name.decode(),
                "class_name": student.class_name.decode(),
                "username": student.username.decode(),
            },
        }
    else:
        # 如果未找到指定UUID的学生，构建失败的响应体
        body = {
            "success": False,
            "msg": f"未找到ID为 {student_id} 的学生！",
            "data": {},
        }
    # 返回JSON格式的响应
    return jsonify(body)


@teacher_api_v1.route("/api/v1/teacher/modifyStudent", methods=["POST"])
def teacher_modify_student() -> Response:
    """
    修改学生信息函数：
    处理教师提交的学生信息修改请求，更新学生的基本信息和密码（如果需要）。
    """
    # 获取请求中的表单数据
    data = request.form
    student_id = data.get("studentId")  # 获取要修改的学生ID

    # 查询当前学生的信息
    student_records = query_users_info_all(1, key="id", content=student_id)
    if not student_records:
        # 如果未找到指定ID的学生，返回错误消息
        body = {"success": False, "msg": f"未找到ID为 {student_id} 的学生！"}
        return jsonify(body)

    student = student_records[0]  # 获取查询到的学生记录

    try:
        # 检查是否提供了所有必填字段
        if not all([data.get("name"), data.get("className"), data.get("number")]):
            body = {"success": False, "msg": "请填写完整的学生信息！"}
            return jsonify(body)

        # 验证学号是否在合法范围内
        student_number = int(data.get("number"))
        if student_number < 0 or student_number > 4294967295:
            body = {
                "success": False,
                "msg": "学号不合法！请填写正确的学号！",
            }
            return jsonify(body)

        # 如果需要重置密码，生成新的盐值；否则保持原盐值
        salt = (
            generate_salt()
            if data.get("resetPassword") == "1"
            else student.salt.decode()
        )

        # 检查新的学号是否已存在且不属于当前学生
        if data.get("number"):
            existing_user = (
                query_user_info(key="number", content=data.get("number"))
                if query_exam_info(key="number", content=data.get("number"))
                and query_exam_info(
                    key="number", content=data.get("number")
                ).id.decode()
                != ""
                else None
            )
            if existing_user:
                if existing_user.id.decode() != student_id:
                    body = {
                        "success": False,
                        "msg": "学号与已有数据重复！请检查学号是否填写正确！",
                    }
                    return jsonify(body)

        # 计算新的哈希密码，如果需要重置密码
        new_hashpass = (
            sha512((salt + data.get("newPassword")).encode()).hexdigest()
            if data.get("resetPassword") == "1"
            else student.hashpass.decode()
        )

        # 尝试更新学生的数据
        if edit_user_data(
            student_id,
            data.get("number"),
            new_hashpass,
            salt,
            student.role,
            data.get("name"),
            data.get("className"),
            student_number,
            student.belong_to.decode(),
        ):
            # 如果更新成功，返回成功消息
            body = {"success": True, "msg": "修改学生信息成功！"}
        else:
            # 如果更新失败，返回错误消息
            body = {"success": False, "msg": "修改学生信息失败！"}
    except Exception as e:
        # 如果发生异常，返回失败消息并包含错误信息
        body = {"success": False, "msg": f"修改学生信息失败！{e}"}

    # 返回JSON格式的响应
    return jsonify(body)
