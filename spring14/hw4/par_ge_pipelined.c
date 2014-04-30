#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/time.h>
#include <mpi.h>

#define maxP 16 
#define maxN 1024

int P = 0; // Total number of processors
int N = 0; // Matrix size
int ID = 0; // ID
int PID = 0;
int R = 1;
int RPP = 1; //Rows per processor
int BS_MODE = 0; // When set to 1, each node reads data.
int GEN_UX = 0; // When set to 1, outputs UX on a separate file
char* suffix = NULL;;


// USAGE: general_all_to_one_bc(log_2(numprocs), myid, &matrix_element, MPI_DOUBLE, destination);
void general_all_to_one_bc(int dimension, int my_id, double *element, MPI_Datatype dtype, int dest)
{
  MPI_Status status;
  int i=0, j=0;
  int virtual_source=0, virtual_dest=0;
  double sendbuf=0, recvbuf=0;

  int my_virtual_id = my_id ^ dest;
  int mask = 0;

  for(i=0; i<dimension; i++)
    {
      if((my_virtual_id & mask)==0)
	if((my_virtual_id & pow_2(i))!=0)
	  {
	    virtual_dest = my_virtual_id ^ pow_2(i);
	    sendbuf = *element;
	    MPI_Send(&sendbuf, 1, dtype, virtual_dest ^ dest, 99, MPI_COMM_WORLD);
	  }
	else
	  {
	    virtual_source = my_virtual_id ^ pow_2(i);
	    MPI_Recv(&recvbuf, 1, dtype, virtual_source ^ dest, 99, MPI_COMM_WORLD, &status);
	    *element = recvbuf;
	  }
      mask = mask ^ pow_2(i);
    }
}

/*
void general_one_to_all_bc(int dimension, int my_id, double *element, MPI_Datatype dtype, int source){
  MPI_Status status;
  int i=0, j=0;
  int virtual_source=0, virtual_dest=0;
  double sendbuf=0, recvbuf=0;

  int my_virtual_id = my_id ^ source;
  int mask = pow_2(dimension) - 1;

  for(i=dimension-1; i>=0, i--){
      mask = mask ^ pow_2(i);
      if((my_virtual_id & pow_2(i))==0)
	{
	  virtual_dest = my_virtual_id ^ pow_2(i);
	  sendbuf = *element;
	  MPI_Send(&sendbuf, 1, dtype, virtual_dest ^ source, 98, MPI_COMM_WORLD);
	}
      else
	{
	  virtual_source = my_virtual_id ^ pow_2(i);
	  MPI_Recv(&recvbuf, 1, dtype, virtual_source ^ source, 98, MPI_COMM_WORLD, &status);
	  *element = recvbuf;
	}
    }
}
*/
int pow_2(int i){
  if(i==0)
    return 1;
  else
    return 2 << (i-1);
}

int log_2(int p)
{
  int level=0;
  while(p >>= 1) ++level;
  return level;
}

// Returns a high precision timer value in milliseconds
double calctime(){
  struct timeval tp1;
  gettimeofday(&tp1, NULL);
  double time = (tp1.tv_sec * 1000000.0) + tp1.tv_usec;
  return time / 1000.0;
}

// Returns owner of row r
int getOwnerPID(int r){
  int byteCount = R * P;
  int rows = N / byteCount;
  int c = r / rows;
  return c % P;
}

// Manipulate index on one processor
int getPRowIndex(int r)
{
  int byteCount = R * P;
  int rows = N / byteCount;
  int c = r / rows;
  int rcid = r % rows;
  int lcid = c / P;
  // Return local row id.
  return lcid * rows + rcid;
}

//offsets pointer to row j for any processsor
double* getLocalRow(int j, double* myLocalRows){
  if(getOwnerPID(j) == PID){
    int lr = getPRowIndex(j);
    return &myLocalRows[lr * (N + 1)];
  }
  return NULL;
}

void readMatrix(const char* inputFile, double* myLocalRows, float* ddtime){
  double ddstartTime = 0;
  if(PID == 0)
    ddstartTime = calctime();
  
  int nelems = 0;
  FILE* in = fopen(inputFile, "rb");
  if(in != NULL){
      int i;
    for(i = 0; i < N; i++){
      if(getOwnerPID(i) == PID){
	//Row offset.
	int rowSize = 12 * (N + 1);
	int offset = rowSize * i;
	fseek(in, offset, SEEK_SET);
	
	//Read
	int j;
	  for(j = 0; j < N + 1; j++){
	    if(feof(in)){
	      printf("EOF\n");
	      exit(-1);
	    }
	    char intString[20];
	    fgets(intString, 13, in);
	    myLocalRows[nelems] = atof(intString);
	    nelems++;
	  }
      }
    }
    fclose(in);
  }
  else
    printf("Unable to  open file %s\n", inputFile);
  *ddtime = calctime() - ddstartTime;
}

