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
plt.figure(figsize=(11, 7))
plt.grid()
plt.plot(dataX, dataY)
fontsize = 17
plt.title("График второй нормы как функции номера итерации", fontsize=fontsize)
plt.xlabel("Номер итерации", fontsize=fontsize)
plt.ylabel("Вторая норма погрешности", fontsize=fontsize)
plt.savefig("graph.png")
plt.show()
