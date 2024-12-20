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
    MPI_Request re;

    buf = (int *)malloc(sizeof(int));
    int sz = 1;

    MPI_Init(&argc, &argv);

    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
    MPI_Comm_rank(MPI_COMM_WORLD, &myrank);
    MPI_Get_processor_name(name, &len);
    int recieved = 0;
    if (myrank == 0){
        *buf = myrank;
        MPI_Send(buf, sz, MPI_INT, (myrank + 1) % nprocs, 10, MPI_COMM_WORLD);
        MPI_Recv(buf, sz, MPI_INT, (myrank - 1) % nprocs, 10, MPI_COMM_WORLD, &st);

    } else {
        MPI_Recv(buf, sz, MPI_INT, (myrank - 1) % nprocs, 10, MPI_COMM_WORLD, &st);
        recieved = *buf;
        *buf = myrank;
        MPI_Send(buf, sz, MPI_INT, (myrank + 1) % nprocs, 10, MPI_COMM_WORLD);
    }
    printf("%d %d\n", myrank, recieved);
    MPI_Finalize();
    return 0;
}