/*
void pivot(double* myLocalRows, double* pivotRow, int j, int jpid, int* localRowIds){
  int i;
  double maxjelem = 0;
  int maxji = -1;
  for(i = 0; i < RPP; i++){
    double* rowi = &myLocalRows[i * (N + 1)];
    double aej = abs(rowi[j]);
      if(aej >= maxjelem && localRowIds[i] >= j){
	maxjelem = aej;
	maxji = i;
      }
  }
  double gm = 0;
  int gmpid = -1;
  double localMaximums[maxP];
  int localMaxRows[maxP];
  
  MPI_Gather(&maxjelem, 1, MPI_DOUBLE, localMaximums, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);
  MPI_Gather(&maxji, 1, MPI_INT, localMaxRows, 1, MPI_INT, 0, MPI_COMM_WORLD);
  
  if(PID == 0){
    for(i = 0; i < P; i++){
      if(localMaximums[i] > gm && localMaxRows[i] != -1){
	gm = localMaximums[i];
	gmpid = i;
      }
    }
  }
  
  MPI_Bcast(&gm, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);
  MPI_Bcast(&gmpid, 1, MPI_INT, 0, MPI_COMM_WORLD);
  
  // Exchange rows.
  if(gmpid == jpid && jpid == PID){
      if(j != localRowIds[maxji]){
	double tmp[maxN];
	memcpy(tmp, &myLocalRows[maxji * (N + 1)], (N + 1) * sizeof(double));
	memcpy(&myLocalRows[maxji * (N + 1)], pivotRow, (N + 1) * sizeof(double));
	memcpy(pivotRow, tmp, (N + 1) * sizeof(double));
      }
  }
  else{
    MPI_Status status;
    if(gmpid == PID){
      double tmp[maxN];
      MPI_Send(&myLocalRows[maxji * (N + 1)], N + 1, MPI_DOUBLE, jpid, 0, MPI_COMM_WORLD);
      MPI_Recv(tmp, N + 1, MPI_DOUBLE, jpid, 0, MPI_COMM_WORLD, &status);
      memcpy(&myLocalRows[maxji * (N + 1)], tmp, (N + 1) * sizeof(double));
    }
    else if(jpid == PID){
      double tmp[maxN];
      MPI_Recv(tmp, N + 1, MPI_DOUBLE, gmpid, 0, MPI_COMM_WORLD, &status);
      MPI_Send(pivotRow, N + 1, MPI_DOUBLE, gmpid, 0, MPI_COMM_WORLD);
      memcpy(pivotRow, tmp, (N + 1) * sizeof(double));
    }
  }
}
*/

void columnPivot(double* localRows, double* pivotRow, int j, int jpid, int* localRowIdss, float* pvtime){

  int i;
  int maxIndex = j;
  float maxElement = pivotRow[j];
  /*if(PID==jpid)
    printf("Element being checked is %f and j = %d\n", maxElement, j);
  if(PID==0)
    printMatrix(localRows, RPP, N+1);
  */
  if(PID == jpid){
    for(i = j+1; i < N; i++){
      if(pivotRow[i] > maxElement){
        maxElement = pivotRow[i];
        maxIndex = i;
      }
    }
    //printf("\nSwap %f at %d with %f at %d if j<maxI\n", pivotRow[j], j, maxElement, maxIndex);
  }

  if(maxIndex>j){
    MPI_Bcast(&maxIndex, 1, MPI_INT, PID, MPI_COMM_WORLD);
    /*if(PID == jpid){
      int i;
      for(i = 0; i<P; i++){
        if(i != jpid)
          MPI_Send(&maxIndex, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
      }
    }
    else{
      MPI_Status status;
      int r_buf;
      MPI_Recv(r_buf,1, MPI_INT, jpid, 0, MPI_COMM_WORLD, &status);
      maxIndex = r_buf;
      }*/
    int k;
    for(k=0; k<RPP; k++){
      float temp = localRows[k*(N+1)+maxIndex];
      localRows[k*(N+1)+maxIndex] = localRows[k*(N+1) + j];
      localRows[k*(N+1) +j] = temp;
    }
    //printf("Swapped Print\n\n");
    //printMatrix(localRows, RPP, N+1);                                                                               
  }
}



