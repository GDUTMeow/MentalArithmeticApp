from __init__ import *
from ctypes import c_char_p, c_int, POINTER


def query_user_info(key: str, content: str) -> User:
    """
    查询用户信息的函数。

    @param key: 查询的键（如用户名或ID），字符串类型。
    @param content: 查询的内容（如用户名的具体值），字符串类型。

    @return: 如果查询成功，返回一个 User 对象，包含查询到的用户信息。
             如果查询失败，返回 None。
    """
    user = User()  # 创建一个 User 对象用于存储查询结果

    # 将字符串参数转换为 C 语言的 c_char_p 类型（即 C 中的 char*）
    key_c = c_char_p(key.encode("utf-8"))
    content_c = c_char_p(content.encode("utf-8"))

    # 调用 C 语言的查询函数，并将结果存入 user
    result = DATABASE_LIB.query_user_info(key_c, content_c, ctypes.byref(user))

    # 根据查询结果判断是否成功，返回相应的结果
    if result == 0:
        return user  # 查询成功，返回 User 对象
    else:
        return None  # 查询失败，返回 None


def query_exam_info(key: str, content: str) -> SqlResponseExam:
    """
    查询考试信息的函数。

    @param key: 查询的键（如考试ID或名称），字符串类型。
    @param content: 查询的内容（如考试的具体值），字符串类型。

    @return: 如果查询成功，返回一个 SqlResponseExam 对象，包含查询到的考试信息。
             如果查询失败，返回 None。
    """
    exam = SqlResponseExam()  # 创建一个 SqlResponseExam 对象用于存储查询结果

    # 将字符串参数转换为 C 语言的 c_char_p 类型（即 C 中的 char*）
    key_c = c_char_p(key.encode("utf-8"))
    content_c = c_char_p(content.encode("utf-8"))

    # 调用 C 语言的查询函数，并将结果存入 exam
    result = DATABASE_LIB.query_exam_info(key_c, content_c, ctypes.byref(exam))

    # 根据查询结果判断是否成功，返回相应的结果
    if result == 0:
        return exam  # 查询成功，返回 SqlResponseExam 对象
    else:
        return None  # 查询失败，返回 None


def query_question_info(key: str, content: str) -> SqlResponseQuestion:
    """
    查询题目信息的函数。

    @param key: 查询的键（如题目ID或考试ID），字符串类型。
    @param content: 查询的内容（如题目的具体值），字符串类型。

    @return: 如果查询成功，返回一个 SqlResponseQuestion 对象，包含查询到的题目信息。
             如果查询失败，返回 None。
    """
    question = (
        SqlResponseQuestion()
    )  # 创建一个 SqlResponseQuestion 对象用于存储查询结果

    # 将字符串参数转换为 C 语言的 c_char_p 类型（即 C 中的 char*）
    key_c = c_char_p(key.encode("utf-8"))
    content_c = c_char_p(content.encode("utf-8"))

    # 调用 C 语言的查询函数，并将结果存入 question
    result = DATABASE_LIB.query_question_info(key_c, content_c, ctypes.byref(question))

    # 根据查询结果判断是否成功，返回相应的结果
    if result == 0:
        return question  # 查询成功，返回 SqlResponseQuestion 对象
    else:
        return None  # 查询失败，返回 None


def query_score_info(key: str, content: str) -> SqlResponseScore:
    """
    查询成绩信息的函数。

    @param key: 查询的字段（"exam_id" 或 "user_id"），字符串类型。
    @param content: 查询的具体内容，字符串类型。

    @return: 如果查询成功，返回一个 SqlResponseScore 对象，包含查询到的成绩信息。
             如果查询失败，返回 None。
    """
    score = SqlResponseScore()  # 创建一个 SqlResponseScore 对象用于存储查询结果

    # 将字符串参数转换为 C 语言的 c_char_p 类型（即 C 中的 char*）
    key_c = c_char_p(key.encode("utf-8"))
    content_c = c_char_p(content.encode("utf-8"))

    # 调用 C 语言的查询函数，并将结果存入 score
    result = DATABASE_LIB.query_score_info(key_c, content_c, ctypes.byref(score))

    # 根据查询结果判断是否成功，返回相应的结果
    if result == 0:
        return score  # 查询成功，返回 SqlResponseScore 对象
    else:
        return None  # 查询失败，返回 None


