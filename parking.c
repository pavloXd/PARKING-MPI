#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define VEHICULOS 1000
#define PARKING 0
#define BOOLEAN 3


int plazas;
int plantas;
int plazas_total;
int plazas_libres;
int vehiculos_esperando;
int cola[VEHICULOS];
int myid;

int** parking;
typedef struct {
	int tipo; //tipo 2 coche, 1 camion
	int id;
} vehiculo;

vehiculo* vehiculos;

int camiones(int identificacion);
void eliminarCola(vehiculo v);
void insertarCola(vehiculo v);
int coches (int identificacion);
int buscarPlazas(int *plaza, int *planta, vehiculo v);
int estaDentro(int identificador);


void imprimirParking() {
  int i, j;

  for(i=0; i<plantas; i++) {
    for(j=0; j<plazas; j++) {
      printf("[%d]\t", parking[i][j]);
    }
    printf("\n");
  }
}


int main(int argc, char *argv[]){

	int numprocs;
    	int myid;
	MPI_Status stat;
	

	int tag;
	int i, j, k;
	

	


	MPI_Init(&argc,&argv);
    	MPI_Comm_size(MPI_COMM_WORLD,&numprocs);
    	MPI_Comm_rank(MPI_COMM_WORLD,&myid);

	printf("PARKING:soy el proceso %d\n",myid);
	printf("PARKING:Tenemos %d procesos\n", numprocs);

	//definimos el numero de plazas
	//Comprobamos que hayan introducido los valores de entrada
	if (argc-1<1){
        printf("PARKING:error de falta parametro\n");
        MPI_Finalize();
        return -1;
	
    	}else{
		//En caso de que intruduzcan un valor negativo o cero
		if((atoi((argv[1])))<1){
			printf("PARKING:error minimo una plaza en el parking\n");
        		MPI_Finalize();
        		return -1;	
		}else{
			plazas= atoi(argv[1]);//guardamos el numero de plazas
		}
	}

	//definimos el numero de plantas
	if (argc - 1 < 2){//En caso de que no se intruduzca ningun parametro, se pondra una planta
        	plantas = 1;
    	}
    	else{
		if ((atoi(argv[2]))<1){
			printf("PARKING:error minimo una planta en el parking\n");
        		MPI_Finalize();
        		return -1;
		}
        	plantas = atoi(argv[2]);
    	}
	printf("PARKING: se crea un parking con %d plazas por planta y %d plantas \n", plazas, plantas);





	//Creamos el parking
	plazas_total=plantas*plazas;
	plazas_libres=plazas_total;

	//pedimos memoria para las plantas
	parking = (int**) malloc(sizeof(int*) * plantas);
	//pedimos memoria para las plazas/planta
	for( k=0; k<plantas; k++){
    		parking[k] = (int *) malloc(sizeof(int) * plazas);
	}
	//ponemos todas las plazas libres
	for(i=0; i<plantas;i++){	
		for( j=0; j<plazas; j++){
				parking[i][j]=0;
		}
	}

	
imprimirParking();

	vehiculos = (vehiculo*) malloc(sizeof(vehiculo) * VEHICULOS);

	
	int nid;

	while(1){
		

		//comprobamos si pudieramos entrar
		int i=0;
		//Tenemos que comprobar si hay algun coche a la espera de querer entrar al parking
		//en caso de que haya en ningun momento se puede procesar un recibo de mpi
		int salir=1;

		//for( i=0; i<vehiculos_esperando; i++) {
		while(i<vehiculos_esperando && salir==1){

			//si son camiones
      			if(vehiculos[i].tipo == 1){
 				int aux=vehiculos[i].id;

				if((camiones(vehiculos[i].id))==0){
					printf("PARKING: Camion %d entra al parking desde la cola\n", aux);
					usleep(100);
					MPI_Send(&myid, 1, MPI_INT, aux, 1, MPI_COMM_WORLD);
				}
				else{

					salir=0;

				}
			}
			//si son coches
      			else{
				int aux=vehiculos[i].id;

				if((coches(vehiculos[i].id))==0){
					printf("PARKING: Coche %d entra al parking desde la cola\n", aux);
					//usleep(100);
					MPI_Send(&myid, 1, MPI_INT, aux, 0, MPI_COMM_WORLD);
				}
				else{
					salir=0;

				}
			}
			i++;
    		}
		//recibimos el mpi
		MPI_Recv(&nid, 1, MPI_INT,MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &stat);
		if(estaDentro(nid)==1){
			//significa que quiere salir
			if(stat.MPI_TAG == 1) {
	      			if((camiones(nid))==0){
					usleep(100);
					MPI_Send(&myid, 1, MPI_INT, nid, 1, MPI_COMM_WORLD);
				}
				
	    		}
			else{
				//En caso de que se haya hecho algo, es decir no se haya quedado en cola
	      			if((coches(nid))==0){
					usleep(100);
					MPI_Send(&myid, 1, MPI_INT, nid, 0, MPI_COMM_WORLD);
				}
	    		}
					
		}
		else{
			//significa que quiere entrar al parking, le ponemos directamente a la cola
			if(stat.MPI_TAG == 1) {
	      			vehiculo camion;
				//creamos el camion
				camion.tipo=1;
				camion.id=nid;
				//Lo ponemos en la cola
				insertarCola(camion);
				
	    		}
			else{
				vehiculo coche;
				//creamos el camion
				coche.tipo=0;
				coche.id=nid;
				insertarCola(coche);
	    		}

		}
		
		
		
		 fflush(stdout);





	}
	MPI_Finalize();
	return 0;

}

