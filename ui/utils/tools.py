import string
import random

def generate_salt(length: int = 16) -> str:
    return "".join(random.choices(string.ascii_letters + string.digits, k=16))    