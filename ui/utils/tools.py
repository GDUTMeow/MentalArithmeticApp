import string
import random
import openpyxl
from io import BytesIO
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


def c_strlen(s):
    """
    模拟C语言的strlen函数，计算字符串的长度。
    中文字符（C语言UTF-8编码下占3个字节）计为3，其他字符计为1。

    参数:
        s (str): 输入的字符串。

    返回:
        int: 按C语言算法计算的字符串长度。
    """
    length = 0
    for char in s:
        code_point = ord(char)
        # 判断是否为中文字符
        if (
            0x4E00 <= code_point <= 0x9FFF or      # CJK统一汉字
            0x3400 <= code_point <= 0x4DBF or      # CJK统一汉字扩展A
            0x20000 <= code_point <= 0x2A6DF or    # CJK统一汉字扩展B
            0x2A700 <= code_point <= 0x2B73F or    # CJK统一汉字扩展C
            0x2B740 <= code_point <= 0x2B81F or    # CJK统一汉字扩展D
            0x2B820 <= code_point <= 0x2CEAF or    # CJK统一汉字扩展E
            0xF900 <= code_point <= 0xFAFF or      # CJK兼容汉字
            0x2F800 <= code_point <= 0x2FA1F       # CJK兼容汉字补充
        ):
            length += 3
        else:
            length += 1
    return length


def calculate_score(question_list: list, user_answer_list: list) -> int:
    """
    @brief 计算用户得分。

    @param question_list 问题列表
    @param user_answer_list 用户答案列表
    @return int 得分
    """
    # 计算总分
    total_score = 0
    score_per_question = 100 / len(question_list)
    for question, answer in zip(question_list, user_answer_list):
        if judge(
            calculate_result(
                question.get("num1"), question.get("num2"), question.get("op")
            ),
            answer,
        ):
            total_score += score_per_question

    return int(total_score)  # 分数只能是整数，否则数据库存储方面存储后会出问题


def questions_xlsx_parse(raw_data: bytes) -> list:
    """
    @brief 解析传入的xlsx的数据

    @param data 传入的二进制xlsx文件数据
    @return list 解析后的数据列表
    """
    try:
        # 将传入的二进制数据流转为ByteIO对象
        data = BytesIO(raw_data)
        # 打开Excel工作簿
        wb = openpyxl.load_workbook(data, data_only=True)
    except Exception as e:
        print(f"无法打开文件 {data}: {e}")
        return

    ws = wb.active

    results = []

    # 遍历每一行（跳过表头）
    for row_idx, row in enumerate(ws.iter_rows(min_row=2, values_only=True), start=2):
        try:
            num1, operator, num2 = row[:3]

            # 检查是否为整数
            if (
                not isinstance(num1, int)
                or not isinstance(operator, int)
                or not isinstance(num2, int)
            ):
                print(f"第{row_idx}行：数据类型错误，跳过计算。")
                continue

            if operator == 3:
                if num2 == 0:
                    print(f"第{row_idx}行：除数不能为0，跳过计算。")
                    continue
            elif operator not in [0, 1, 2, 3]:
                print(f"第{row_idx}行：无效的操作符 {operator}，跳过计算。")
                continue
            operation = operator
            results.append((num1, operation, num2))

        except Exception as e:
            print(f"第{row_idx}行：发生错误: {e}，跳过计算。")
            continue

    return results


def students_xlsx_parser(raw_data: bytes) -> list:
    """
    @brief 解析传入的xlsx的数据

    @param data 传入的二进制xlsx文件数据
    @return list 解析后的数据列表
    """
    try:
        # 将传入的二进制数据流转为ByteIO对象
        data = BytesIO(raw_data)
        # 打开Excel工作簿
        wb = openpyxl.load_workbook(data, data_only=True)
    except Exception as e:
        print(f"无法打开文件 {data}: {e}")
        return

    ws = wb.active

    results = []

    # 遍历每一行（跳过表头）
    for row_idx, row in enumerate(ws.iter_rows(min_row=2, values_only=True), start=2):
        try:
            number, name, student_class = row[:3]
            if number > 4294967295 or number <= 0:
                print(f"第{row_idx}行：学号不符合要求，跳过此行数据 {number}, {name}, {student_class}")
                continue
            if c_strlen(name) > 45:
                print(f"第{row_idx}行：姓名长度不符合要求，跳过此行数据 {number}, {name}, {student_class}")
                continue
            if c_strlen(student_class) > 30:
                print(f"第{row_idx}行：班级长度不符合要求，跳过此行数据 {number}, {name}, {student_class}")
                continue
        except Exception as e:
            print(f"第{row_idx}行：发生错误: {e}")
            continue

    return results
