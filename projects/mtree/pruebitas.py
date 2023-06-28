import random

ls = [['a', 'b', 'c'], ['d', 'e', 'f'], ['g', 'h', 'i'], ['j', 'k', 'l'], ['m', 'n', 'o']]

print(random.choices(ls, k=3))
print(random.sample(ls, 3))