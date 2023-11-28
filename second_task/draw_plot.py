import matplotlib.pyplot as plt
import numpy as np

path_to_fileX = "/home/ubuntu/code/uni/5sem/chmy/second_task/statX.csv"
path_to_fileY = "/home/ubuntu/code/uni/5sem/chmy/second_task/statY.csv"

dataX = np.genfromtxt(path_to_fileX, delimiter=",", dtype=np.float32)
dataY = np.genfromtxt(path_to_fileY, delimiter=",", dtype=np.float32)

assert dataX.shape[0] == dataY.shape[0], "Incorrect array sizes"
not_nan_mask = ~np.isnan(dataY)
dataY = dataY[not_nan_mask]
dataX = dataX[not_nan_mask]
# print(dataY.max(), dataX.max())
plt.grid()
plt.plot(dataX, dataY)
plt.title("График второй нормы как функции номера итерации")
plt.xlabel("Номер итерации")
plt.ylabel("Вторая норма погрешности")
plt.savefig("graph.png")
plt.show()