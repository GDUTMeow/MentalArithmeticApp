from . import *


def traverse_question_list(question_list: list):
    """
    @brief 遍历问题链表并提取数据

    @param question_list 包含问题数据的列表
    @return 列表，每个元素是一个字典，包含问题数据
    """
    questions = []
    for item in question_list:
        questions.append({"num1": item.num1, "op": item.op, "num2": item.num2, "id": item.id.decode(), "exam_id": item.exam_id.decode()})
    return questions


def generate_question_list(exam_id_str, length: int):
    """
    @brief 调用 C 函数 generate_question_list 来生成问题链表。

    @param exam_id_str 考试 ID 字符串
    @return POINTER(Question) 链表头指针
    """
    # 转换考试 ID 为字节字符串
    exam_id = exam_id_str.encode("utf-8")

    # 分配内存给 question_list_to_return
    question_list = Question()
    ctypes.memset(ctypes.byref(question_list), 0, ctypes.sizeof(Question))

    # 调用 generate_question_list 函数
    result = APP_LIB.generate_question_list(
        c_char_p(exam_id), ctypes.byref(question_list), c_int(length)
    )

    if result != 0:
        raise Exception("Failed to generate question list")

    return ctypes.pointer(question_list)


def randomize_question_list(original_list_ptr):
    """
    @brief 调用 C 函数 randomize_question_list 来随机化问题链表。

    @param original_list_ptr POINTER(Question) 原始链表头指针
    @return POINTER(Question) 随机化后的链表头指针
    """
    # 分配内存给 question_list_to_return
    randomized_list = Question()
    ctypes.memset(ctypes.byref(randomized_list), 0, ctypes.sizeof(Question))

    # 调用 randomize_question_list 函数
    result = APP_LIB.randomize_question_list(
        ctypes.byref(randomized_list), original_list_ptr
    )

    if result != 0:
        raise Exception("Failed to randomize question list")

    return ctypes.pointer(randomized_list)


def free_question_list_func(head_ptr):
    """
    @brief 调用 C 函数 free_question_list 来释放问题链表的内存。

    @param head_ptr POINTER(Question) 链表头指针
    """
    APP_LIB.free_question_list(head_ptr)


if __name__ == "__main__":
    try:
        exam_id = "0fcec351-7bd9-4b8a-a7f3-84bdc7150324"

        # 生成问题链表
        original_list_ptr = generate_question_list(exam_id, 4)
        print("Original Question List:")
        original_questions = traverse_question_list(original_list_ptr)
        for idx, item in enumerate(original_questions, 1):
            print(f"Question {idx}: {item['num1']} {item['op']} {item['num2']}")

        # 随机化问题链表
        randomized_list_ptr = randomize_question_list(original_list_ptr)
        print("\nRandomized Question List:")
        randomized_questions = traverse_question_list(randomized_list_ptr)
        for idx, item in enumerate(randomized_questions, 1):
            print(f"Question {idx}: {item['num1']} {item['op']} {item['num2']}")

    except Exception as e:
        print(f"Error: {e}")
    finally:
        # 释放内存
        if "original_list_ptr" in locals():
            free_question_list_func(original_list_ptr)
        if "randomized_list_ptr" in locals():
            free_question_list_func(randomized_list_ptr)
