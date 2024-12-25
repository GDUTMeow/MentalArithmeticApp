# C语言函数调用器，在这里定义了一些函数，用于直接调用我需要的C语言代码

import ctypes
import os
from ctypes import c_char_p, c_int, POINTER, c_float, c_uint

# dll链接
APP_LIB = ctypes.CDLL(os.path.join(os.getcwd(), "app.dll"))
DATABASE_LIB = ctypes.CDLL(os.path.join(os.getcwd(), "database.dll"))
INITIALIZER_LIB = ctypes.CDLL(os.path.join(os.getcwd(), "initializer.dll"))

class Permission(ctypes.Structure):
    """
    表示用户的权限设置。

    Attributes:
        stu_answer (ctypes.c_int): 学生答题权限。
        stu_inspect_personal_info (ctypes.c_int): 学生查看个人信息权限。
        stu_inspect_exam_info (ctypes.c_int): 学生查看考试信息权限。
        tea_manage_exam (ctypes.c_int): 教师管理考试权限。
        tea_manage_student (ctypes.c_int): 教师管理学生权限。
        tea_inspect_student_info (ctypes.c_int): 教师查看学生信息权限。
        tea_inspect_exam_scores (ctypes.c_int): 教师查看考试成绩权限。
        general_edit_info (ctypes.c_int): 一般编辑信息权限。
    """

    _fields_ = [
        ("stu_answer", ctypes.c_int),
        ("stu_inspect_personal_info", ctypes.c_int),
        ("stu_inspect_exam_info", ctypes.c_int),
        ("tea_manage_exam", ctypes.c_int),
        ("tea_manage_student", ctypes.c_int),
        ("tea_inspect_student_info", ctypes.c_int),
        ("tea_inspect_exam_scores", ctypes.c_int),
        ("general_edit_info", ctypes.c_int),
    ]

    def __repr__(self):
        return (
            f"Permission(stu_answer={self.stu_answer}, "
            f"stu_inspect_personal_info={self.stu_inspect_personal_info}, "
            f"stu_inspect_exam_info={self.stu_inspect_exam_info}, "
            f"tea_manage_exam={self.tea_manage_exam}, "
            f"tea_manage_student={self.tea_manage_student}, "
            f"tea_inspect_student_info={self.tea_inspect_student_info}, "
            f"tea_inspect_exam_scores={self.tea_inspect_exam_scores}, "
            f"general_edit_info={self.general_edit_info})"
        )


class User(ctypes.Structure):
    """
    表示系统中的用户。

    Attributes:
        id (ctypes.c_char * 37): 用户的唯一标识符。
        username (ctypes.c_char * 25): 用户的用户名。
        role (ctypes.c_int): 用户的角色标识符。
        name (ctypes.c_char * 46): 用户的真实姓名。
        class_name (ctypes.c_char * 50): 用户所属的班级名称。
        number (ctypes.c_uint): 用户的工号。
        belong_to (ctypes.c_char * 50): 用户所属的部门或组织。
        permission (Permission): 用户的权限级别。
    """

    _fields_ = [
        ("id", ctypes.c_char * 37),
        ("username", ctypes.c_char * 25),
        ("role", ctypes.c_int),
        ("name", ctypes.c_char * 46),
        ("class_name", ctypes.c_char * 31),
        ("number", ctypes.c_uint),
        ("belong_to", ctypes.c_char * 37),
        ("permission", Permission),
    ]

    def __repr__(self):
        return (
            f"User(id={self.id.decode('utf-8')}, "
            f"username={self.username.decode('utf-8')}, "
            f"role={self.role}, "
            f"name={self.name.decode('utf-8')}, "
            f"class_name={self.class_name.decode('utf-8')}, "
            f"number={self.number}, "
            f"belong_to={self.belong_to.decode('utf-8')}, "
            f"permission={self.permission})"
        )


