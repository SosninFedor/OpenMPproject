// Подключаем библиотеку OpenMP для параллельного программирования
#include <omp.h>
// Подключаем стандартную библиотеку ввода-вывода (printf, scanf, fopen)
#include <cstdio>
// Подключаем математическую библиотеку (здесь не используется, но оставлено для совместимости)
#include <cmath>
// Подключаем Windows API для работы с кодировкой консоли
#include <windows.h>

/**
 * Функция инициализации матрицы и векторов
 * double* &pMatrix - ссылка на указатель матрицы (чтобы функция могла изменить сам указатель)
 * double* &pVector  - ссылка на указатель вектора
 * double* &pResult  - ссылка на указатель результата
 * int &Size         - ссылка на размер матрицы
 */
void ProcessInit(double*& pMatrix, double*& pVector, double*& pResult, int& Size) {
    // Выводим приглашение для ввода размера
    printf("Введите размер матрицы: ");
    // Считываем размер с клавиатуры
    scanf("%d", &Size);

    // Выделяем память под матрицу Size x Size (Size*Size элементов типа double)
    pMatrix = new double[Size * Size];
    // Выделяем память под вектор длины Size
    pVector = new double[Size];
    // Выделяем память под вектор результата длины Size
    pResult = new double[Size];

    // Инициализация единичной матрицей (все элементы = 0, на главной диагонали = 1)
    // Проходим по всем элементам матрицы (их Size*Size)
    for (int i = 0; i < Size * Size; i++)
        pMatrix[i] = 0.0;  // Обнуляем каждый элемент

    // Заполняем главную диагональ единицами, вектор b единицами, результат нулями
    for (int i = 0; i < Size; i++) {
        // Элемент на главной диагонали: строка i, столбец i
        pMatrix[i * Size + i] = 1.0;
        // Все элементы вектора заполняем 1.0
        pVector[i] = 1.0;
        // Обнуляем результат (на всякий случай)
        pResult[i] = 0.0;
    }

    // Выводим подтверждение с размером
    printf("Размер: %d\n", Size);
}

/**
 * Последовательное умножение матрицы на вектор
 * double* pMatrix - указатель на матрицу
 * double* pVector - указатель на вектор
 * double* pResult - указатель на результат
 * int Size       - размер матрицы/вектора
 */
void SerialProduct(double* pMatrix, double* pVector, double* pResult, int Size) {
    // Засекаем время начала с помощью OpenMP (в секундах)
    double start = omp_get_wtime();

    // Внешний цикл по строкам матрицы (i - номер строки)
    for (int i = 0; i < Size; i++) {
        // Обнуляем i-й элемент результата
        pResult[i] = 0.0;
        // Внутренний цикл по столбцам (j - номер столбца)
        for (int j = 0; j < Size; j++) {
            // Умножаем элемент матрицы A[i][j] на элемент вектора b[j]
            // и добавляем к результату: c[i] = c[i] + A[i][j] * b[j]
            pResult[i] += pMatrix[i * Size + j] * pVector[j];
        }
    }

    // Засекаем время конца
    double end = omp_get_wtime();

    // Открываем файл для записи результатов (режим "a" - дозапись в конец)
    FILE* file = fopen("results.txt", "a");
    // Проверяем, удалось ли открыть файл
    if (file) {
        // Записываем размер и время выполнения в файл
        fprintf(file, "Size=%d, Serial=%.6f сек\n", Size, end - start);
        // Закрываем файл
        fclose(file);
    }
    // Выводим время на экран
    printf("Последовательное: время = %.6f сек\n", end - start);
}

/**
 * Параллельное умножение матрицы на вектор (разложение по строкам)
 * double* pMatrix - указатель на матрицу
 * double* pVector - указатель на вектор
 * double* pResult - указатель на результат
 * int Size       - размер матрицы/вектора
 */
void ParallelProduct(double* pMatrix, double* pVector, double* pResult, int Size) {
    // Засекаем время начала
    double start = omp_get_wtime();
    // Объявляем переменные для циклов
    int i, j;

    // ДИРЕКТИВА OPENMP: параллельное выполнение цикла for
    // private(j) - переменная j будет своей для каждого потока (чтобы не мешали друг другу)
#pragma omp parallel for private (j) 
// Цикл по строкам - распределяется между потоками
    for (i = 0; i < Size; i++) {
        // Обнуляем результат для текущей строки
        pResult[i] = 0.0;
        // Внутренний цикл по столбцам (каждый поток считает свои строки полностью)
        for (j = 0; j < Size; j++) {
            // Умножение и сложение
            pResult[i] += pMatrix[i * Size + j] * pVector[j];
        }
    }

    // Засекаем время конца
    double end = omp_get_wtime();

    // Открываем файл для дозаписи
    FILE* file = fopen("results.txt", "a");
    if (file) {
        fprintf(file, "Size=%d, Parallel=%.6f сек\n", Size, end - start);
        fclose(file);
    }
    printf("Параллельное: время = %.6f сек\n", end - start);
}

/**
 * Функция завершения - освобождение памяти
 * double* pMatrix - указатель на матрицу
 * double* pVector - указатель на вектор
 * double* pResult - указатель на результат
 * int Size       - размер (не используется, но оставлен для совместимости)
 */
void ProcessTerminate(double* pMatrix, double* pVector, double* pResult, int Size) {
    // Освобождаем память, выделенную через new[]
    delete[] pMatrix;
    delete[] pVector;
    delete[] pResult;
}

/**
 * Главная функция программы
 */
int main() {
    // Устанавливаем кодировку консоли для правильного отображения русских букв
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    // Объявляем указатели на матрицу и векторы
    double* pMatrix;
    double* pVector;
    double* pResult;
    // Переменная для хранения размера
    int Size;

    // Вызываем функцию инициализации
    ProcessInit(pMatrix, pVector, pResult, Size);
    // Вызываем последовательное умножение
    SerialProduct(pMatrix, pVector, pResult, Size);
    // Вызываем параллельное умножение
    ParallelProduct(pMatrix, pVector, pResult, Size);
    // Освобождаем память
    ProcessTerminate(pMatrix, pVector, pResult, Size);

    // Ждём нажатия любой клавиши перед закрытием консоли
    system("pause");
    // Возвращаем 0 - программа завершена успешно
    return 0;
}