#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

//#define CAMION 1

int main(int argc, char *argv[]){
	int numprocs;
	int myid;
	int posparking;
	MPI_Status stat;

	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD,&numprocs);
    	MPI_Comm_rank(MPI_COMM_WORLD,&myid);

	printf("COCHE:soy el proceso %d\n",myid);
	//MPI_Barrier(MPI_COMM_WORLD);
	while(1){
		printf("COCHE %d: dado una vuelta\n", myid);
		sleep(rand()%(10+myid));

		printf("COCHE %d:Quiero entrar al parking\n",myid);
		//Envimos peticion para entrar al parking y queda bloqueado. 
        	MPI_Send(&myid, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);

		//Recibimos peticion para entrar en parking, desbloquea el proceso
		MPI_Recv(&posparking, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &stat);
	
		printf("COCHE %d: ya estoy aparcado \n",myid);
		sleep(rand()%(10+myid));

		sleep(rand()%(10+myid));
		//printf("COCHE:%d quiero salir del parking\n",myid);
        	MPI_Send(&myid, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
		
        	//Recibe que ha salido correctamente del parking
        	MPI_Recv(&posparking, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &stat);
        	printf("COCHE %d saliendo del parking\n",myid);
	}

	MPI_Finalize();
    	return 0;
}