class SqlResponseExam(ctypes.Structure):
    """
    表示考试信息的响应结构。

    Attributes:
        id (ctypes.c_char * 37): 考试的唯一标识符。
        name (ctypes.c_char * 91): 考试的名称。
        start_time (ctypes.c_int): 考试的开始时间（时间戳）。
        end_time (ctypes.c_int): 考试的结束时间（时间戳）。
        allow_answer_when_expired (ctypes.c_int): 考试过期后是否允许答题。
        random_question (ctypes.c_int): 是否随机生成试题。
    """

    _fields_ = [
        ("id", ctypes.c_char * 37),
        ("name", ctypes.c_char * 91),
        ("start_time", ctypes.c_int),
        ("end_time", ctypes.c_int),
        ("allow_answer_when_expired", ctypes.c_int),
        ("random_question", ctypes.c_int),
    ]

    def __repr__(self):
        return (
            f"SqlResponseExam(id={self.id.decode('utf-8')}, "
            f"name={self.name.decode('utf-8')}, "
            f"start_time={self.start_time}, "
            f"end_time={self.end_time}, "
            f"allow_answer_when_expired={self.allow_answer_when_expired}, "
            f"random_question={self.random_question})"
        )


class SqlResponseQuestion(ctypes.Structure):
    """
    表示试题信息的响应结构。

    Attributes:
        id (ctypes.c_char * 37): 试题的唯一标识符。
        exam_id (ctypes.c_char * 37): 所属考试的唯一标识符。
        num1 (ctypes.c_int): 第一个数字（用于运算）。
        op (ctypes.c_int): 运算符（例如，加、减、乘、除）。
        num2 (ctypes.c_int): 第二个数字（用于运算）。
    """

    _fields_ = [
        ("id", ctypes.c_char * 37),
        ("exam_id", ctypes.c_char * 37),
        ("num1", ctypes.c_int),
        ("op", ctypes.c_int),
        ("num2", ctypes.c_int),
    ]

    def __repr__(self):
        return (
            f"SqlResponseQuestion(id={self.id.decode('utf-8')}, "
            f"exam_id={self.exam_id.decode('utf-8')}, "
            f"num1={self.num1}, "
            f"op={self.op}, "
            f"num2={self.num2})"
        )


class SqlResponseScore(ctypes.Structure):
    """
    表示成绩信息的响应结构。

    Attributes:
        id (ctypes.c_char * 37): 成绩记录的唯一标识符。
        exam_id (ctypes.c_char * 37): 所属考试的唯一标识符。
        user_id (ctypes.c_char * 37): 所属用户的唯一标识符。
        score (ctypes.c_int): 用户在考试中的得分。
        expired_flag (ctypes.c_int): 成绩是否已过期的标志。
    """

    _fields_ = [
        ("id", ctypes.c_char * 37),
        ("exam_id", ctypes.c_char * 37),
        ("user_id", ctypes.c_char * 37),
        ("score", ctypes.c_int),
        ("expired_flag", ctypes.c_int),
    ]

    def __repr__(self):
        return (
            f"SqlResponseScore(id={self.id.decode('utf-8')}, "
            f"exam_id={self.exam_id.decode('utf-8')}, "
            f"user_id={self.user_id.decode('utf-8')}, "
            f"score={self.score}, "
            f"expired_flag={self.expired_flag})"
        )


