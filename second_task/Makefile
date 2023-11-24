EXEC_NAME=prog
all:
	g++ second-task.cpp -lGLEW -lGLU -lGL `pkg-config --static --libs glfw3` -lfreetype -std=c++11 -o ${EXEC_NAME} -I /usr/include/freetype2
run: all
	./${EXEC_NAME}
