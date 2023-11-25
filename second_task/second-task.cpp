#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <cmath>
#include <vector>
#include <map>

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


template<typename T>
void
linspace(std::vector<T> &v, float start, float stop, int amount)
{
    double step = (stop - start) / (amount - 1);
    if (v.size() < amount) v.resize(amount);
    for (int i = 0; i < v.size(); ++i) {
        v[i] = start + step * i;
    }
}

template<typename T>
std::vector<T>
calculate_values(std::vector<T> v, double(*func)(double))
{
    std::vector<T> ret(v.size() * 2);
    for (int i = 0; i < v.size(); ++i)
    {
        ret[2 * i] = v[i];
        ret[2 * i  + 1] = func(v[i]);
    }
    return ret;
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

// printing for debug
template<typename T>
std::ostream& 
operator<<(std::ostream &out, const std::vector<T> &v)
{
    for (auto &it : v) out << it << " ";
    return out;
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
    GLFWwindow* window = glfwCreateWindow(1000,700, "Graph", NULL, NULL);
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
    GLuint vertexShader, fragmentShader, textFragmentShader, textVertexShader;
    GLuint shaderProgram, textShaderProgram;
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

    // создаем координатную сетку на ось Ox
    std::vector<GLfloat> coords(num_of_dots);
    linspace(coords, left_border, right_border, num_of_dots);

    // Добавляем в массив значения вычисленной на данной сетке функции
    coords = calculate_values(coords, sin);

    int windowWidth, windowHeight;
    glfwGetWindowSize(window, &windowWidth, &windowHeight);
    // Параметры масштабирования
    float scaleX = 0.2f, scaleY = 0.4f;

    std::vector<GLfloat> axis_vertices = {
        -1.0f * (GLfloat)windowWidth / (windowHeight * scaleX), 0.0f,
        1.0f * (GLfloat)windowWidth / (windowHeight * scaleX), 0.0f,
        0.0f, -1.0f * (GLfloat)windowWidth / (windowHeight * scaleY),
        0.0f, 1.0f * (GLfloat)windowWidth / (windowHeight * scaleY)
    };
    // Добавляем координаты для отрисовки координатных осей 
    coords.insert(coords.end(), axis_vertices.begin(), axis_vertices.end());

    // Буфер вершин
    GLuint VBO, VAO;
    glGenBuffers(1, &VBO);
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, coords.size() * sizeof(GLfloat), coords.data(), GL_STATIC_DRAW);
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
        float scale_matrix[16] = {0};
        create_scale_matrix(scale_matrix, 4, scaleX, scaleY);
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, scale_matrix);
        if ((success = glGetError()) != 0) std::cerr << success << ":0" << std::endl;

        // Отрисовка графика
        glBindVertexArray(VAO);
        glDrawArrays(GL_LINE_STRIP, 0, num_of_dots); // Отрисовка графика
        glDrawArrays(GL_LINES, num_of_dots, axis_vertices.size() / 2); // Отрисовка осей
        glBindVertexArray(0);

        // Показать результаты отрисовки
        glfwSwapBuffers(window);
    }

    // Освобождение ресурсов
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(shaderProgram);

    // Завершение работы GLFW
    glfwTerminate();

    return 0;
}