#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#define MSG_SIZE 2
#define TAG 100

char* wpis;
int* numer;
int num_hosts;
MPI_Status status,status2;
void init()
{
	wpis=(char*)malloc(sizeof(char)*num_hosts);
	numer=(int*)malloc(sizeof(int)*num_hosts);
	for(int i=0;i<num_hosts;i++)
	{
		wpis[i]='F';
		numer[i]=0;
	}
}

void blokuj(int i)
{
	wpis[i]='T';
	int max=0;
	for(int id=0;id<num_hosts;id++)
	{
		if(numer[id]>max)
		{
		max=numer[id];
		}
	}
	numer[i]=1+max;
	wpis[i]='F';
	for(int id=0;id<num_hosts;id++)
	{
	if(id!=i)
	{
	MPI_Send( numer, num_hosts, MPI_INT, id, TAG+1, MPI_COMM_WORLD );
	MPI_Send( wpis, num_hosts, MPI_CHAR, id, TAG, MPI_COMM_WORLD );
	}
	}
	
	for(int j=0;j<num_hosts;j++)
	{
	
	if(i!=j)
	{

		do
		{
			//printf("%d: czeka na pozwolenie %d\n", i, j);
			
			MPI_Recv(wpis, num_hosts, MPI_CHAR, j, TAG, MPI_COMM_WORLD, &status);
			//printf("%d: wpis%d %c\n", i, j,wpis[j]);
		}
		while(wpis[j]=='T');
		do
		{
			//printf("%d: czeka na priorytet %d\n", i, j);
			MPI_Recv(numer, num_hosts, MPI_INT,  j, TAG+1, MPI_COMM_WORLD, &status);
		}while(!(numer[j]==0 || (numer[j]>numer[i]||(numer[j]==numer[i] && i<j))));
	}
	}


}

void odblokuj(int i)
{
numer[i]=0;
}

int main(int argc, char **argv)
{
	srand(time(NULL));
	int rank,sender;
	
	int sum[1];
	MPI_Init(&argc, &argv);

	MPI_Comm_rank( MPI_COMM_WORLD, &rank );
	MPI_Comm_size(MPI_COMM_WORLD,&num_hosts);
	init();
	sender = 0;
	int msg[MSG_SIZE];
	if(rank == sender)
	{
	sum[0]=0;
	
	for(int i=0;i<num_hosts;i++)
	{
	
	MPI_Send( sum, 1, MPI_INT, i, TAG+2, MPI_COMM_WORLD );
	
	}
	}
	while(1)
	{
        sleep(rand()%3);
	blokuj(rank);
	
	printf("Sekcja krytyczna dla proces: %d\n",rank);
	MPI_Recv(sum, 1, MPI_INT,  MPI_ANY_SOURCE, TAG+2, MPI_COMM_WORLD, &status);
	
	printf("Value received: %d\n",sum[0]);
	
	printf("Proces zmieniający: %d\n",rank);
	sum[0]++;
        
	for(int i=0;i<num_hosts;i++)
	{
	if(i!=rank)
	{
	
	MPI_Send( sum, 1, MPI_INT, i, TAG+2, MPI_COMM_WORLD );
	}
	}
	
	sleep(rand()%3);
	odblokuj(rank);
	}	

	MPI_Finalize();
}
