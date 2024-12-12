# C语言函数调用器，在这里定义了一些函数，用于直接调用我需要的C语言代码

import ctypes
from ctypes import c_char_p, c_int, POINTER

# 数据库dll
DATABASE_LIB: ctypes.CDLL = ctypes.CDLL("database.dll")


class User(ctypes.Structure):
    _fields_ = [
        ("id", ctypes.c_char * 37),
        ("username", ctypes.c_char * 37),
        ("role", ctypes.c_int),
        ("name", ctypes.c_char * 100),
        ("class_name", ctypes.c_char * 50),
        ("number", ctypes.c_uint),
        ("belong_to", ctypes.c_char * 50),
        ("permission", ctypes.c_int),
    ]

    def __repr__(self):
        return f"User(id={self.id}, username={self.username}, role={self.role}, name={self.name}, class_name={self.class_name}, number={self.number}, belong_to={self.belong_to}, permission={self.permission})"


class Permission(ctypes.Structure):
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
        return f"Permission(stu_answer={self.stu_answer}, stu_inspect_personal_info={self.stu_inspect_personal_info}, stu_inspect_exam_info={self.stu_inspect_exam_info}, tea_manage_exam={self.tea_manage_exam}, tea_manage_student={self.tea_manage_student}, tea_inspect_student_info={self.tea_inspect_student_info}, tea_inspect_exam_scores={self.tea_inspect_exam_scores}, general_edit_info={self.general_edit_info})"


class SqlResponseExam(ctypes.Structure):
    _fields_ = [
        ("id", ctypes.c_char * 37),
        ("name", ctypes.c_char * 100),
        ("start_time", ctypes.c_int),
        ("end_time", ctypes.c_int),
        ("allow_answer_when_expired", ctypes.c_int),
        ("random_question", ctypes.c_int),
    ]

    def __repr__(self):
        return f"SqlResponseExam(id={self.id}, name={self.name}, start_time={self.start_time}, end_time={self.end_time}, allow_answer_when_expired={self.allow_answer_when_expired}, random_question={self.random_question})"


class SqlResponseQuestion(ctypes.Structure):
    _fields_ = [
        ("id", ctypes.c_char * 37),
        ("exam_id", ctypes.c_char * 37),
        ("num1", ctypes.c_double),
        ("op", ctypes.c_int),
        ("num2", ctypes.c_double),
    ]

    def __repr__(self):
        return f"SqlResponseQuestion(id={self.id}, exam_id={self.exam_id}, num1={self.num1}, op={self.op}, num2={self.num2})"


class SqlResponseScore(ctypes.Structure):
    _fields_ = [
        ("id", ctypes.c_char * 37),
        ("exam_id", ctypes.c_char * 37),
        ("user_id", ctypes.c_char * 37),
        ("score", ctypes.c_double),
        ("expired_flag", ctypes.c_int),
    ]

    def __repr__(self):
        return f"SqlResponseScore(id={self.id}, exam_id={self.exam_id}, user_id={self.user_id}, score={self.score}, expired_flag={self.expired_flag})"


class SqlResponseUser(ctypes.Structure):
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
        return f"SqlResponseUser(id={self.id}, username={self.username}, hashpass={self.hashpass}, salt={self.salt}, role={self.role}, name={self.name}, class_name={self.class_name}, number={self.number}, belong_to={self.belong_to})"


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

DATABASE_LIB.query_exams_info_all.argtypes = [POINTER(SqlResponseExam), c_int]
DATABASE_LIB.query_exams_info_all.restype = c_int

DATABASE_LIB.query_users_info_all.argtypes = [POINTER(SqlResponseUser), c_int]
DATABASE_LIB.query_users_info_all.restype = c_int

DATABASE_LIB.query_questions_info_all.argtypes = [POINTER(SqlResponseQuestion), c_int]
DATABASE_LIB.query_questions_info_all.restype = c_int

DATABASE_LIB.query_scores_info_all.argtypes = [POINTER(SqlResponseScore), c_int]
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
    ctypes.c_float,  # num1
    ctypes.c_int,  # op
    ctypes.c_float,  # num2
]


DATABASE_LIB.insert_score_data.argtypes = [
    ctypes.c_char_p,  # score_id
    ctypes.c_char_p,  # exam_id
    ctypes.c_char_p,  # user_id
    ctypes.c_float,  # score
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
    ctypes.c_int,  # number
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
    ctypes.c_int,  # number
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


