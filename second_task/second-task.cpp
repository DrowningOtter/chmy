#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <cmath>
#include <vector>
#include <map>
#include <algorithm>

#include "../lu.cpp"

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

void
create_model_matrix(float matrix[], int nrows, float scaleX, float scaleY, float translateX = 0.0f, float translateY = 0.0f)
{
    matrix[0] = scaleX;
    matrix[5] = scaleY;
    matrix[10] = 1.0f;
    matrix[12] = translateX;
    matrix[13] = translateY;
    matrix[15] = 1.0f;
}

// Перегрузки операторов
template<typename T>
std::ostream& 
operator<<(std::ostream &out, const std::vector<T> &v)
{
    for (auto &it : v) out << it << " ";
    return out;
}

template<typename T>
std::vector<T>
operator*(const std::vector<std::vector<T>> &m1, const std::vector<std::vector<T>> &m2)
{
    if (m1[0].size() != m2.size()) throw "Matrix sizes doesnt match";
    std::vector<std::vector<T>> res(m1.size());
    for (auto &it : res) it.resize(m2[0].size());
    for (int i = 0; i < m1.size(); ++i) 
    {
        for (int j = 0; j < m2[0].size(); ++j)
        {
            double sum = 0.0;
            for (int k = 0; k < m1[0].size(); ++k)
            {
                sum += m1[i][k] * m2[k][j];
            }
            res[i][j] = sum;
        }
    }
    return res;
}

template<typename T>
std::vector<T>
operator*(const std::vector<std::vector<T>> &m, const std::vector<T> &x)
{
    if (m[0].size() != x.size()) throw "Matrix and vector sizes doesnt match";
    std::vector<T> ret(x.size());
    for (int i = 0; i < x.size(); ++i)
    {
        double sum = 0.0;
        for (int j = 0; j < x.size(); ++j)
        {
            sum += m[i][j] * x[j];
        }
        ret[i] = sum;
    }
    return ret;
}

template<typename T>
std::vector<T>
operator-(const std::vector<T> &v1, const std::vector<T> &v2)
{
    if (v1.size() != v2.size()) throw "Vectors sizes doesnt match";
    std::vector<T> ret(v1.size());
    for (int i = 0; i < v1.size(); ++i)
    {
        ret[i] = v1[i] - v2[i];
    }
    return ret;
}


int draw_plot(std::vector<float> coords, float scaleX, float scaleY, float translateX=0.0f, float translateY=0.0f)
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

    int windowWidth, windowHeight;
    glfwGetWindowSize(window, &windowWidth, &windowHeight);

    std::vector<GLfloat> axis_vertices = {
        -1.0f * (GLfloat)windowWidth / (windowHeight * scaleX), 0.0f,
        1.0f * (GLfloat)windowWidth / (windowHeight * scaleX), 0.0f,
        0.0f, -1.0f * (GLfloat)windowWidth / (windowHeight * scaleY),
        0.0f, 1.0f * (GLfloat)windowWidth / (windowHeight * scaleY)
    };

    // Добавляем координаты для отрисовки координатных осей
    // if (X.size() != Y.size()) throw "Input arrays must have same size!\n";
    // std::vector<GLfloat> coords(X.size() + Y.size());
    // for (int i = 0; i < X.size(); ++i)
    // {
    //     coords[2 * i] = X[i];
    //     coords[2 * i + 1] = Y[i];
    // }
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
        create_model_matrix(scale_matrix, 4, scaleX, scaleY, translateX, translateY);
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, scale_matrix);
        if ((success = glGetError()) != 0) std::cerr << success << ":0" << std::endl;

        // Отрисовка графика
        glBindVertexArray(VAO);
        int num_of_dots = (coords.size() - axis_vertices.size()) / 2;
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