def query_exams_info_all(length: int) -> list[SqlResponseExam]:
    """
    @brief 查询所有考试信息，并返回查询结果。

    @param length 要查询的考试数量。

    @return list 查询到的所有考试信息列表，若查询失败返回空列表。
    """
    # 创建返回结构体数组，长度为指定的查询数量
    exams_to_return = (SqlResponseExam * length)()

    # 调用C函数，查询所有考试信息
    result = DATABASE_LIB.query_exams_info_all(ctypes.byref(exams_to_return[0]), length)

    if result == 0:
        # 查询成功，返回所有考试信息
        return [exam for exam in exams_to_return]
    else:
        # 查询失败，返回空列表
        return []


def query_users_info_all(length: int) -> list[SqlResponseUser]:
    """
    @brief 查询所有用户信息，并返回查询结果。

    @param length 要查询的用户数量。

    @return list 查询到的所有用户信息列表，若查询失败返回空列表。
    """
    # 创建返回结构体数组，长度为指定的查询数量
    users_to_return = (SqlResponseUser * length)()

    # 调用C函数，查询所有用户信息
    result = DATABASE_LIB.query_users_info_all(ctypes.byref(users_to_return[0]), length)

    if result == 0:
        # 查询成功，返回所有用户信息
        return [user for user in users_to_return]
    else:
        # 查询失败，返回空列表
        return []


def query_questions_info_all(length: int) -> list[SqlResponseQuestion]:
    """
    @brief 查询所有问题信息，并返回查询结果。

    @param length 要查询的问题数量。

    @return list 查询到的所有问题信息列表，若查询失败返回空列表。
    """
    # 创建返回结构体数组，长度为指定的查询数量
    questions_to_return = (SqlResponseQuestion * length)()

    # 调用C函数，查询所有问题信息
    result = DATABASE_LIB.query_questions_info_all(
        ctypes.byref(questions_to_return[0]), length
    )

    if result == 0:
        # 查询成功，返回所有问题信息
        return [question for question in questions_to_return]
    else:
        # 查询失败，返回空列表
        return []


def query_scores_info_all(length: int) -> list[SqlResponseScore]:
    """
    @brief 查询所有成绩信息，并返回查询结果。

    @param length 要查询的成绩数量。

    @return list 查询到的所有成绩信息列表，若查询失败返回空列表。
    """
    # 创建返回结构体数组，长度为指定的查询数量
    scores_to_return = (SqlResponseScore * length)()

    # 调用C函数，查询所有成绩信息
    result = DATABASE_LIB.query_scores_info_all(
        ctypes.byref(scores_to_return[0]), length
    )

    if result == 0:
        # 查询成功，返回所有成绩信息
        return [score for score in scores_to_return]
    else:
        # 查询失败，返回空列表
        return []


def insert_exam_data(
    exam_id: str,
    name: str,
    start_time: int,
    end_time: int,
    allow_answer_when_expired: int,
    random_question: int,
) -> int:
    """
    @brief 插入考试数据到数据库。

    @param exam_id 考试ID，唯一标识每个考试。
    @param name 考试名称。
    @param start_time 考试开始时间，时间戳格式。
    @param end_time 考试结束时间，时间戳格式。
    @param allow_answer_when_expired 是否允许过期后继续答题，布尔值。
    @param random_question 是否启用随机题目顺序，布尔值。

    @return int 操作结果，1 表示成功，0 表示失败。
    """
    result = DATABASE_LIB.insert_exam_data(
        exam_id.encode("utf-8"),
        name.encode("utf-8"),
        start_time,
        end_time,
        allow_answer_when_expired,
        random_question,
    )
    return 1 if not result else 0


