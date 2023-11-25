#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <ft2build.h>
#include <freetype/freetype.h>


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

// // Шейдер для текста
// const char* textVertexShaderSource = R"(
//     #version 330 core
//     layout (location = 0) in vec4 vertex; // входные координаты вершин
//     out vec2 TexCoords;

//     uniform mat4 projection; // матрица проекции

//     void main()
//     {
//         gl_Position = projection * vec4(vertex.xy, 0.0, 1.0);
//         TexCoords = vertex.zw;
//     }
// )";


// Вершинный шейдер для текста
const char* textVertexShaderSource = R"(
    #version 330 core
    layout (location = 0) in vec4 vertex; // входные координаты вершин
    out vec2 TexCoords;

    uniform vec2 position;   // позиция начала текста
    uniform float scale;     // масштаб текста

    void main()
    {
        // Применяем сначала масштаб и трансляцию, а затем проекцию
        gl_Position = vec4((vertex.xy * scale) + position, 0.0, 1.0);
        TexCoords = vertex.zw;
    }
)";

const char* textFragmentShaderSource = R"(
    #version 330 core
    in vec2 TexCoords;
    out vec4 FragColor;

    uniform sampler2D text; // текстурный атлас
    uniform vec3 textColor; // цвет текста

    void main()
    {
        vec4 sampled = vec4(1.0, 1.0, 1.0, texture(text, TexCoords).r);
        FragColor = vec4(textColor, 1.0) * sampled;
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

std::map<FT_ULong, std::array<int, 4>>
createTextureAtlas(FT_Face& face, GLuint& texture, GLuint& vao, GLuint& vbo) {
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    int atlasWidth = 4096;
    int atlasHeight = 4096;
    std::vector<unsigned char> atlasData(atlasWidth * atlasHeight * 4, 0);  // 4 компонента цвета (RGBA) на пиксель
    // Текущие координаты в атласе
    int atlasX = 0;
    int atlasY = 0;

    // Промежуток между символами в атласе
    int paddingX = 1;
    // Координаты каждого символа в атласе
    std::map<FT_ULong, std::array<int, 4>> characterAtlasCoords;
    // Набор символов, которые вы хотите включить в атлас (ASCII)
    std::string characters = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";    
    for (auto& currentChar : characters) {
        FT_Load_Char(face, currentChar, FT_LOAD_RENDER);

        // Копирование данных символа в текстурный атлас
        for (int y = 0; y < face->glyph->bitmap.rows; ++y) {
            for (int x = 0; x < face->glyph->bitmap.width; ++x) {
                int destX = atlasX + x;
                int destY = atlasY + y;
                // Пересчитываем индекс в буфере символа
                int srcIndex = x + y * face->glyph->bitmap.width;
                // Пересчитываем индекс в буфере текстуры
                int destIndex = (destX + destY * atlasWidth) * 4;
                // Копируем компоненту R из буфера символа в текстурный атлас
                atlasData[destIndex] = face->glyph->bitmap.buffer[srcIndex];
                // Копируем компоненту G из буфера символа в текстурный атлас
                atlasData[destIndex + 1] = face->glyph->bitmap.buffer[srcIndex];
                // Копируем компоненту B из буфера символа в текстурный атлас
                atlasData[destIndex + 2] = face->glyph->bitmap.buffer[srcIndex];
                // Копируем компоненту A из буфера символа в текстурный атлас
                atlasData[destIndex + 3] = face->glyph->bitmap.buffer[srcIndex];
            }
        }

        // Запомните координаты символа в атласе
        characterAtlasCoords[currentChar] = {atlasX, atlasY, atlasX + static_cast<int>(face->glyph->bitmap.width), atlasY + static_cast<int>(face->glyph->bitmap.rows)};

        // Обновите координаты для следующего символа в атласе
        atlasX += face->glyph->bitmap.width + paddingX;
    }
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, atlasWidth, atlasHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, atlasData.data());
    return characterAtlasCoords;
}