int test_draw()
{
    std::vector<float> data;
    linspace(data, 0, 100, 20000);
    std::vector<float> coords(data.size() * 2);
    for (int i = 0; i < data.size(); ++i)
    {
        coords[2 * i] = data[i];
        coords[2 * i + 1] = sin(data[i]);
    }
    draw_plot(data, 0.2f, 0.5f);
    return 0;
}

// функция для постороения множества тетта, которое используется для генерации 
// последовательности оптимальных итерационных параметров
std::vector<int>
theta_set_construction(int m)
{
    if ((m & (m - 1)) != 0) throw "Argument m must be power of 2";
    if (m == 1) return std::vector<int>{0, 1};
    m = m / 2;
    std::vector<int> smaller_set = theta_set_construction(m);
    std::vector<int> ret(m * 2 + 1);
    for (int i = 1; i <= m; ++i)
    {
        ret[2 * i] = 4 * m - smaller_set[i];
        ret[2 * i - 1] = smaller_set[i];
    }
    return ret;
}

std::vector<double>
optim_iterative_parameters_set(int n)
{
    if ((n & (n - 1)) != 0) throw "Argument n must be power of 2";
    std::vector<double> ret(n + 1);
    auto theta = theta_set_construction(n);
    for (int i = 1; i <= n; ++i)
    {
        ret[i] = cos(M_PI * theta[i] / (n * 2));
    }
    return ret;
}

// Решение системы линейных уравнений методом Чебышева
std::vector<double> chebyshevIteration(const std::vector<std::vector<double>>& A,
                                       const std::vector<double>& F,
                                       std::vector<float> &statX,
                                       std::vector<float> &statY,
                                       int maxIterations, double tolerance=1e-4)
{
    if (A.size() != A[0].size()) throw "Matrix should be n*n!\n";
    if ((maxIterations & (maxIterations - 1)) != 0) throw "maxIterations argument should be power of 2";
    statX.resize(maxIterations), statY.resize(maxIterations);
    int n = A.size();
    std::vector<double> x(n, 0.0);
    std::vector<double> xPrev(n, 0.0);

    // Находим максимальное и минимальное собственные значения матрицы A
    double lambdaMax = 0.0;
    double lambdaMin = 0.0;
    for (int i = 0; i < n; ++i) {
        double sum = 0.0;
        for (int j = 0; j < n; ++j) {
            sum += std::abs(A[i][j]);
        }
        if (sum > lambdaMax) lambdaMax = sum;
        if (sum < lambdaMin) lambdaMin = sum;
    }
    // lambdaMax = 84.0, lambdaMax = 48.0;
    double tau0 = 2.0 / (lambdaMax + lambdaMin);
    double ro = (lambdaMax - lambdaMin) / (lambdaMax + lambdaMin);

    std::vector<double> tau_parameters = optim_iterative_parameters_set(maxIterations);
    for (int k = 0; k < maxIterations; ++k) {
        double tau = tau0 / (1 - tau_parameters[k + 1] * ro);
        // double tau = 2.0 / (lambdaMax + lambdaMin);
        for (int i = 0; i < n; ++i) {
            double sum = 0.0;
            for (int j = 0; j < n; ++j) {
                sum += A[i][j] * xPrev[j];
            }
            x[i] = xPrev[i] + tau * (F[i] - sum);
        }

        // Проверка на сходимость
        double normResidual = 0.0;
        for (int i = 0; i < n; ++i) {
            double sum = 0.0;
            for (int j = 0; j < n; ++j) {
                sum += A[i][j] * x[j];
            }
            normResidual += std::pow(F[i] - sum, 2);
        }
        normResidual = std::sqrt(normResidual);

        if (normResidual < tolerance) {
            std::cout << "Converged in " << k + 1 << " iterations." << std::endl;
            return x;
        }
        statX[k] = k;
        statY[k] = normResidual;
        xPrev = x;
    }

    std::cout << "Did not converge within the specified number of iterations." << std::endl;
    return x;
}

double
norm2(const std::vector<double> &v1)
{
    double ans = 0.0;
    for (const auto &item : v1)
    {
        ans += item * item;
    }
    return sqrt(ans);
}