int camiones(int identificacion) {
	vehiculo camion;
	//creamos el camion
	camion.tipo=1;
	camion.id=identificacion;	

	//inicializamos para evitar que nos de error
	int plaza=0;
	int planta=0;
	
	if(buscarPlazas(&plaza, &planta, camion)==-1){
		//si NO hay plaza insertamos el vehiculo en la cola
		insertarCola(camion);
		return -1;
	}
	else{
		//si la hay, buscarPlaza ya he ha asignado una plaza, borraremos de la cola
		eliminarCola(camion);

		imprimirParking();
    		return 0;
  	}
}

int coches (int identificacion){
	vehiculo coche;
	//creamos el camion
	coche.tipo=2;
	coche.id=identificacion;	

	//inicializamos para evitar que nos de error
	int plaza=0;
	int planta=0;
	
	if(buscarPlazas(&plaza, &planta, coche)==-1){
		//si NO hay plaza insertamos el vehiculo en la cola

		insertarCola(coche);
		return -1;
	}
	else{
		//si la hay, buscarPlaza ya he ha asignado una plaza, borraremos de la cola
		eliminarCola(coche);

		imprimirParking();
    		return 0;
  	}

}

void insertarCola(vehiculo v){
	int salir=1;
	for (int i=0; i<vehiculos_esperando;i++){
		if(vehiculos[i].id==v.id){
		salir=0;	
		}
	}
	//en caso de que no se encuentre ya en la lista, lo anadimos
	if(salir==1){
		vehiculos[vehiculos_esperando]=v;
		vehiculos_esperando++;
	}
}


void eliminarCola(vehiculo v){


	for (int i=0; i<vehiculos_esperando;i++){

		if(vehiculos[i].id==v.id){
			vehiculos[i]=vehiculos[vehiculos_esperando-1];
			vehiculos_esperando --;
		}
	}
}

int buscarPlazas(int *plaza, int *planta, vehiculo v){
	//en caso de que sea camion
	if(v.tipo==1){
		//Recorremos el for en busca de si el vehiculo pudiera ya estar aparcado
		for(int i=0; i<plantas; i++) {
      			for(int j=0; j<plazas; j++){
				//En caso de que ya este aparcado lo retiraremos
				if(parking[i][j]==v.id){
					parking[i][j]=0;
					parking[i][j+1]=0;	
					plazas_libres++;
					plazas_libres++;
					printf("PARKING: Camion %d saliendo, quedan %d plazas libres\n", v.id, plazas_libres);
					return 0;
					
				}

			}
		}
		//Si llegamos aqui es que no esta aparcado, procedemos a buscar un sitio acorde

		
		for(int i=0; i<plantas; i++) {
      			for(int j=0; j<plazas; j++){
				//comprobamos que esten dos plazas seguidas libres y que no sea la primera el ultimo sitio para no salirse del array
				if(parking[i][j] == 0 && parking[i][j+1] == 0 && j != (plazas-1)) {
					*plaza=j;
					*planta=i;
					parking[i][j]=v.id;
					parking[i][j+1]=v.id;
					plazas_libres--;
					plazas_libres--;
					printf("PARKING: Camion %d entrando, quedan %d plazas libres, aparca en Planta %d, Plaza %d \n", v.id, plazas_libres, *planta, *plaza);
					return 0;
				}
			}
		}
	}
	//Caso en que el vehiculo sea un coche

	else{
		for(int i=0; i<plantas; i++) {
      			for(int j=0; j<plazas; j++){
				if(parking[i][j]==v.id){
					parking[i][j]=0;	
					plazas_libres++;
					printf("PARKING: Coche %d saliendo, quedan %d plazas libres\n", v.id, plazas_libres);
					return 0;
					
				}

		
			}
		}
		for(int i=0; i<plantas; i++) {
      			for(int j=0; j<plazas; j++){
				if(parking[i][j] == 0 ) {
					*plaza=j;
					*planta=i;
					parking[i][j]=v.id;
					plazas_libres--;
					printf("PARKING: Coche %d entrando, quedan %d plazas libres, aparca en Planta %d, Plaza %d \n", v.id, plazas_libres, *planta, *plaza);
					//MPI_Send(&myid, 1, MPI_INT, colaAux->pid, 0, MPI_COMM_WORLD);
					return 0;
				}
			}
		}	
	}
	//Caso en el que ni este dentro del parking ni haya plaza disponible
	printf("PARKING: No hay sitio libre para el vehiculo %d\n", v.id);
	return -1;
}


int estaDentro(int identificador){
	for(int i=0; i<plantas; i++) {
      			for(int j=0; j<plazas; j++){
				if(parking[i][j] ==identificador) {
					return 1;
				}
			}
	}
	return 0;

}












