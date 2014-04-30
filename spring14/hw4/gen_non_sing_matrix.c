#include<stdlib.h>
#include<stdio.h>
#include<math.h>

int det(int, int, float[][]);
float generate_rand(float);

int main(int argc, char* argv[]) {
	
	if(argc < 3){
    	printf("Usage: %s <size of matrix> <# of matrices to generate>\n", argv[0]);
    	return 0;
	}
	
	int rows = atoi(argv[1]);
	int cols = rows;
	int matrix_num = atoi(argv[2]);
    int rank, size, num, i, j, k;
    float tempMatrix[rows][cols], aMatrix[rows][cols], bMatrix[rows][1], final_number, mant_r, exp_r, b_r, file_r ;
    int limit_per_no = pow(2,16), sign_exp, sign_mant;
    char op_file_name[100];
    FILE *fp;
    srand(time(NULL));
    
    
    double det_val;
    for(k=0;k<matrix_num;k++){
        file_r = (rand()%100)/(float)100;
        
      do{
        for(i=0; i<rows;i++){
            for(j=0;j<cols;j++){
                final_number = generate_rand(file_r);
                aMatrix[i][j] = final_number;
                tempMatrix[i][j]= final_number;
            }
            bMatrix[i][0] = generate_rand(file_r);
        }
        det_val = det(rows, cols, tempMatrix);
        //~ printf("The determinant: %f", det_val);
      }while(det_val == 0);
    sprintf(op_file_name,"matrix_%d_%d.txt",rows,k+1);
    fp = fopen(op_file_name, "w");
        //~ fprintf(fp, "%f\n", exp_r);
      for(i=0; i<rows; i++){
        for(j=0; j<cols; j++){
          fprintf(fp, "%011.4e", (float)aMatrix[i][j]);
          if((j+1)%16 == 0 )
            fprintf(fp, "\n");
          else
            fprintf(fp, " ");
        }
        fprintf(fp, "%011.4e",(float) bMatrix[i][0]);
        if(j!=cols-1){
          fprintf(fp, "\n");
        }
      }
      fclose(fp);
    }
        char inputFileName[80];
       return 0;
}

int det(int rows, int cols, float mat[rows][cols]){
    double ratio, det;
    int i, j, k;
    /* Conversion of matrix to upper triangular */
    for(i = 0; i < rows; i++){
        for(j = 0; j < cols; j++){
            if(j>i){
                ratio = mat[j][i]/mat[i][i];
                for(k = 0; k < rows; k++){
                    mat[j][k] -= ratio * mat[i][k];
                    if((mat[j][k] <= 0.001 && mat[j][k] >0) || (mat[j][k] >= -0.01 && mat[j][k] <0))
                      mat[j][k] =0;
                }
            }
        }
    }
    det = 1; //storage for determinant
    for(i = 0; i < rows; i++){
        if(mat[i][i] == 0)
            return 0;
      }
   return 1;
}


float generate_rand(float file_r){
  float final_number, mant_r, exp_r;
  int sign_exp;
  do{
    mant_r = rand() % 65536;
    while(mant_r >10){
    mant_r = mant_r/(float)10;
        if (mant_r<10)
            break;
    }
    sign_exp = (rand() % 2 ==0)? -1:1 ;
    mant_r = mant_r * sign_exp;
    
    if(file_r <=.33){
        exp_r = (rand() % 7) -3;
    }
    else if(file_r <=.66){
        exp_r = (rand() % 7) -1;
    }
    else{
        exp_r = (rand() % 7) +2;
    }
    
    final_number = mant_r * pow(2, exp_r);
  }while(final_number == 0);
  return final_number;
}