int main_main() {
    // Задайте матрицу A и вектор F
    std::vector<std::vector<double>> A = {
        {25, 16, 11, 9, 9},
        {16, 24, 8, 9, 17},
        {11, 8, 19, 16, 11},
        {9, 9, 16, 15, 16},
        {9, 17, 11, 16, 21},
    };
    // std::vector<std::vector<double>> A = {
    //     {6, 0, 1, 1, 0},
    //     {0, 5, 0, 1, 0},
    //     {1, 0, 6, 1, 0},
    //     {1, 1, 1, 6, 1},
    //     {0, 0, 0, 1, 5},
    // };
    // std::string filename = "../SLAU_var_2.csv";
    // std::vector<std::vector<double>> A = read_csv(filename);
    std::vector<double> x = generate_random_vect(A.size());
    std::vector<double> F;
    try 
    {
        F = A * x;
    }
    catch (const char* str)
    {
        std::cerr << std::string(str) << std::endl;
    }

    // LU-разложение
    std::vector<std::vector<double>> L;
    std::vector<std::vector<double>> U;
    std::vector<std::vector<double>> tmp_A = A;
    LU_decomposition(tmp_A, L, U);

    std::vector<double> x_computed = solve_system(L, U, F);
    std::cout << "Погрешность решения прямым методом по второй норме: " << norm2(x_computed - x) << std::endl;

    // Задайте параметры метода
    int maxIterations = 4096;
    //64 128 256 512 1024 2048 4096 8192 16384 32768 65536 131072 262144 524288 1048576 2097152
    double tolerance = norm2(x_computed - x);

    // Решение системы линейных уравнений методом Чебышева
    std::vector<float> statX, statY;
    std::vector<double> solution = chebyshevIteration(A, F, statX, statY, maxIterations, tolerance);

    // Построение графика
    int num_of_dots_for_plot = 100;
    int step = statX.size() / num_of_dots_for_plot;
    std::vector<float> data(num_of_dots_for_plot * 2);
    float quantile = 0.9;
    float normX = maxIterations;
    std::vector<float> sorted_y;
    std::copy(statY.begin(), statY.end(), std::back_inserter(sorted_y));
    std::sort(sorted_y.begin(), sorted_y.end());
    float normY = sorted_y[static_cast<int>(quantile * (sorted_y.size() - 1))];
    normY = *std::max_element(statY.begin(), statY.end());
    for (int i = 0; i < num_of_dots_for_plot; ++i)
    {
        data[2 * i] = statX[i * step] / normX;
        data[2 * i + 1] = statY[i * step + 1] / normY;
    }
    draw_plot(data, 1.9f, 1.8f, -0.9f, -0.9f);

    // std::cout << "Решение прямым методом: " << x_computed << std::endl;
    // std::cout << "Решение методом Чебышева: " << solution << std::endl;
    
    std::cout << "Погрешность решения методом Чебышева по второй норме: " << norm2(solution - x) << std::endl;

    return 0;
}

int main1()
{
    try{
        std::cout << (theta_set_construction(8) == std::vector<int>{0, 1, 15, 7, 9, 3, 13, 5, 11}) << std::endl;
        std::cout << (theta_set_construction(16) == std::vector<int>{0, 1, 31, 15, 17, 7, 25, 9, 23, 3, 29, 13, 19, 5, 27, 11, 21}) << std::endl;
        std::cout << (theta_set_construction(32) == std::vector<int>{0, 1, 63, 31, 33, 15, 49, 17, 47, 7, 57,
        25, 39, 9, 55, 23, 41, 3, 61, 29, 35, 13, 51, 19, 45, 5, 59, 27, 37, 11, 53, 21, 43}) << std::endl;
    }
    catch (const char *str) {
        std::cerr << std::string(str) << std::endl;
    }
    return 0;
}

int main()
{
    main_main();
    // main1();
    // test_draw();
    return 0;
}