#include <iostream>
#include <cmath>
#include <vector>
#include <map>
#include <algorithm>

// Импорт кода из первого задания(прямого метода)
#include "../lu.cpp"

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

template<typename T>
std::vector<T>
operator+(const std::vector<T> &v1, const std::vector<T> &v2)
{
    if (v1.size() != v2.size()) throw "Vectors sizes doesnt match";
    std::vector<T> ret(v1.size());
    for (int i = 0; i < v1.size(); ++i)
    {
        ret[i] = v1[i] + v2[i];
    }
    return ret;
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

// Функция для нахождения оценки собственных значений с помощью теоремы Гершгорина
std::vector<double>
eigenvalue_estimation(const std::vector<std::vector<double>> &A)
{
    double lambdaMax = 0.0;
    double lambdaMin = 0.0;
    for (int i = 0; i < A.size(); ++i)
    {
        double sum_abs_not_diag = 0.0;
        for (int j = 0; j < A[0].size(); ++j)
        {
            if (i != j) sum_abs_not_diag += std::fabs(A[i][j]);
        }
        if (i == 0) {
            lambdaMin = sum_abs_not_diag;
        } else if (lambdaMin > sum_abs_not_diag) lambdaMin = A[i][i] - sum_abs_not_diag;
        if (A[i][i] + sum_abs_not_diag > lambdaMax) lambdaMax = A[i][i] + sum_abs_not_diag;
    }
    return std::vector<double> {lambdaMin, lambdaMax};
}

// Решение системы линейных уравнений методом Чебышева
std::vector<double> chebyshevIteration(const std::vector<std::vector<double>>& A,
                                       const std::vector<double>& F,
                                       std::vector<float> &statX,
                                       std::vector<float> &statY,
                                       int maxIterations)
{
    if (A.size() != A[0].size()) throw "Matrix should be n*n!\n";
    if ((maxIterations & (maxIterations - 1)) != 0) throw "maxIterations argument should be power of 2";
    statX.resize(maxIterations), statY.resize(maxIterations);
    int n = A.size();
    std::vector<double> x(n, 0.0);
    std::vector<double> xPrev(n, 0.0);

    // Оценка для собственных значений с помощью теоремы Гершгорина
    std::vector<double> estim = eigenvalue_estimation(A);
    double lambdaMin = estim[0], lambdaMax = estim[1];

    double tau0 = 2.0 / (lambdaMax + lambdaMin);
    double ro = (lambdaMax - lambdaMin) / (lambdaMax + lambdaMin);

    std::vector<double> tau_parameters = optim_iterative_parameters_set(maxIterations);
    for (int k = 0; k < maxIterations; ++k) {
        double tau = tau0 / (1 - tau_parameters[k + 1] * ro);
        for (int i = 0; i < n; ++i) {
            double sum = 0.0;
            for (int j = 0; j < n; ++j) {
                sum += A[i][j] * xPrev[j];
            }
            x[i] = xPrev[i] + tau * (F[i] - sum);
        }

        statX[k] = k;
        statY[k] = norm2(F - A * x);
        xPrev = x;
    }

    return x;
}

int main() {
    std::string filename = "../SLAU_var_2.csv";
    std::vector<std::vector<double>> A = read_csv(filename);
    for (int i = 0; i < A.size(); i++) ++A[i][i];
    std::vector<double> x = generate_random_vect(A.size());
    std::vector<double> F;
    try { F = A * x; }
    catch (const char* str) { std::cerr << std::string(str) << std::endl; }

    // LU-разложение
    std::vector<std::vector<double>> L;
    std::vector<std::vector<double>> U;
    std::vector<std::vector<double>> tmp_A = A;
    LU_decomposition(tmp_A, L, U);

    std::vector<double> x_computed = solve_system(L, U, F);
    double direct_method_error = norm2(x_computed - x);

    // Метод Чебышева
    int pow_of_two = 0;
    std::vector<float> statX, statY;
    std::vector<double> solution(x.size(), 0);
    int maxIterations = 0;
    while (norm2(solution - x) >= direct_method_error)
    {
        ++pow_of_two;
        statX.clear(), statY.clear();
        maxIterations = pow(2, pow_of_two);
        solution = chebyshevIteration(A, F, statX, statY, maxIterations);
    }
    statX.shrink_to_fit(), statY.shrink_to_fit();

    std::cout << "Оценка спектра матрицы с помощью теоремы Гершгорина(минимальное, максимальное значения): " <<
    eigenvalue_estimation(A) << std::endl;
    std::cout << "Количество итераций метода Чебышева: " << maxIterations << std::endl;
    std::cout << "Погрешность решения прямым методом по второй норме: " << direct_method_error << std::endl;
    std::cout << "Погрешность решения методом Чебышева по второй норме: " << norm2(solution - x) << std::endl;
    std::cout << "Относительная погрешность решения методом Чебышева по второй норме: " << norm2(solution - x) / norm2(x) << std::endl;

    // Сохраним данные в csv файлы для отрисовки графика в Python
    std::ofstream fileX("statX.csv");
    for (const auto &value : statX)
    {
        fileX << value << ",";
    }
    std::ofstream fileY("statY.csv");
    for (const auto &value : statY)
    {
        fileY << value << ",";
    }

    return 0;
}