void forwardElimination(double* myLocalRows){
  int localRowIds[maxN];
  int i = 0;
  int j;
  int pred, succ;
  pred = (P + (PID -1 )) % P;
  succ = (PID + 1) % P;
  for(j = 0; j < N; j++){
    if(getOwnerPID(j) == PID) {
      localRowIds[i] = j;     
      i++;
    }
  }
        
  for(j = 0; j < N; j++){
    double rowj[maxN];
    double* rowPtr = getLocalRow(j, myLocalRows);
    MPI_Status status;
    int rowSource = getOwnerPID(j);
    
    // Pivot
    //columnPivot(myLocalRows, rowPtr, j, rowSource, localRowIds);
    
    if(rowPtr != NULL)
      memcpy(rowj, rowPtr, (N + 1) * sizeof(double));
    
    if(P>1){
      if(rowSource == PID)
        MPI_Send(rowj, N+1, MPI_DOUBLE, succ, 0, MPI_COMM_WORLD);
      else{
        MPI_Recv(rowj, N + 1, MPI_DOUBLE, pred, 0, MPI_COMM_WORLD, &status);
        if(j < N-1)
          MPI_Send(rowj, N+1, MPI_DOUBLE, succ, 0, MPI_COMM_WORLD);
      }
    }

    // Broadcast the row.
    //MPI_Bcast(rowj, N + 1, MPI_DOUBLE, rowSource, MPI_COMM_WORLD);
    
    int i;
    for(i = 0; i < RPP; i++){
      double* rowi = &myLocalRows[i * (N + 1)];
      double mult = -(rowi[j]/rowj[j]);
      if(localRowIds[i] > j){
	int k;
	for(k = 0; k < N + 1; k++){
	  if(k == j)
	    rowi[k] = 0;
	  else 
	    rowi[k] = mult * rowj[k] + rowi[k];
	}
      }
    }
  }
}


backSubstitution(double* myLocalRows, double* localSolution){
  MPI_Status status;
  int j;
  
  char sendMask[maxN][maxP];
  char recvMask[maxN];
  
  memset(sendMask, 0, maxN * maxP);
  memset(recvMask, 0, maxN);
  
  double x[maxN];
  memset(x, 0, maxN * sizeof(double));
  
  for(j = N - 1; j >= 0; j--){
    double* row = getLocalRow(j, myLocalRows);
    if(row){
      int i;
      double sum = 0;
      for(i = N - 1; i > j; i--){
	int rpid = getOwnerPID(i);
	if(rpid != PID && !recvMask[i]){
	  int err = MPI_Recv(&x[i], 1, MPI_DOUBLE, rpid, i, MPI_COMM_WORLD, &status);
	  if(err != MPI_SUCCESS)
	    printf("MPI_Recv ERROR on NODE %d\n", PID);
	  
	  recvMask[i] = 1;
	}
	sum += x[i] * row[i];
      }
      x[j] = 1.0 / row[j] * (row[N] - sum);

      for(i = j - 1; i >= 0; i--){
	int rpid = getOwnerPID(i);
	if(rpid != PID && !sendMask[j][rpid]){
	  MPI_Send(&x[j], 1, MPI_DOUBLE, rpid, j, MPI_COMM_WORLD);
	  sendMask[j][rpid] = 1;
	}
      }                       
    }
  }
  memcpy(localSolution, x, maxN * sizeof(double));
}

void solve(double* myLocalRows, double* localSolution, float* fetime, float* bstime){
  double festartTime = 0;
  if(PID == 0) 
    festartTime = calctime();
  
  forwardElimination(myLocalRows);
  *fetime = calctime() - festartTime;
  double bsstartTime = 0;
  if(PID == 0) bsstartTime = calctime();
  backSubstitution(myLocalRows, localSolution);
  *bstime = calctime() - bsstartTime;
}

void gatherX(double* localSolution, double* globalSolution){
  MPI_Status status;
  int j;
  for(j = 0; j < N; j++){
    if(getOwnerPID(j) == PID && PID != 0){
      MPI_Send(&localSolution[j], 1, MPI_DOUBLE, 0, j, MPI_COMM_WORLD);
    }
  }
  if(PID == 0){
    for(j = 0; j < N; j++){
      if(getOwnerPID(j) == 0){
	globalSolution[j] = localSolution[j];
      }
      else{
	MPI_Recv(&globalSolution[j], 1, MPI_DOUBLE, getOwnerPID(j), j, MPI_COMM_WORLD, &status);
      }
    }
  }
}