class SqlResponseUser(ctypes.Structure):
    """
    表示用户信息的响应结构。

    Attributes:
        id (ctypes.c_char * 37): 用户的唯一标识符。
        username (ctypes.c_char * 25): 用户的用户名。
        hashpass (ctypes.c_char * 129): 用户密码的哈希值。
        salt (ctypes.c_char * 17): 用于密码哈希的盐值。
        role (ctypes.c_int): 用户的角色标识符。
        name (ctypes.c_char * 46): 用户的真实姓名。
        class_name (ctypes.c_char * 31): 用户所属的班级名称。
        number (ctypes.c_uint): 用户的工号。
        belong_to (ctypes.c_char * 37): 用户所属的部门或组织。
    """

    _fields_ = [
        ("id", ctypes.c_char * 37),
        ("username", ctypes.c_char * 25),
        ("hashpass", ctypes.c_char * 129),
        ("salt", ctypes.c_char * 17),
        ("role", ctypes.c_int),
        ("name", ctypes.c_char * 46),
        ("class_name", ctypes.c_char * 31),
        ("number", ctypes.c_uint),
        ("belong_to", ctypes.c_char * 37),
    ]

    def __repr__(self):
        return (
            f"SqlResponseUser(id={self.id.decode('utf-8')}, "
            f"username={self.username.decode('utf-8')}, "
            f"hashpass={self.hashpass.decode('utf-8')}, "
            f"salt={self.salt.decode('utf-8')}, "
            f"role={self.role}, "
            f"name={self.name.decode('utf-8')}, "
            f"class_name={self.class_name.decode('utf-8')}, "
            f"number={self.number}, "
            f"belong_to={self.belong_to.decode('utf-8')})"
        )


class QuestionData(ctypes.Structure):
    _fields_ = [
        ("num1", c_int),  # int
        ("op", c_int),  # int
        ("num2", c_int),  # int
    ]

    def __repr__(self):
        return f"QuestionData(num1={self.num1}, op={self.op}, num2={self.num2})"


class Question(ctypes.Structure):
    pass  # self-referential


Question._fields_ = [
    ("data", QuestionData),
    ("next_question", POINTER(Question)),
]


def question_repr_with_cycle_detection(self):
    questions = []
    visited = set()
    current = self
    while current and id(current) not in visited:
        questions.append(repr(current.data))
        visited.add(id(current))
        if current.next_question:
            current = current.next_question.contents
        else:
            current = None
    if current:
        questions.append("... (cycle detected)")
    return " -> ".join(questions)


Question.__repr__ = question_repr_with_cycle_detection

# 定义数据库函数的原型及返回值
DATABASE_LIB.query_user_info.argtypes = [c_char_p, c_char_p, POINTER(User)]
DATABASE_LIB.query_user_info.restype = c_int

DATABASE_LIB.query_exam_info.argtypes = [c_char_p, c_char_p, POINTER(SqlResponseExam)]
DATABASE_LIB.query_exam_info.restype = c_int

DATABASE_LIB.query_question_info.argtypes = [
    c_char_p,
    c_char_p,
    POINTER(SqlResponseQuestion),
]
DATABASE_LIB.query_question_info.restype = c_int

DATABASE_LIB.query_score_info.argtypes = [c_char_p, c_char_p, POINTER(SqlResponseScore)]
DATABASE_LIB.query_score_info.restype = c_int

DATABASE_LIB.query_exams_info_all.argtypes = [
    POINTER(SqlResponseExam),
    c_int,
    c_char_p,
    c_char_p,
]
DATABASE_LIB.query_exams_info_all.restype = c_int

DATABASE_LIB.query_users_info_all.argtypes = [
    POINTER(SqlResponseUser),
    c_int,
    c_char_p,
    c_char_p,
]
DATABASE_LIB.query_users_info_all.restype = c_int

DATABASE_LIB.query_questions_info_all.argtypes = [
    POINTER(SqlResponseQuestion),
    c_int,
    c_char_p,
    c_char_p,
]
DATABASE_LIB.query_questions_info_all.restype = c_int

DATABASE_LIB.query_scores_info_all.argtypes = [
    POINTER(SqlResponseScore),
    c_int,
    c_char_p,
    c_char_p,
]
DATABASE_LIB.query_scores_info_all.restype = c_int

DATABASE_LIB.del_user_data.argtypes = [ctypes.c_char_p]
DATABASE_LIB.del_user_data.restype = ctypes.c_int

DATABASE_LIB.del_exam_data.argtypes = [ctypes.c_char_p]
DATABASE_LIB.del_exam_data.restype = ctypes.c_int

DATABASE_LIB.del_score_data.argtypes = [ctypes.c_char_p]
DATABASE_LIB.del_score_data.restype = ctypes.c_int

