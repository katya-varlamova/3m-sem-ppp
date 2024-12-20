#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

void printMatrix(double **matrix, int size, FILE *file) {
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            fprintf(file, "%.4f ", matrix[i][j]);
        }
        fprintf(file, "\n");
    }
}

double** createMatrix(int size) {
    double **matrix = (double **)malloc(size * sizeof(double *));
    for (int i = 0; i < size; i++) {
        matrix[i] = (double *)malloc(size * sizeof(double));
    }
    return matrix;
}

void freeMatrix(double **matrix, int size) {
    for (int i = 0; i < size; i++) {
        free(matrix[i]);
    }
    free(matrix);
}

int invertMatrix(double **matrix, double **inverse, int size) {
    // шаг 1. Создаем единичную матрицу
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            inverse[i][j] = (i == j) ? 1.0 : 0.0;
        }
    }

    for (int i = 0; i < size; i++) {
        // Поиск максимального элемента в столбце
        double maxEl = fabs(matrix[i][i]);
        int maxRow = i;
        for (int k = i + 1; k < size; k++) {
            if (fabs(matrix[k][i]) > maxEl) {
                maxEl = fabs(matrix[k][i]);
                maxRow = k;
            }
        }

        // шаг 2. Меняем местами текущую строку и строку с максимальным элементом
        for (int k = i; k < size; k++) {
            double tmp = matrix[maxRow][k];
            matrix[maxRow][k] = matrix[i][k];
            matrix[i][k] = tmp;
        }
        for (int k = 0; k < size; k++) {
            double tmp = inverse[maxRow][k];
            inverse[maxRow][k] = inverse[i][k];
            inverse[i][k] = tmp;
        }

        // шаг 3. Нормализация текущей строки
        double diagEl = matrix[i][i];
        if (fabs(diagEl) < 1e-10) {
            return 0; // Матрица вырождена
        }
        for (int k = 0; k < size; k++) {
            matrix[i][k] /= diagEl;
            inverse[i][k] /= diagEl;
        }

        // шаг 4. Обнуление остальных строк
        for (int j = 0; j < size; j++) {
            if (j != i) {
                double factor = matrix[j][i];
                for (int k = 0; k < size; k++) {
                    matrix[j][k] -= factor * matrix[i][k];
                    inverse[j][k] -= factor * inverse[i][k];
                }
            }
        }
    }
    return 1;
}
void test1() {
    int size = 3;
    double **matrix = createMatrix(size);
    matrix[0][0] = 1;
    matrix[0][1] = 2;
    matrix[0][2] = 4;
    matrix[1][0] = 6;
    matrix[1][1] = 5;
    matrix[1][2] = 8;
    matrix[2][0] = 2;
    matrix[2][1] = 1;
    matrix[2][2] = 2;

    // for (int i = 0; i < size; i++) {
    //     for (int j = 0; j < size; j++) {
    //         matrix[i][j] = rand() % 10 + 1;
    //     }
    // }

    double **inverse = createMatrix(size);
    printMatrix(matrix, size, stdout);
    invertMatrix(matrix, inverse, size);
    
    printf("------\n");
    printMatrix(inverse, size, stdout);
}
void test2() {
    int size = 5;
    double **matrix = createMatrix(size);
    matrix[0][0] = 1;
    matrix[0][1] = 2;
    matrix[0][2] = 4;
    matrix[0][3] = 5;
    matrix[0][4] = 7;

    matrix[1][0] = 6;
    matrix[1][1] = 5;
    matrix[1][2] = 8;
    matrix[1][3] = 8;
    matrix[1][4] = 1;

    matrix[2][0] = 2;
    matrix[2][1] = 1;
    matrix[2][2] = 2;
    matrix[2][3] = 3;
    matrix[2][4] = 10;

    matrix[3][0] = 3;
    matrix[3][1] = 5;
    matrix[3][2] = 2;
    matrix[3][3] = 1;
    matrix[3][4] = 8;

    matrix[4][0] = 9;
    matrix[4][1] = 0;
    matrix[4][2] = 1;
    matrix[4][3] = 3;
    matrix[4][4] = 5;
    // for (int i = 0; i < size; i++) {
    //     for (int j = 0; j < size; j++) {
    //         matrix[i][j] = rand() % 10 + 1;
    //     }
    // }

    double **inverse = createMatrix(size);
    printMatrix(matrix, size, stdout);
    invertMatrix(matrix, inverse, size);
    
    printf("------\n");
    printMatrix(inverse, size, stdout);
}
int research() {
    FILE *file;
    #ifdef CSV
        file = fopen("inverse_matrix.csv", "w");
    #else
        file = fopen("inverse_matrix.txt", "w");
    #endif
    if (!file) {
        perror("Unable to open file");
        return EXIT_FAILURE;
    }
    #ifdef CSV
        fprintf(file, "core,size,time\n");
    #endif
    int sizes[] = {20, 50, 100, 500, 1000};
    for (int size = 50; size < 1050; size += 50) {
        
        double **matrix = createMatrix(size);
        for (int i = 0; i < size; i++) {
            for (int j = 0; j < size; j++) {
                matrix[i][j] = rand() % 10 + 1;
            }
        }

        double **inverse = createMatrix(size);

        clock_t start = clock();
        invertMatrix(matrix, inverse, size);
        clock_t end = clock();


        double time_spent = (double)(end - start) / CLOCKS_PER_SEC;
        
        #ifdef CSV
            fprintf(file, "1,%d,%.8f\n", size, time_spent);
        #else 
            fprintf(file, "Inverse of %dx%d matrix:\n", size, size);
            printMatrix(inverse, size, file);
            fprintf(file, "\n");
            fprintf(file, "Time taken to invert %dx%d matrix: %.4f seconds\n\n", size, size, time_spent);
        #endif
        freeMatrix(matrix, size);
        freeMatrix(inverse, size);
    }

    fclose(file);
    return EXIT_SUCCESS;
}
int main() {
    printf("test1:\n");
    test1();
    printf("test2:\n");
    test2();
    return 0;
}