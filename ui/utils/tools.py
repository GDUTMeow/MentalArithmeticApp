import string
import random
from .app import judge, calculate_result

def generate_salt(length: int = 16) -> str:
    """
    @brief 生成哈希盐值
    
    @param length 哈希盐值长度
    @return str 盐值
    """
    return "".join(random.choices(string.ascii_letters + string.digits, k=16))    

def generate_correct_answer_list(question_list: list) -> list[float]:
    """
    @brief 生成正确答案列表。

    @param question_list 问题列表
    @return list[float] 正确答案列表
    """
    return [question["correct_answer"] for question in question_list]

def calculate_score(question_list: list, user_answer_list: list) -> float:
    """
    @brief 计算用户得分。

    @param question_list 问题列表
    @param user_answer_list 用户答案列表
    @return float 得分
    """
    # 计算总分
    total_score = 0
    score_per_question = 100 / len(question_list)
    for question, answer in zip(question_list, user_answer_list):
        if judge(calculate_result(question.get("num1"), question.get("num2"), question.get("op")), answer):
            total_score += score_per_question

    return round(total_score,2)