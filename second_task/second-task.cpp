#include <iostream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <cmath>

// Вершинный шейдер
const char* vertexShaderSource = R"(
    #version 330 core
    layout (location = 0) in vec2 aPos;
    uniform mat4 model; // для масштабирования картинки

    void main()
    {
        gl_Position = model * vec4(aPos.x, aPos.y, 0.0, 1.0);
    }
)";

// Фрагментный шейдер
const char* fragmentShaderSource = R"(
    #version 330 core
    out vec4 FragColor;
    void main()
    {
        FragColor = vec4(1.0, 1.0, 1.0, 1.0);
    }
)";


double*
linspace(double *coords, double start, double stop, int amount)
{
    double step = (stop - start) / (amount - 1);
    for (int i = 0; i < amount; ++i) {
        coords[i] = start + step * i;
    }
    return coords;
}

GLfloat*
calculate_values(double *coords, int coords_len, double(*func)(double))
{
    GLfloat *res = new GLfloat[coords_len * 2];
    for (int i = 0; i < coords_len; ++i) {
        res[2 * i] = coords[i];
        res[2 * i + 1] = func(coords[i]);
    }
    return res;
}

void
create_scale_matrix(float matrix[], int nrows, float scaleX, float scaleY, float scaleZ = 1.0f)
{
    float scaling[] = {scaleX, scaleY, scaleZ, 1.0f};
    // int nrows = sizeof(matrix) / sizeof(matrix[0]);
    for (int i = 0; i < nrows; ++i)
    {
        matrix[i * nrows + i] = scaling[i];
    }
}



int main2() {
    int num_of_dots = 40;
    double* coords = new double[num_of_dots];
    coords = linspace(coords, -1, 1, 40);

    GLfloat *vertices = calculate_values(coords, num_of_dots, sin);
    for (int i = 0; i < num_of_dots * 2; ++i)
        std::cout << vertices[i] << " ";
    std::cout << std::endl;
    delete[] coords;
    return 0;
}

// Основная функция
int main()
{
    // Инициализация GLFW
    if (!glfwInit())
    {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    // Создание окна
    GLFWwindow* window = glfwCreateWindow(1300,900, "Graph", NULL, NULL);
    if (window == NULL)
    {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    // Инициализация GLEW
    if (glewInit() != GLEW_OK)
    {
        std::cerr << "Failed to initialize GLEW" << std::endl;
        return -1;
    }

    // Компиляция и связывание шейдеров
    GLuint vertexShader, fragmentShader;
    GLuint shaderProgram;
    GLint success;
    GLchar infoLog[512];

    // Вершинный шейдер
    vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cerr << "Vertex shader compilation failed\n" << infoLog << std::endl;
        return -1;
    }

    // Фрагментный шейдер
    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cerr << "Fragment shader compilation failed\n" << infoLog << std::endl;
        return -1;
    }

    // Привязка шейдеров к программе
    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cerr << "Shader program linking failed\n" << infoLog << std::endl;
        return -1;
    }
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    int num_of_dots = 100;
    double left_border = -4, right_border = 4;

    double* coords = new double[num_of_dots];
    coords = linspace(coords, left_border, right_border, num_of_dots);

    GLfloat *vertices = calculate_values(coords, num_of_dots, tan);
    // for (int i = 0; i < num_of_dots * 2; ++i)
    //     std::cout << vertices[i] << " ";
    // std::cout << std::endl;
    delete[] coords;

    // Буфер вершин
    GLuint VBO, VAO;
    glGenBuffers(1, &VBO);
    glGenVertexArrays(1, &VAO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, num_of_dots * 2 * sizeof(GLfloat), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // Главный цикл приложения
    while (!glfwWindowShouldClose(window))
    {
        // Обработка ввода
        glfwPollEvents();

        // Очистка экрана
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // Использование шейдерной программы
        glUseProgram(shaderProgram);
        // масштабирование
        GLuint modelLoc = glGetUniformLocation(shaderProgram, "model");
        // std::cout << glGetError() << std::endl;
        float scale_matrix[16] = {0};
        float scaleX = 0.2f, scaleY = 0.1f;
        create_scale_matrix(scale_matrix, 4, scaleX, scaleY);
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, scale_matrix);
        // std::cout << (GLenum)glGetError() << std::endl;

        // Отрисовка графика
        glBindVertexArray(VAO);
        glDrawArrays(GL_LINE_STRIP, 0, num_of_dots);
        glBindVertexArray(0); 

        // Показать результаты отрисовки
        glfwSwapBuffers(window);
    }

    // Освобождение ресурсов
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(shaderProgram);
    delete[] vertices;

    // Завершение работы GLFW
    glfwTerminate();

    return 0;
}