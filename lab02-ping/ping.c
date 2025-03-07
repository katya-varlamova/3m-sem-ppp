#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

#define SIZE 1024
// int MPI_Send(void* message, int count,
//     MPI_Datatype datatype, int dest, int tag,
//     MPI_Comm comm)

// int MPI_Recv(void* message, int count,
//     MPI_Datatype datatype, int source, int tag,
//     MPI_Comm comm, MPI_Status* status)
int main(int argc, char **argv)
{
    int myrank, nprocs, len;
    char name[MPI_MAX_PROCESSOR_NAME];
    int *buf;
    MPI_Status st;

    buf = (int *)malloc(sizeof(int) * (SIZE * 1024 + 100));

    MPI_Init(&argc, &argv);

    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
    MPI_Comm_rank(MPI_COMM_WORLD, &myrank);
    MPI_Get_processor_name(name, &len);

    printf("Hello from processor %s[%d] %d of %d\n", name, len, myrank, nprocs);

    if (myrank % 2 == 0)
    {
        if (myrank < nprocs - 1)
        {
            int i, cl, sz = SIZE;
            double time;

            for (i = 0; i < SIZE * 1024; i++)
                buf[i] = i + 10;

            for (; i < SIZE * 1024 + 100; i++)
                buf[i] = 0;

            for (cl = 0; cl < 11; cl++)
            {
                time = MPI_Wtime();
                for (i = 0; i < 100; i++)
                {
                    MPI_Send(buf, sz, MPI_INT, myrank + 1, 10, MPI_COMM_WORLD);
                    MPI_Recv(buf, sz + 100, MPI_INT, myrank + 1, 20, MPI_COMM_WORLD, &st);
                }
                time = MPI_Wtime() - time;
                printf("[%d] Time = %lf  Data=%ld KByte\n",
                       myrank,
                       time,
                       sz * sizeof(int) / 1024);
                printf("[%d] Bandwith[%d] = %lf MByte/sek\n",
                       myrank,
                       cl,
                       sz * sizeof(int) / (time * 1024 * 1024));
                sz *= 2;
            }
        }
        else
            printf("[%d] Idle\n", myrank);
    }
    else
    {
        int i, cl, sz = SIZE;
        for (cl = 0; cl < 11; cl++)
        {
            for (i = 0; i < 100; i++)
            {
                MPI_Recv(buf, sz + 100, MPI_INT, myrank - 1, 10, MPI_COMM_WORLD, &st);
                MPI_Send(buf, sz, MPI_INT, myrank - 1, 20, MPI_COMM_WORLD);
            }
            sz *= 2;
        }
    }
    MPI_Finalize();
    return 0;
}
