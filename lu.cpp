#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include <string>
#include <cmath>
#include <chrono>

enum
{
    LEFT_BOUND = -1,
    RIGHT_BOUND = 1
};

// Функция для выполнения LU-разложения
void LU_decomposition(std::vector<std::vector<double>>& A, std::vector<std::vector<double>>& L, std::vector<std::vector<double>>& U)
{
    int n = A.size();
    L.resize(n, std::vector<double>(n, 0));
    U.resize(n, std::vector<double>(n, 0));
    for (int i = 0; i < n; i++) {
        L[i][i] = 1;
    }
    for (int k = 0; k < n; k++) {
        U[k][k] = A[k][k];   
        for (int i = k + 1; i < n; i++) {
            L[i][k] = A[i][k] / U[k][k];
            U[k][i] = A[k][i];
        }
        for (int i = k + 1; i < n; i++) {
            for (int j = k + 1; j < n; j++) {
                A[i][j] = A[i][j] - L[i][k] * U[k][j];
            }
        }
    }
}

// Функция для решения системы линейных уравнений
std::vector<double> solve_system(std::vector<std::vector<double>>& L, std::vector<std::vector<double>>& U, std::vector<double>& b)
{
    int n = L.size();
    std::vector<double> y(n, 0);
    for (int i = 0; i < n; i++) {
        double sum = 0;
        for (int j = 0; j < i; j++) {
            sum += L[i][j] * y[j];
        }
        y[i] = b[i] - sum;
    }
    std::vector<double> x(n, 0);
    for (int i = n - 1; i >= 0; i--) {
        double sum = 0;
        for (int j = i + 1; j < n; j++) {
            sum += U[i][j] * x[j];
        }
        x[i] = (y[i] - sum) / U[i][i];
    }
    return x;
}

// Функция для умножения матрицы на столбец
std::vector<double> matrix_vector_multiply(const std::vector<std::vector<double>>& matrix, const std::vector<double>& vector)
{
    int rows = matrix.size();
    int cols = matrix[0].size();
    std::vector<double> result(rows, 0);
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            result[i] += matrix[i][j] * vector[j];
        }
    }
    return result;
}

std::vector<std::vector<double>> read_csv(const std::string& filename)
{
    std::vector<std::vector<double>> matrix;
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error when try to open file" << std::endl;
        return matrix;
    }
    std::string line;
    while (std::getline(file, line)) {
        std::vector<double> row;
        std::stringstream ss(line);
        std::string cell;
        while (std::getline(ss, cell, ',')) {
            row.push_back(std::stod(cell));
        }
        matrix.push_back(row);
    }
    return matrix;
}

double max_norm(const std::vector<double> v)
{
    double max = fabs(v.at(0));
    for (auto &it : v) {
        max = (max > fabs(it)) ? max : fabs(it);
    }
    return max;
}

std::vector<double> operator-(const std::vector<double> &v1, const std::vector<double> &v2)
{
    if (v1.size() != v2.size()) throw "Incorrect sizes!!!\n";
    std::vector<double> ans(v2.size());
    for (int i = 0; i < v1.size(); ++i) {
        ans[i] = v1[i] - v2[i];
    }
    return ans;
}

std::vector<double> generate_random_vect(int s)
{
    std::vector<double> v(s);
    for (int i = 0; i < v.size(); ++i) {
        v[i] = double(rand()) * (RIGHT_BOUND - LEFT_BOUND) / RAND_MAX + LEFT_BOUND;
    }
    return v;
}

// int main()
// {
//     std::string filename = "./SLAU_var_2.csv";
//     std::vector<std::vector<double>> A = read_csv(filename);
    
//     // LU-разложение
//     std::vector<std::vector<double>> L;
//     std::vector<std::vector<double>> U;
//     std::vector<std::vector<double>> tmp_A = A;
//     LU_decomposition(tmp_A, L, U);

//     // генерация решения
//     std::vector<double> x = generate_random_vect(A.size());

//     // Вычисление правой части системы
//     std::vector<double> f = matrix_vector_multiply(A, x);

//     std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
//     // Решение системы линейных уравнений
//     std::vector<double> x_computed = solve_system(L, U, f);
//     std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();

//     std::cout << "||x_true - x_computed|| = " << max_norm(x - x_computed) << std::endl;
//     std::cout << "time in microseconds spent to find solution: " <<
//     std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() << std::endl;
    
//     return 0;
// }