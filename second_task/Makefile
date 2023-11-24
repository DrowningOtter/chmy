EXEC_NAME=prog
all:
	g++ second-task.cpp -lGLEW -lGLU -lGL `pkg-config --static --libs glfw3` -std=c++11 -o ${EXEC_NAME}
run: all
	./${EXEC_NAME}