std::map<FT_ULong, std::array<int, 4>>
loadFont(const char* fontPath, unsigned int fontSize, FT_Library& library, FT_Face& face, GLuint& texture, GLuint& vao, GLuint& vbo) {
    // Инициализация FreeType
    if (FT_Init_FreeType(&library)) {
        throw "Failed to initialize FreeType";
    }

    // Загрузка шрифта
    if (FT_New_Face(library, fontPath, 0, &face)) {
        throw "Failed to load font";
    }

    // Установка размера шрифта
    FT_Set_Pixel_Sizes(face, 0, fontSize);

    // Создание текстурного атласа
    auto atlas = createTextureAtlas(face, texture, vao, vbo);
    return atlas;
}

void renderText(std::map<FT_ULong, std::array<int, 4>> &characterAtlasCoords, FT_Face& face, GLuint &VAO, GLuint &VBO, GLuint &textShader, GLuint &texture, const char* text, float x, float y, float scale) {
    // Активация шейдера для текста
    glUseProgram(textShader);

    // Установка позиции начала текста
    glUniform2f(glGetUniformLocation(textShader, "position"), x, y);
    if (glGetError() != GL_NO_ERROR) throw "error in render text";

    // Установка масштаба текста
    glUniform1f(glGetUniformLocation(textShader, "scale"), scale);
    if (glGetError() != GL_NO_ERROR) throw "error in render text";

     // Активация цвета текста (белый в данном случае)
    glUniform3f(glGetUniformLocation(textShader, "textColor"), 1.0f, 1.0f, 1.0f);

    // Активация VAO и VBO
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    // Активация текстурного атласа
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);

    // Установка значения uniform переменной "text"
    glUniform1i(glGetUniformLocation(textShader, "text"), 0);

    // Итерация по каждому символу в тексте
    for (const char* c = text; *c; ++c) {
        auto it = characterAtlasCoords.find(*c);
        if (it == characterAtlasCoords.end()) {
            // Символ не найден в атласе, пропускаем
            continue;
        }
        std::array<int, 4> coords = it->second;

        // Обновление данных VBO для текущего глифа
        GLfloat vertices[] = {
            // Первый треугольник
            x + coords[0] * scale, y + coords[3] * scale,
            x + coords[2] * scale, y + coords[3] * scale,
            x + coords[2] * scale, y + coords[1] * scale,
            // Второй треугольник
            x + coords[0] * scale, y + coords[3] * scale,
            x + coords[2] * scale, y + coords[1] * scale,
            x + coords[0] * scale, y + coords[1] * scale
        };
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);

        // Рендеринг текущего глифа
        glDrawArrays(GL_TRIANGLES, 0, 6);

        // Перемещение позиции для следующего глифа
        x += (coords[2] - coords[0]) * scale;
    }

    // Отключение VAO и VBO
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // Отключение шейдера текста
    glUseProgram(0);
}