def insert_question_data(
    question_id: str, exam_id: str, num1: int, op: int, num2: int
) -> int:
    """
    @brief 插入题目数据到数据库。

    @param question_id 题目ID，唯一标识每道题目。
    @param exam_id 考试ID，表示该题目所属的考试。
    @param num1 第一个操作数，用于生成数学题目。
    @param op 运算符，表示该题目是加、减、乘、除之一。
    @param num2 第二个操作数，用于生成数学题目。

    @return int 操作结果，1 表示成功，0 表示失败。
    """
    result = DATABASE_LIB.insert_question_data(
        question_id.encode("utf-8"), exam_id.encode("utf-8"), num1, op, num2
    )
    return 1 if not result else 0


def insert_score_data(
    score_id: str, exam_id: str, user_id: str, score: float, expired_flag: int
) -> int:
    """
    @brief 插入成绩数据到数据库。

    @param score_id 成绩ID，唯一标识每条成绩记录。
    @param exam_id 考试ID，表示该成绩所属的考试。
    @param user_id 用户ID，表示该成绩属于哪个用户。
    @param score 用户在该考试中的得分。
    @param expired_flag 标识该成绩是否已过期，0表示未过期，1表示已过期。

    @return int 操作结果，1 表示成功，0 表示失败。
    """
    result = DATABASE_LIB.insert_score_data(
        score_id.encode("utf-8"),
        exam_id.encode("utf-8"),
        user_id.encode("utf-8"),
        score,
        expired_flag,
    )
    return 1 if not result else 0


def insert_user_data(
    user_id: str,
    username: str,
    hashpass: str,
    salt: str,
    role: int,
    name: str,
    class_name: str,
    number: int,
    belong_to: str,
) -> int:
    """
    @brief 插入用户数据到数据库

    @param user_id 用户ID，唯一标识每个用户。
    @param username 用户名，用于登录的标识。
    @param hashpass 用户密码的哈希值。
    @param salt 用于加密密码的盐值。
    @param role 用户角色，通常用于区分管理员、学生等。
    @param name 用户的真实姓名。
    @param class_name 用户所在班级名称。
    @param number 用户的学号。
    @param belong_to 用户所属的学校或组织。

    @return int 操作结果，1 表示成功，0 表示失败。
    """
    result = DATABASE_LIB.insert_user_data(
        user_id.encode("utf-8"),
        username.encode("utf-8"),
        hashpass.encode("utf-8"),
        salt.encode("utf-8"),
        role,
        name.encode("utf-8"),
        class_name.encode("utf-8"),
        number,
        belong_to.encode("utf-8"),
    )
    return 1 if not result else 0


def delete_user_data(user_id: str) -> int:
    """
    @brief 删除指定用户数据

    @param user_id 用户ID，唯一标识每个用户。

    @return int 结果，成功返回 1，失败返回 0。
    """
    user_id_bytes = user_id.encode("utf-8")
    result = DATABASE_LIB.del_user_data(user_id_bytes)
    return 1 if not result else 0


def delete_exam_data(exam_id: str) -> int:
    """
    @brief 删除指定考试数据

    @param exam_id 考试ID，唯一标识每个考试。

    @return int 结果，成功返回 1，失败返回 0。
    """
    exam_id_bytes = exam_id.encode("utf-8")
    result = DATABASE_LIB.del_exam_data(exam_id_bytes)
    return 1 if not result else 0


def delete_score_data(score_id: str) -> int:
    """
    @brief 删除指定成绩数据

    @param score_id 成绩ID，唯一标识每条成绩记录。

    @return int 结果，成功返回 1，失败返回 0。
    """
    score_id_bytes = score_id.encode("utf-8")
    result = DATABASE_LIB.del_score_data(score_id_bytes)
    return 1 if not result else 0


