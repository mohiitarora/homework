#include "mpi.h"
#include <math.h>
#include <time.h>
#include <sys/time.h>
int main(argc,argv)
     int argc; char *argv[];
{
  struct timeval start,end;
  //gettimeofday(&start,NULL);
  double startTime, endTime;
  int done = 0, n, myid,numprocs,i,rc;
  double PI25DT = 3.141592653589793238462643;
  double mypi,pi,h,sum,x,a;

  MPI_Init(&argc,&argv);
  MPI_Comm_size(MPI_COMM_WORLD,&numprocs);
  MPI_Comm_rank(MPI_COMM_WORLD,&myid);
  startTime = MPI_Wtime();
  if (myid==0)
      n = atoi(argv[1]);
  while(!done) {
    MPI_Bcast(&n,1,MPI_INT,0,MPI_COMM_WORLD);
    if (n == 0) break;
    h = 1.0/(double)n;
    sum = 0.0;
    for (i=myid + 1;i <= n; i+= numprocs){
      x = h*((double)i - 0.5);
      sum += 4.0/(1.0 + x*x);
    }
    mypi = h * sum;
    MPI_Reduce(&mypi,&pi,1,MPI_DOUBLE,MPI_SUM,0,MPI_COMM_WORLD);
    done = 1;
  }
  //gettimeofday(&end,NULL);
  //int etime = ((end.tv_sec*1000000 + end.tv_usec) - (start.tv_sec * 1000000 + start.tv_usec));
  endTime = MPI_Wtime();
  if (myid == 0){
    printf("Processor Count: %d,  Iterations(n): %d and Time: %f \n", numprocs, n, (endTime-startTime)* 1000);
    printf("Calculated Pi: %.15f, Error: %.15f \n",pi,fabs(pi-PI25DT));
    printf("<------------------------------------>\n");
    
  }
  MPI_Finalize();
}