void gatherU(double* myLocalRows, double* U){
  MPI_Status status;
  int j;
  for(j = 0; j < N; j++){
    double* row = getLocalRow(j, myLocalRows);
    if(row != NULL && PID != 0) 
      MPI_Send(row, N + 1, MPI_DOUBLE, 0, j, MPI_COMM_WORLD);
  }
 
  if(PID == 0){
    for(j = 0; j < N; j++){
      if(getOwnerPID(j) == 0)
	memcpy(&U[j * (N + 1)], &myLocalRows[getPRowIndex(j) * (N + 1)], (N + 1) * sizeof(double));
      else
        MPI_Recv(&U[j * (N + 1)], N + 1, MPI_DOUBLE, getOwnerPID(j), j, MPI_COMM_WORLD, &status);
    }
  }
}

void calcChecksums(double* U, double* X, double* cu1, double* cu2, double* cx1, double* cx2){
  *cu1 = 0;
  *cu2 = 0;
  *cx1 = 0;
  *cx2 = 0;
  int j, k;
  for(j = 0; j < N; j++){
    *cx1 += X[j];
    *cx2 += j%2 ? -X[j] : X[j];
    for(k = 0; k < N; k++){
      double u = U[j * (N + 1) + k];
      *cu1 += u;
      *cu2 += k%2 ? -u : u;
    }
  }
}

void outputResults(const char* inputFile, double* myLocalRows, double* localSolution, float ddtime, float bstime, float fetime){
  int j = 0;
  double X[maxN];
  double* U = malloc(N * (N + 1) * sizeof(double));
        
  gatherX(localSolution, X);
  gatherU(myLocalRows, U);
        
  double cx1, cx2, cu1, cu2;
        
  if(PID == 0){
    calcChecksums(U, X, &cu1, &cu2, &cx1, &cx2);
    
    float sequentialTimeGE = 0;
    float sequentialTimeGEBS = 0;
    float sequentialTimeGEDD = 0;
    float sequentialTimeGEBSDD = 0;
    char filename[32];
    
    if(P > 1){
      sprintf(filename, "stat_P%d_bs%d_r%d_n%d_%d_%s.txt", 1, BS_MODE, R, N, ID, suffix);
      FILE* in = fopen(filename, "r");
      
      char line[512];
      // seek to line 6
      fgets(line, 512, in);
      fgets(line, 512, in);
      fgets(line, 512, in);
      fgets(line, 512, in);
      fgets(line, 512, in);
      fgets(line, 512, in);
      
      fscanf(in, "Sequential time: GE = %f ms\n", &sequentialTimeGE);
      fscanf(in, "Sequential time: GE+BS = %f ms\n", &sequentialTimeGEBS);
      fscanf(in, "Sequential time: GE+DD = %f ms\n", &sequentialTimeGEDD);
      fscanf(in, "Sequential time: GE+BS+DD =  %f ms\n", &sequentialTimeGEBSDD);
      fclose(in);
    }
    
    // Write output
    sprintf(filename, "stat_P%d_bs%d_r%d_n%d_%d_%s.txt", P, BS_MODE, R, N, ID, suffix);
    FILE* out = fopen(filename, "w");
    
    fprintf(out, "P = %d, BS mode = %d, n = %d, r = %d, input_file = %s\n", P, BS_MODE, N, R, inputFile);
    fprintf(out, "**************************************************************************************************************\n\n");
    fprintf(out, "Checksum1(U) = %11.4e,    Checksum2(U) = %11.4e,    Checksum1(X) = %11.4e,    Checksum2(X) = %11.4e\n", cu1, cu2, cx1, cx2);
    fprintf(out, "**************************************************************************************************************\n\n");
    
    if(P == 1){
      fprintf(out, "Sequential time: GE = %.3f ms\n", fetime);
      fprintf(out, "Sequential time: GE+BS = %.3f ms\n", fetime + bstime);
      fprintf(out, "Sequential time: GE+DD = %.3f ms\n", fetime + ddtime);
      fprintf(out, "Sequential time: GE+BS+DD = %.3f ms\n", fetime + bstime + ddtime);
    }
    
    // Print Speedups
    if(P > 1){

      fprintf(out, "Sequential time GE = %.3f ms\n", sequentialTimeGE);
      fprintf(out, "Parallel Time: GE = %.3f ms\n", fetime);
      fprintf(out, "Speedup: GE = %.3f\n\n", sequentialTimeGE/fetime);

      fprintf(out, "Sequential Time GE+BS = %.3f ms\n", sequentialTimeGEBS);
      fprintf(out, "Parallel Time: GE+BS = %.3f ms\n", fetime + bstime);
      fprintf(out, "Speedup: GE+BS = %.3f\n\n", sequentialTimeGEBS/(fetime + bstime));

      fprintf(out, "Sequential time GE+DD = %.3f ms\n", sequentialTimeGEDD);
      fprintf(out, "Parallel time GE+DD = %.3f ms\n", fetime + ddtime);
      fprintf(out, "Speedup: GE+DD = %.3f\n\n", sequentialTimeGEDD/(fetime + ddtime));

      fprintf(out, "Sequential Time GE+BS+DD = %.3f ms\n", sequentialTimeGEBSDD);
      fprintf(out, "Parallel Time GE+BS+DD = %.3f ms\n", fetime+bstime+ddtime);
      fprintf(out, "Speedup GE+BS+DD = %.3f\n", sequentialTimeGEBSDD/(fetime + bstime+ ddtime));
      
    }
    fprintf(out, "-------------------------------------------------------------------------------------------------------------\n\n");
    fclose(out);
    
  }
  
  if(GEN_UX){
    if(PID == 0)
      {
	// Write the solution vector and U matrix.
	char filename[32];
	sprintf(filename, "UX_P%d_bs%d_r%d_n%d_%d_%s.txt", P, BS_MODE, R, N, ID, suffix);
	FILE* out = fopen(filename, "w");
	fprintf(out, "P = %d, bs_mode = %d, N = %d, R = %d, input = %s\n", P, BS_MODE, N, R, inputFile);
	fprintf(out, "****************************************************************************************************\n");
	fprintf(out, "\nChecksum1(U) = %11.4e,    Checksum2(U) = %11.4e,    Checksum1(X) = %11.4e,    Checksum2(X) = %11.4e\n", cu1, cu2, cx1, cx2);
	fprintf(out, "\n****************************************************************************************************\n");
	fprintf(out, "\nX Vector:\n");
	//outputSolutionVector(X)
	for(j = 0; j < N; j++)
	{
		fprintf(out, "%11.4e", X[j]);
		if((j + 1) % 16 == 0)
			fprintf(out, "\n");
		else
			fprintf(out, " ");
	}
 	fprintf(out, "\n\n****************************************************************************************************\n");
	fprintf(out, "\nU Matrix:\n");
	//outputMatrix(U)
	for(j = 0; j < N; j++)
	{
		fprintf(out, "\nRow %d:\n", j);
		int i;
		double* row = &U[j * (N + 1)];
		for(i = 0; i < N + 1; i++)
		{
		fprintf(out, "%11.4e", row[i]);
		if((i + 1) % 16 == 0)
			fprintf(out, "\n");
		else
			fprintf(out, " ");
		}
		fprintf(out, "\n----------------------------------------------------------------");
	}
	fclose(out);
      }
  }
}

