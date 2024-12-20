#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <omp.h>
#define EPS 1e-9

void printMatrix(double *matrix, int size, FILE *file) {
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            fprintf(file, "%.4f ", matrix[j * size + i]);
        }
        fprintf(file, "\n");
    }
}

double* createMatrix(int size) {
    double *matrix = (double *)calloc(size, sizeof(double));
    return matrix;
}

void freeMatrix(double *matrix) {
    free(matrix);
}

void fill_test_5(double *matrix, int N) {
    matrix[0] = 1;
    matrix[1] = 2;
    matrix[2] = 4;
    matrix[3] = 5;
    matrix[4] = 7;

    matrix[N + 0] = 6;
    matrix[N + 1] = 5;
    matrix[N + 2] = 8;
    matrix[N + 3] = 8;
    matrix[N + 4] = 1;

    matrix[2 * N + 0] = 2;
    matrix[2 * N + 1] = 1;
    matrix[2 * N + 2] = 2;
    matrix[2 * N + 3] = 3;
    matrix[2 * N + 4] = 10;

    matrix[3 * N + 0] = 3;
    matrix[3 * N + 1] = 5;
    matrix[3 * N + 2] = 2;
    matrix[3 * N + 3] = 1;
    matrix[3 * N + 4] = 8;

    matrix[4 * N + 0] = 9;
    matrix[4 * N + 1] = 0;
    matrix[4 * N + 2] = 1;
    matrix[4 * N + 3] = 3;
    matrix[4 * N + 4] = 5;
    for (int i = 0; i < N; i++) {
        for (int j = i + 1; j < N ; j++) {
            //matrix[i][j] = i + j * 0.01; //(rand() % 10) + 1;
            int tmp = matrix[i * N + j];
            matrix[i * N + j] = matrix[j * N + i];
            matrix[j * N + i] = tmp;
        }
    }
}
// OMP,4,50,0.02474300
// OMP,4,100,0.09379900
// OMP,4,150,0.18044300
void generate_matrixes(double *matrix, double *augmented, int N) {
    srand(42);
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            matrix[i * N + j] = (rand() % 10) + 1;
            augmented[i * N + j] = (i == j) ? 1.0 : 0.0;
        }
    }
    // fill_test_5(matrix, N);
}
void find_aug_matrix(double *matrix_part, double *aug_part, int N, int size, int rank, MPI_Datatype mpi_row_shared) {
    double *shared_col_data = malloc(sizeof(double) * (N + 1));
    for (int k = 0; k < N; k++) {
        double max_val = -1.0;
        int max_row = -1;

        int cols_to_handle =  rank < (N % size) ? N / size + 1 : N / size;

        int cur_column = k / size;
        // Каждый процесс ищет максимум в текущем столбце
        if (rank == k % size) {
            for (int i = k; i < N; i++) {
                if (fabs(matrix_part[cur_column * N + i]) > max_val) {
                    max_val = fabs(matrix_part[cur_column * N + i]);
                    max_row = i;
                }
            }
            shared_col_data[N] = max_row;
            for (int i = 0; i < N; i++) {
                shared_col_data[i] = matrix_part[cur_column * N + i];
            }
        }

        // Бродкаст структуры shared_col
        MPI_Bcast(shared_col_data, 1, mpi_row_shared, k % size, MPI_COMM_WORLD);
        int maxRow = shared_col_data[N];


        // шаг 2. Меняем местами текущую строку и строку с максимальным элементом для всех столбцов на этом процессоре
        for (int col_to_handle = 0; col_to_handle < cols_to_handle; col_to_handle++) {
            double tmp = matrix_part[col_to_handle * N + maxRow];
            matrix_part[col_to_handle * N + maxRow] = matrix_part[col_to_handle * N + k];
            matrix_part[col_to_handle * N + k] = tmp;
            
            tmp = aug_part[col_to_handle * N + maxRow];
            aug_part[col_to_handle * N + maxRow] = aug_part[col_to_handle * N + k];
            aug_part[col_to_handle * N + k] = tmp;
        }
        double tmp = shared_col_data[maxRow];
        shared_col_data[maxRow] = shared_col_data[k];
        shared_col_data[k] = tmp;

        double pivot = shared_col_data[k];

        //шаг 3. Нормализация текущей строки
        if (fabs(pivot) < EPS) {  // Матрица вырождена
            printf("matrix is bad..\n");
            exit(1);
        }

        for (int col_to_handle = 0; col_to_handle < cols_to_handle; col_to_handle++) {
            matrix_part[col_to_handle * N + k] /= pivot;
            aug_part[col_to_handle * N + k] /= pivot;
        }

        
        #pragma omp parallel for schedule(dynamic)
        for (int col_to_handle = 0; col_to_handle < cols_to_handle; col_to_handle++) {
            double factor = matrix_part[col_to_handle * N + k];
            for (int row = 0; row < N; row++) {
                if (row != k) {
                    matrix_part[col_to_handle * N + row] -= shared_col_data[row] * factor;
                    aug_part[col_to_handle * N + row] -= shared_col_data[row] * aug_part[col_to_handle * N + k];
                }
            }
        }
    }
    free(shared_col_data);
}
void solve_inv_jordan_mpi(double **matrix_res, double *matrix, double *identity, int N, int size) {
    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    int max_matrix_rows = (N - 1) / size + 1;
    // Распределение столбцов матрицы по процессам
    double *matrix_part = calloc(sizeof(double), max_matrix_rows * N);
    double *aug_part = calloc(sizeof(double), max_matrix_rows * N);

    MPI_Datatype mpi_row;
    MPI_Datatype mpi_row_shift, mpi_row_shared;

    MPI_Type_contiguous(N, MPI_DOUBLE, &mpi_row);
    {
        MPI_Datatype mpi_row_tmp;
        MPI_Type_vector(max_matrix_rows, N, N * size, MPI_DOUBLE, &mpi_row_tmp);
        MPI_Type_create_resized(mpi_row_tmp, 0, sizeof(double) * N, &mpi_row_shift);
        MPI_Type_free(&mpi_row_tmp);
    }
    MPI_Type_commit(&mpi_row_shift);
    MPI_Type_commit(&mpi_row);

    MPI_Type_contiguous(N + 1, MPI_DOUBLE, &mpi_row_shared);
    MPI_Type_commit(&mpi_row_shared);

    MPI_Scatter(matrix,    1, mpi_row_shift, matrix_part, max_matrix_rows, mpi_row, 0, MPI_COMM_WORLD);
    MPI_Scatter(identity, 1, mpi_row_shift, aug_part,    max_matrix_rows, mpi_row, 0, MPI_COMM_WORLD);

    find_aug_matrix(matrix_part, aug_part, N, size, rank, mpi_row_shared);

    //MPI_Gather(aug_part, max_matrix_rows, mpi_row, *matrix_res, 1, mpi_row_shift, 0, MPI_COMM_WORLD);
    for (int i = 0; i < max_matrix_rows; i++) {
        MPI_Gather(aug_part + N * i, 1, mpi_row,
                *matrix_res + N * i * size, 1, mpi_row,
                0, MPI_COMM_WORLD);
    }
    free(matrix_part);
    free(aug_part);

    MPI_Type_free(&mpi_row);
    MPI_Type_free(&mpi_row_shift);
    MPI_Type_free(&mpi_row_shared);
}
void check(int N) {
    int rank, size;
    //int N = 70;
    omp_set_num_threads(4);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    int max_matrix_rows = (N - 1) / size + 1;
    double *matrix, *identity, *matrix_res;
    if (rank == 0) {
        matrix = createMatrix(N * max_matrix_rows * size);
        identity = createMatrix(N * max_matrix_rows * size);
        generate_matrixes(matrix, identity, N);
        printf("Исходная матрица:\n");
        printMatrix(matrix, N, stdout);
        matrix_res = createMatrix(N * max_matrix_rows * size);
    }

    solve_inv_jordan_mpi(&matrix_res, matrix, identity, N, size);

    if (rank == 0) {
        printf("\n");
        printf("Обратная: \n");
        printMatrix(matrix_res, N, stdout);
        printf("\n");

        double *check = createMatrix(N * max_matrix_rows * size);
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < N; j++) {
                for (int k = 0; k < N; k++) {
                    check[j * N + i] += matrix[k * N + i] * matrix_res[j * N + k];
                }
            }
        }
        
        printf("Результат умножения исходной матрицы на обратную (ожидается единичная матрица):\n");
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < N; j++) {
                if (fabs(check[j * N + i]) < EPS)
                    printf("0 ");
                else
                    printf("%1.0f ", check[j * N + i]);
            }
            printf("\n");
        }
        freeMatrix(check);
        freeMatrix(matrix);
        freeMatrix(matrix_res);
        freeMatrix(identity);
    }

    MPI_Finalize();
}
int research() {
    int rank, size;
    double start;
    FILE *file;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    if (rank == 0) {
        file = fopen("inverse_matrix.csv", "w");
        if (!file) {
            perror("Unable to open file");
            return EXIT_FAILURE;
        }
        //fprintf(file, "implementation,core,size,time\n");
    }
    int Ns[] = {100, 200, 300, 400, 500, 600, 700, 800, 900, 1000, 2000, 3000, 5000};
    int u = 13;
    for (int num_threads = 1; num_threads < 5; num_threads++) {
        for (int ni = 0; ni < u; ni += 1) {
            int N = Ns[ni];

            int max_matrix_rows = (N - 1) / size + 1;
            double *matrix, *identity, *matrix_res;
            omp_set_num_threads(num_threads);
            if (rank == 0) {
                matrix = createMatrix(N * max_matrix_rows * size);
                identity = createMatrix(N * max_matrix_rows * size);
                generate_matrixes(matrix, identity, N);
                matrix_res = createMatrix(N * max_matrix_rows * size);
                start = omp_get_wtime();
            }

            solve_inv_jordan_mpi(&matrix_res, matrix, identity, N, size);

            if (rank == 0) {
                double end = omp_get_wtime(); 
            
                double time_spent = end - start;
                
                fprintf(file, "OMP,%d,%d,%.8f,%d\n", size, N, time_spent, num_threads);

                freeMatrix(matrix);
                freeMatrix(matrix_res);
                freeMatrix(identity);
            }
        }
    }
    if (rank == 0) {
        fclose(file);
    }
    MPI_Finalize();
}
int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);
    check(25);
    //research();
    return 0;
}