def delete_question_data(question_id: str) -> int:
    """
    @brief 删除指定问题数据

    @param question_id 题目ID，唯一标识每道题目。

    @return int 结果，成功返回 1，失败返回 0。
    """
    question_id_bytes = question_id.encode("utf-8")
    result = DATABASE_LIB.del_question_data(question_id_bytes)
    return 1 if not result else 0


def edit_user_data(
    user_id: str,
    username: str,
    hashpass: str,
    salt: str,
    role: str,
    name: str,
    class_name: str,
    number: str,
    belong_to: str,
) -> int:
    """
    @brief 编辑用户数据

    @param user_id 用户ID，唯一标识每个用户。
    @param username 用户名，用于登录的标识。
    @param hashpass 用户密码的哈希值。
    @param salt 用于加密密码的盐值。
    @param role 用户角色，通常用于区分管理员、学生等。
    @param name 用户的真实姓名。
    @param class_name 用户所在班级名称。
    @param number 用户的学号。
    @param belong_to 用户所属的学校或组织。

    @return int 结果，成功返回 1，失败返回 0
    """
    result = DATABASE_LIB.edit_user_data(
        user_id.encode("utf-8"),
        username.encode("utf-8"),
        hashpass.encode("utf-8"),
        salt.encode("utf-8"),
        role,
        name.encode("utf-8"),
        class_name.encode("utf-8"),
        number,
        belong_to.encode("utf-8") if belong_to else None,
    )
    return 1 if not result else 0


def edit_exam_data(
    exam_id: str,
    name: str,
    start_time: int,
    end_time: int,
    allow_answer_when_expired: int,
    random_question: int,
) -> int:
    """
    @brief 编辑考试数据

    @param exam_id 考试ID，唯一标识每个考试。
    @param name 考试名称。
    @param start_time 考试开始时间，时间戳格式。
    @param end_time 考试结束时间，时间戳格式。
    @param allow_answer_when_expired 是否允许过期后继续答题，布尔值。
    @param random_question 是否启用随机题目顺序，布尔值。

    @return int 结果，成功返回 1，失败返回 0。
    """
    result = DATABASE_LIB.edit_exam_data(
        exam_id.encode("utf-8"),
        name.encode("utf-8"),
        start_time,
        end_time,
        allow_answer_when_expired,
        random_question,
    )
    return 1 if not result else 0


def edit_score_data(
    score_id: str, exam_id: str, user_id: str, score: float, expired_flag: int
) -> int:
    """
    @brief 编辑成绩数据

    @param score_id 成绩ID，唯一标识每条成绩记录。
    @param exam_id 考试ID，表示该成绩所属的考试。
    @param user_id 用户ID，表示该成绩属于哪个用户。
    @param score 用户在该考试中的得分。
    @param expired_flag 标识该成绩是否已过期，0表示未过期，1表示已过期。

    @return int 结果，成功返回 1，失败返回 0。
    """
    result = DATABASE_LIB.edit_score_data(
        score_id.encode("utf-8"),
        exam_id.encode("utf-8"),
        user_id.encode("utf-8"),
        score,
        expired_flag,
    )
    return 1 if not result else 0


def edit_question_data(
    question_id: str, exam_id: str, num1: float, op: int, num2: float
) -> int:
    """
    @brief 编辑问题数据

    @param question_id 题目ID，唯一标识每道题目。
    @param exam_id 考试ID，表示该题目所属的考试。
    @param num1 第一个操作数，用于生成数学题目。
    @param op 运算符，表示该题目是加、减、乘、除之一。
    @param num2 第二个操作数，用于生成数学题目。

    @return int 结果，成功返回 1，失败返回 0。
    """
    result = DATABASE_LIB.edit_question_data(
        question_id.encode("utf-8"), exam_id.encode("utf-8"), num1, op, num2
    )
    return 1 if not result else 0