int main1() {
    FT_Library library;
    if (FT_Init_FreeType(&library)) {
        return -1;
    }
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


    // Текстовый шейдер
    // Компиляция и линковка текстового шейдера
    textVertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(textVertexShader, 1, &textVertexShaderSource, NULL);
    glCompileShader(textVertexShader);
    glGetShaderiv(textVertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        GLint logLength;
        glGetShaderiv(vertexShader, GL_INFO_LOG_LENGTH, &logLength);
        std::vector<GLchar> infoLog(logLength);
        glGetShaderInfoLog(vertexShader, logLength, nullptr, infoLog.data());

        std::cerr << "Error compiling vertex shader:\n" << infoLog.data() << std::endl;
        // Возможно, следует добавить код для корректной обработки ошибки
    }
    textFragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(textFragmentShader, 1, &textFragmentShaderSource, NULL);
    glCompileShader(textFragmentShader);
    glGetShaderiv(textFragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        GLint logLength;
        glGetShaderiv(fragmentShader, GL_INFO_LOG_LENGTH, &logLength);
        std::vector<GLchar> infoLog(logLength);
        glGetShaderInfoLog(fragmentShader, logLength, nullptr, infoLog.data());

        std::cerr << "Error compiling fragment shader:\n" << infoLog.data() << std::endl;
        // Возможно, следует добавить код для корректной обработки ошибки
    }

    textShaderProgram = glCreateProgram();
    glAttachShader(textShaderProgram, textVertexShader);
    glAttachShader(textShaderProgram, textFragmentShader);
    success = 0;
    glLinkProgram(textShaderProgram);
    glGetProgramiv(textShaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        GLint logLength;
        glGetProgramiv(textShaderProgram, GL_INFO_LOG_LENGTH, &logLength);
        std::vector<GLchar> infoLog(logLength);
        glGetProgramInfoLog(textShaderProgram, logLength, nullptr, infoLog.data());
        
        std::cerr << "Error linking shader program:\n" << infoLog.data() << std::endl;
        // Возможно, следует добавить код для корректной обработки ошибки
    }
    glDeleteShader(textVertexShader);
    glDeleteShader(textFragmentShader);


    int num_of_dots = 100;
    double left_border = -4, right_border = 4;

    // создаем координатную сетку на ось Ox
    std::vector<GLfloat> coords(num_of_dots);
    linspace(coords, left_border, right_border, num_of_dots);

    // Добавляем в массив значения вычисленной на данной сетке функции
    coords = calculate_values(coords, tan);

    const int num_axis_coords = 8;
    std::vector<GLfloat> axis_vertices = {
        -1.0f * (GLfloat)fabs(left_border), 0.0f,
        1.0f * (GLfloat)fabs(right_border), 0.0f,
        0.0f, -1.0f,
        0.0f, 1.0f
    };
    // Добавляем координаты для отрисовки координатных осей 
    coords.insert(coords.end(), axis_vertices.begin(), axis_vertices.end());

    // Буфер вершин
    GLuint VBO, VAO;
    glGenBuffers(1, &VBO);
    glGenVertexArrays(1, &VAO);

    // Инициализация FreeType
    FT_Library library;
    FT_Face face;
    GLuint texture;
    std::map<FT_ULong, std::array<int, 4>> atlas = loadFont("/usr/share/fonts/truetype/freefont/FreeMono.ttf", 48, library, face, texture, VAO, VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, coords.size() * sizeof(GLfloat), coords.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);


    // Создание буферов и массива атрибутов для текста
    GLuint textVBO, textVAO;
    glGenVertexArrays(1, &textVAO);
    glGenBuffers(1, &textVBO);

    // Инициализация буфера текста
    glBindVertexArray(textVAO);
    glBindBuffer(GL_ARRAY_BUFFER, textVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 6 * 4, nullptr, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), 0);
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

        // // Использование шейдерной программы
        // glUseProgram(shaderProgram);
        // // масштабирование
        // GLuint modelLoc = glGetUniformLocation(shaderProgram, "model");
        // float scale_matrix[16] = {0};
        // float scaleX = 0.2f, scaleY = 0.8f;
        // create_scale_matrix(scale_matrix, 4, scaleX, scaleY);
        // glUniformMatrix4fv(modelLoc, 1, GL_FALSE, scale_matrix);
        // if ((success = glGetError()) != 0) std::cerr << success << ":0" << std::endl;

        // // Отрисовка графика
        // glBindVertexArray(VAO);
        // glDrawArrays(GL_LINE_STRIP, 0, num_of_dots); // Отрисовка графика
        // glDrawArrays(GL_LINES, num_of_dots, 4); // Отрисовка осей
        // // glPointSize(10.0f);
        // // glDrawArrays(GL_POINTS, num_of_dots + 4, labels.size() / 2); // Отрисовка подписей
        // // glPointSize(1.0f);
        // glBindVertexArray(0);
        
        // Использование шейдера для текста
        glUseProgram(textShaderProgram);

        // Отрисовка текста
        renderText(atlas, face, textVAO, textVBO, textShaderProgram, texture, "Hello, world!", 0.0f, 0.0f, 1.0f);

        // Возвращение к шейдеру для графика
        glUseProgram(shaderProgram);


        // Показать результаты отрисовки
        glfwSwapBuffers(window);
    }

    // Освобождение ресурсов
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(shaderProgram);
    FT_Done_Face(face);
    FT_Done_FreeType(library);

    // Завершение работы GLFW
    glfwTerminate();

    return 0;
}