import numpy as np

left_bound, right_bound = -10, 10

def generate_positive_definite_matrix(size):
    # Генерация случайной матрицы
    # random_matrix = np.random.rand(size, size) * (right_bound - left_bound)
    random_matrix = np.random.rand(size, size)

    # Создание симметричной матрицы
    symmetric_matrix = (random_matrix + random_matrix.T) / 2
    symmetric_matrix = np.round(symmetric_matrix)
    
    # Добавление диагональных элементов для обеспечения положительной определенности
    positive_definite_matrix = symmetric_matrix + size * np.eye(size)
    
    return positive_definite_matrix.astype(np.int32)

def main():
    # Пример для 5x5 матрицы
    size = 5
    matrix = generate_positive_definite_matrix(size)
    for item in matrix:
        print('{', sep='', end='')
        print(*item, sep=', ', end='},\n')
    print()
    # x = (np.random.rand(size) * (right_bound - left_bound)).astype(np.int32)
    x = (np.random.rand(size))
    print('{', sep='', end='')
    print(*x, sep=', ', end='}')
    print()
    res = matrix @ x
    print('res: ', res)

if __name__ == "__main__":
    for i in range(6, 22):
        print(2 ** i, end=' ')