DATABASE_LIB.del_question_data.argtypes = [ctypes.c_char_p]
DATABASE_LIB.del_question_data.restype = ctypes.c_int

DATABASE_LIB.insert_exam_data.argtypes = [
    ctypes.c_char_p,  # exam_id
    ctypes.c_char_p,  # name
    ctypes.c_int,  # start_time
    ctypes.c_int,  # end_time
    ctypes.c_int,  # allow_answer_when_expired
    ctypes.c_int,  # random_question
]

DATABASE_LIB.insert_question_data.argtypes = [
    ctypes.c_char_p,  # question_id
    ctypes.c_char_p,  # exam_id
    ctypes.c_int,  # num1
    ctypes.c_int,  # op
    ctypes.c_int,  # num2
]


DATABASE_LIB.insert_score_data.argtypes = [
    ctypes.c_char_p,  # score_id
    ctypes.c_char_p,  # exam_id
    ctypes.c_char_p,  # user_id
    ctypes.c_int,  # score
    ctypes.c_int,  # expired_flag
]

DATABASE_LIB.insert_user_data.argtypes = [
    ctypes.c_char_p,  # user_id
    ctypes.c_char_p,  # username
    ctypes.c_char_p,  # hashpass
    ctypes.c_char_p,  # salt
    ctypes.c_int,  # role
    ctypes.c_char_p,  # name
    ctypes.c_char_p,  # class_name
    ctypes.c_uint,  # number
    ctypes.c_char_p,  # belong_to
]

DATABASE_LIB.edit_user_data.argtypes = [
    ctypes.c_char_p,  # user_id
    ctypes.c_char_p,  # username
    ctypes.c_char_p,  # hashpass
    ctypes.c_char_p,  # salt
    ctypes.c_int,  # role
    ctypes.c_char_p,  # name
    ctypes.c_char_p,  # class_name
    ctypes.c_uint,  # number
    ctypes.c_char_p,  # belong_to
]
DATABASE_LIB.edit_user_data.restype = ctypes.c_int

DATABASE_LIB.edit_exam_data.argtypes = [
    ctypes.c_char_p,  # exam_id
    ctypes.c_char_p,  # name
    ctypes.c_int,  # start_time
    ctypes.c_int,  # end_time
    ctypes.c_int,  # allow_answer_when_expired
    ctypes.c_int,  # random_question
]
DATABASE_LIB.edit_exam_data.restype = ctypes.c_int

DATABASE_LIB.edit_score_data.argtypes = [
    ctypes.c_char_p,  # score_id
    ctypes.c_char_p,  # exam_id
    ctypes.c_char_p,  # user_id
    ctypes.c_int,  # score
    ctypes.c_int,  # expired_flag
]
DATABASE_LIB.edit_score_data.restype = ctypes.c_int

DATABASE_LIB.edit_question_data.argtypes = [
    ctypes.c_char_p,  # question_id
    ctypes.c_char_p,  # exam_id
    ctypes.c_int,  # num1
    ctypes.c_int,  # op
    ctypes.c_int,  # num2
]
DATABASE_LIB.edit_question_data.restype = ctypes.c_int

APP_LIB.generate_question_list.argtypes = [c_char_p, POINTER(Question), c_int]
APP_LIB.generate_question_list.restype = c_int

APP_LIB.randomize_question_list.argtypes = [POINTER(Question), POINTER(Question)]
APP_LIB.randomize_question_list.restype = c_int

APP_LIB.free_question_list.argtypes = [POINTER(Question)]
APP_LIB.free_question_list.restype = None

APP_LIB.calculate_result.argtypes = [c_int, c_int, c_int]
APP_LIB.calculate_result.restype = c_float

APP_LIB.judge.argtypes = [c_float, c_float]
APP_LIB.judge.restype = c_int

INITIALIZER_LIB.initialize.argtypes = []
INITIALIZER_LIB.initialize.restype = None