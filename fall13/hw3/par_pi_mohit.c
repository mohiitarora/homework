#include "mpi.h"
#include <math.h>
#include <time.h>
#include <sys/time.h>
#include <stdio.h>

void decToBinary(int myid, int* decimalRank, int* e);

int main(argc,argv)
     int argc; char *argv[];
{
  struct timeval start,end;
  //gettimeofday(&start,NULL);
  double startTime, endTime;

  int done = 0, n, myid,numprocs,i,rc, e = 0, dimension = 0;//dimension holds the iteration number!
  int decimalRank[20];
  double PI25DT = 3.141592653589793238462643;
  double mypi,pi,h,sum,x,a, receive_buffer;
  MPI_Status status;  
  MPI_Init(&argc,&argv);
  MPI_Comm_size(MPI_COMM_WORLD,&numprocs);
  MPI_Comm_rank(MPI_COMM_WORLD,&myid);
  startTime = MPI_Wtime();//has to be after MPIinitialization to run else gives error
  //Get the decimal equivalent of myid.
  decToBinary(myid, &decimalRank[0], &e);

  //printf("For P = %d\n", numprocs);
  if (myid==0)
    n = atoi(argv[1]);
  while(!done){
    MPI_Bcast(&n,1,MPI_INT,0,MPI_COMM_WORLD);
    if (n == 0)
      break;
    h = 1.0/(double)n;
    sum = 0.0;
    for (i = myid + 1; i <= n; i += numprocs){
      x = h*((double)i - 0.5);
      sum += 4.0/(1.0 + x*x);  
    }
    mypi = h * sum;
    //printf("Processor %d has local pi = %f and mypi = %f\n", myid, pi, mypi);
    //Logic for send and receive along dimensions!
    if(numprocs == 1)
      pi = mypi;
    else {
      
      int k;
      for(k = 0; k < log10(numprocs) / log10(2.0); k++){
	//printf("entering dimension %d \n", dimension);
	dimension = k;
	if(decimalRank[k] == 1){
	  if(k<e){
	    //if it is a processor that need to send then
	    int destination = 0;
	    //find destination processor and send
	    destination = myid ^ (int)pow(2,dimension);
	    //printf("Processor %d sending to %d in dimension %d the value %f\n", myid, destination, dimension,  mypi);
	    
	    MPI_Send(&mypi, 1, MPI_DOUBLE, destination, 0, MPI_COMM_WORLD);
	    //printf("Processor %d done sending to %d in dimension %d the value %f\n", myid, destination, dimension, mypi);
	  }
	}
	else{
	  //Else this processor is supposed to be receiving
	  
	  //printf("Processor %d ready to receive in dimension %d\n", myid, dimension);
	  int source = myid - (int)pow(2,dimension);
	  
	  MPI_Recv(&receive_buffer, 1, MPI_DOUBLE, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status); 
	  //printf("Processor %d received value %f in dimension %d\n", myid, mypi, dimension);
	  mypi += receive_buffer;
	  pi = mypi;
	}
      }
    }
    done = 1;
  }
  endTime = MPI_Wtime();
  //gettimeofday(&end,NULL);
  //int etime = ((end.tv_sec*1000000 + end.tv_usec) - (start.tv_sec * 1000000 + start.tv_usec));
  if (myid == 0){
    printf("Processor Count: %d and Iterations(n): %d---- Time: %f mS.\n", numprocs, n, (endTime-startTime)*1000);
    printf("Calculated Pi: %.15f, Error: %.15f, Iterations: %d, Processor Count: %d\n",pi,fabs(pi-PI25DT),n,numprocs);
    printf("<---------------------------------->\n");
  }
  MPI_Finalize();
}

void decToBinary(int dec, int* decimalRank, int* e)
{
  if(dec == 0){
    decimalRank[0] = 0;
    decimalRank[1] = '\0';
    *e = 1;
  }
  else{
    while(dec>0){
      decimalRank[*e] = dec%2;
      (*e)++;
      dec = dec/2;
    }
  }
}
