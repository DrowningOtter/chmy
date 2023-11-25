import numpy as np
import matplotlib.pyplot as plt

def gershgorin_bounds(matrix):
    n = matrix.shape[0]
    bounds = []

    for i in range(n):
        center = matrix[i, i]
        radius = np.sum(np.abs(matrix[i, :])) - np.abs(center)
        bounds.append((center, radius))

    return bounds

# Создание симметричной положительно определенной матрицы
A = np.array([[4, -1, 0, -1],
              [-1, 4, -1, 0],
              [0, -1, 4, -1],
              [-1, 0, -1, 4]])

# Получение оценок Гершгорина
bounds = gershgorin_bounds(A)

# Печать оценок
for i, (center, radius) in enumerate(bounds):
    print(f"Eigenvalue {i + 1} lies in the circle with center {center} and radius {radius}.")

# Визуализация
fig, ax = plt.subplots()
for center, radius in bounds:
    circle = plt.Circle((center, 0), radius, fill=False, color='r', linestyle='dashed')
    ax.add_patch(circle)

# Подписи и оси
ax.set_aspect('equal', adjustable='datalim')
ax.set_xlabel('Real')
ax.set_ylabel('Imaginary')
plt.title('Gershgorin Bounds for Eigenvalues')
plt.grid(True)
plt.show()
