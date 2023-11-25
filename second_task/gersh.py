import numpy as np

# Создаем матрицу
file_path = '../SLAU_var_2.csv'
matrix = np.loadtxt(file_path, delimiter=',')

# Получаем количество строк в матрице
n = len(matrix)

# Создаем список для хранения оценок Гершгорина
gerggorin_bounds = []

# Проходим по каждой строке матрицы
for i in range(n):
    # Вычисляем центр круга Гершгорина (диагональный элемент)
    center = matrix[i, i]
    
    # Вычисляем радиус круга Гершгорина (сумма абсолютных значений остальных элементов в строке)
    radius = np.sum(np.abs(matrix[i, :])) - np.abs(center)
    
    # Добавляем оценку в список
    gerggorin_bounds.append((center, radius))

# Выводим оценки Гершгорина
for i, bounds in enumerate(gerggorin_bounds):
    print(f"Круг Гершгорина {i + 1}: Центр = {bounds[0]}, Радиус = {bounds[1]}")

# Получаем собственные значения матрицы
eigenvalues = np.linalg.eigvals(matrix)

# Выводим собственные значения матрицы
print("Спектр матрицы (собственные значения):", eigenvalues)