if __name__ == "__main__":
    def test_query_user_info():
        key = "username"
        content = "john_doe"
        user = query_user_info(key, content)
        
        if user:
            print(user)
        else:
            print("User Not Found!")

    def test_query_exam_info():
        key = "id"
        content = "exam_002"
        exam = query_exam_info(key, content)
        
        if exam:
            print(exam)
        else:
            print("Exam Not Found!")

    def test_query_question_info():
        key = "id"
        content = "q002"
        question = query_question_info(key, content)
        
        if question:
            print(question)
        else:
            print("Question Not Found!")

    def test_query_score_info():
        key = "id"
        content = "score_001"
        score = query_score_info(key, content)
        
        if score:
            print(score)
        else:
            print("Score Not Found!")

    def test_query_exams_info_all():
        length = 5  # Example: We want to query top 5 exams
        exams = query_exams_info_all(length)
        
        if exams:
            for exam in exams:
                print(exam)
        else:
            print("No Exams Found!")

    def test_query_users_info_all():
        length = 5  # Example: We want to query top 5 users
        users = query_users_info_all(length)
        
        if users:
            for user in users:
                print(user)
        else:
            print("No Users Found!")

    def test_query_questions_info_all():
        length = 5  # Example: We want to query top 5 questions
        questions = query_questions_info_all(length)
        
        if questions:
            for question in questions:
                print(question)
        else:
            print("No Questions Found!")

    def test_query_scores_info_all():
        length = 5  # Example: We want to query top 5 scores
        scores = query_scores_info_all(length)
        
        if scores:
            for score in scores:
                print(score)
        else:
            print("No Scores Found!")

    def test_insert_exam_data():
        exam_id = "exam_002"
        name = "Math Final Exam"
        start_time = 1700000000  # Example timestamp
        end_time = 1700500000  # Example timestamp
        allow_answer_when_expired = 0  # No
        random_question = 1  # Yes
        
        result = insert_exam_data(exam_id, name, start_time, end_time, allow_answer_when_expired, random_question)
        
        if result == 1:
            print("Exam Data Inserted Successfully!")
        else:
            print("Failed to Insert Exam Data!")

    def test_insert_question_data():
        question_id = "q002"
        exam_id = "exam_002"
        num1 = 5
        op = 1  # Plus
        num2 = 10
        
        result = insert_question_data(question_id, exam_id, num1, op, num2)
        
        if result == 1:
            print("Question Data Inserted Successfully!")
        else:
            print("Failed to Insert Question Data!")

    def test_insert_score_data():
        score_id = "score_001"
        exam_id = "exam_002"
        user_id = "john_doe"
        score = 95.5
        expired_flag = 0  # Not expired
        
        result = insert_score_data(score_id, exam_id, user_id, score, expired_flag)
        
        if result == 1:
            print("Score Data Inserted Successfully!")
        else:
            print("Failed to Insert Score Data!")

    def test_insert_user_data():
        user_id = "user_001"
        username = "john_doe"
        hashpass = "hashedpassword"
        salt = "randomsalt"
        role = 2  # Student
        name = "John Doe"
        class_name = "Computer Science"
        number = 123456
        belong_to = "University XYZ"
        
        result = insert_user_data(user_id, username, hashpass, salt, role, name, class_name, number, belong_to)
        
        if result == 1:
            print("User Data Inserted Successfully!")
        else:
            print("Failed to Insert User Data!")

    # Run all the tests
    def run_tests():
        test_insert_exam_data()
        test_insert_question_data()
        test_insert_score_data()
        test_insert_user_data()
        test_query_user_info()
        test_query_exam_info()
        test_query_question_info()
        test_query_score_info()
        test_query_exams_info_all()
        test_query_users_info_all()
        test_query_questions_info_all()
        test_query_scores_info_all()
    run_tests()
