#include <omp.h>
#include <stdio.h>
#include <stdlib.h>

#define CHUNK 100
#define NMAX 1000

// Раздел 1-3, базовые директивы
void basic_examples() {
    printf("1. BASIC PARALLEL REGION\n");

#pragma omp parallel
    {
        int tid = omp_get_thread_num();
        printf("Thread %d: Hello World\n", tid);

        if (tid == 0) {
            int nthreads = omp_get_num_threads();
            printf("Master thread: Total threads = %d\n", nthreads);
        }
    }
}

// Раздел 2: Private/shared переменные
void variable_scope() {
    printf("\n2. PRIVATE vs SHARED VARIABLES\n");

    int shared_var = 10;
    int private_var;

#pragma omp parallel shared(shared_var) private(private_var)
    {
        private_var = omp_get_thread_num();
        shared_var += 1;  // это вызовет состояние гонки!

#pragma omp critical
        {
            printf("Thread %d: private = %d, shared = %d\n",
                omp_get_thread_num(), private_var, shared_var);
        }
    }

    printf("Final shared_var = %d (data race!)\n", shared_var);
}

// Раздел 3: Директива for с schedule
void schedule_examples() {
    printf("\n3. FOR DIRECTIVE WITH SCHEDULE\n");

    float a[NMAX], b[NMAX], c[NMAX];
    int i;

    // Инициализация массивов
    for (i = 0; i < NMAX; i++) {
        a[i] = i * 1.0f;
        b[i] = i * 0.5f;  // изменил коэффициент для разнообразия
    }

    printf("\n3.1 Dynamic schedule (chunk = %d):\n", CHUNK);
#pragma omp parallel for schedule(dynamic, CHUNK)
    for (i = 0; i < NMAX; i++) {
        c[i] = a[i] + b[i];
        if (i < 10) {
            printf("  i = %d -> thread %d\n", i, omp_get_thread_num());
        }
    }

    // Static schedule
    printf("\n3.2 Static schedule (chunk = %d):\n", CHUNK);
#pragma omp parallel for schedule(static, CHUNK)
    for (i = 0; i < NMAX; i++) {
        c[i] = a[i] * b[i];  // теперь умножение вместо сложения
        if (i < 10) {
            printf("  i = %d -> thread %d\n", i, omp_get_thread_num());
        }
    }

    printf("\n3.3 Reduction example:\n");
    double sum = 0.0;
#pragma omp parallel for reduction(+:sum)
    for (i = 0; i < NMAX; i++) {
        sum += a[i];
    }
    printf("  Sum of array = %.2f\n", sum);

    printf("\nTASK: Compare static vs dynamic\n");
    printf("First 10 iterations distribution shown above.\n");
    printf("Conclusion: Static assigns blocks in advance,\n");
    printf("            Dynamic assigns as threads become free.\n");
}

// Раздел 4: Директива sections
void sections_example() {
    printf("\n4. PARALLEL SECTIONS\n");

    float a[NMAX], b[NMAX], c[NMAX];
    int i;

    // Инициализация 
    for (i = 0; i < NMAX; i++) {
        a[i] = i * 1.0f;
        b[i] = i * 2.0f;  // изменил коэффициент
    }

#pragma omp parallel shared(a, b, c) private(i)
    {
#pragma omp sections
        {
#pragma omp section
            {
                printf("Section 1 (thread %d): First half\n", omp_get_thread_num());
                for (i = 0; i < NMAX / 2; i++) {
                    c[i] = a[i] + b[i];
                }
            }

#pragma omp section
            {
                printf("Section 2 (thread %d): Second half\n", omp_get_thread_num());
                for (i = NMAX / 2; i < NMAX; i++) {
                    c[i] = a[i] + b[i];
                }
            }
        }
    }

    printf("Sections completed. First 5 results:\n");
    for (i = 0; i < 5; i++) {
        printf("  c[%d] = %.2f\n", i, c[i]);
    }

    printf("\nTASK: Split into 3 sections\n");
#pragma omp parallel shared(a, b, c) private(i)
    {
#pragma omp sections
        {
#pragma omp section
            {
                int tid = omp_get_thread_num();
                for (i = 0; i < NMAX / 3; i++) {
                    c[i] = a[i] - b[i];  // вычитание вместо сложения
                }
                printf("  Section 1 (thread %d): indices 0 - %d\n",
                    tid, NMAX / 3 - 1);
            }

#pragma omp section
            {
                int tid = omp_get_thread_num();
                for (i = NMAX / 3; i < 2 * NMAX / 3; i++) {
                    c[i] = a[i] - b[i];
                }
                printf("  Section 2 (thread %d): indices %d - %d\n",
                    tid, NMAX / 3, 2 * NMAX / 3 - 1);
            }

#pragma omp section
            {
                int tid = omp_get_thread_num();
                for (i = 2 * NMAX / 3; i < NMAX; i++) {
                    c[i] = a[i] - b[i];
                }
                printf("  Section 3 (thread %d): indices %d - %d\n",
                    tid, 2 * NMAX / 3, NMAX - 1);
            }
        }
    }
}

// Раздел 5: Директивы single и critical
void single_critical_example() {
    printf("\n5. SINGLE AND CRITICAL DIRECTIVES\n");

    int counter = 0;

#pragma omp parallel
    {
#pragma omp single
        {
            printf("Thread %d: This block executes only once\n",
                omp_get_thread_num());
        }

#pragma omp critical
        {
            counter++;
            printf("Thread %d: counter = %d\n",
                omp_get_thread_num(), counter);
        }

#pragma omp single nowait
        {
            printf("Thread %d: Final check (nowait used)\n",
                omp_get_thread_num());
        }

        printf("Thread %d: Continuing work...\n", omp_get_thread_num());
    }
}

int main() {
    printf("LAB WORK #1: OpenMP Parallel Regions\n");

    // Устанавливаем количество потоков
    omp_set_num_threads(4);
    printf("Using %d threads\n\n", omp_get_max_threads());

    basic_examples();          // Раздел 1: Базовые директивы
    variable_scope();          // Раздел 2: Private/shared
    schedule_examples();       // Раздел 3: Директива for
    sections_example();        // Раздел 4: Директива sections  
    single_critical_example(); // Раздел 5: single и critical

    printf("\nPress Enter to exit...");
    getchar();

    return 0;
}