void printMatrix(double* myData, int rows, int columns){
  int i, j;
  for( i=0; i < rows; i++ ) {
    for( j=0; j<columns; j++ )
      printf("%f    ",myData[i*(N+1) + j]);
    printf("\n");
  }
}

int main(int argc, char** argv)
{
  if(argc < 8){
    printf("Usage: %s <n:matrix size> <id> <matrix input file> <bs mode:0/1> <r:block cyclic factor> <genUX:0/1> <suffix>\n", argv[0]);
    return 0;
  }
  
  // Read arguments
  N = atoi(argv[1]);
  ID = atoi(argv[2]);
  char* inputFile = argv[3];	  
  BS_MODE = atoi(argv[4]);
  R = atoi(argv[5]);
  GEN_UX = atoi(argv[6]);
  suffix = malloc(strlen(argv[7])*sizeof(char));
  strncpy(suffix, argv[7], strlen(argv[7]));
        
  // Initialize MPI
  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &P);
  MPI_Comm_rank(MPI_COMM_WORLD, &PID);
        
  RPP = N/P;
        
  MPI_Bcast(&N, 1, MPI_INT, 0, MPI_COMM_WORLD);
  MPI_Bcast(&R, 1, MPI_INT, 0, MPI_COMM_WORLD);
        
  double* myLocalRows = malloc(RPP * (N + 1) * sizeof(double));

  double x[maxN];
  memset(x, 0, maxN * sizeof(double));
        
  float ddtime;
  float fetime;
  float bstime;

  readMatrix(inputFile, myLocalRows, &ddtime);
  // Gaussian Elimination:
  solve(myLocalRows, x, &fetime, &bstime);
  // Output solution and statistics
  outputResults(inputFile, myLocalRows, x, ddtime, bstime, fetime);
  MPI_Finalize();
  return 0;
}
