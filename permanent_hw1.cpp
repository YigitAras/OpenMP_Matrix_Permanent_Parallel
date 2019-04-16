#include <iostream>
#include <fstream>
#include <string.h>
#include <sstream>
#include <omp.h>
#include <stdlib.h>
#include <math.h>

using namespace std;



void usage()
{
  cout << "USAGE: ./exec <filename>" << endl;
  exit(0);
}
void transpose(int **A, int **B,int N){
  int i,j;
  for(int i=0; i< N ; i++){
    for (j=0 ; j<N; j++){
      B[i][j] = A[i][j];
    }
  }

}

int main(int argc, const char** argv)
{

  if(argc != 2)
    usage();

  string line;

  const char* filename = argv[1];
  ifstream input (filename);
  if(input.fail())
    return 0;


  int N;
  int **M;
  int **MT;
  getline(input,line);
  N = atoi(line.c_str());
  M = new int*[N];
  MT = new int*[N];
  for(int i = 0; i < N; i ++){
    M[i] = new int[N];
    MT[i] = new int[N];
  }


  int linectr = 0;
  while(getline(input,line)){
    stringstream ss(line);
    int temp;
    int ctr = 0;
    while(ss >> temp)
      M[linectr][ctr++] = temp;

    linectr++;
  }
  transpose(M,MT,N);
  double start, end;
  for(int t = 1; t <=16; t*=2) { //t is the number of threads
    start = omp_get_wtime();
  ////YOUR CODE GOES HERE

    // C memory magic
    double *x = (double*) malloc(sizeof(double)*N);
    memset(x,0.0,sizeof(double)*N);

    // set the X colm
    double p  = 1.0;
    for(int i=0;i<N;i++){
      x[i] = MT[N-1][i];
      for(int j=0;j<N;j++){
          x[i] -= ((double)MT[j][i] / 2 );
      }
      p *= x[i];
    }

    // TRANSPOSE
    // __builtin_ctz instead of log2
    omp_set_num_threads(t);
    #pragma omp parallel proc_bind(spread) reduction(+:p)
    {
      // FOR BETTER PARALLEL IMPLEMENTATION [3]
      double *x_specul = (double*)malloc(sizeof(double)*N);
      memset(x_specul,0.0,sizeof(double)*N);
      int id = omp_get_thread_num();
      long long int start_loc = (long long int)id*(1<<(N-1)) / t;
      int y = (start_loc>>1) ^ start_loc;

      for(int j=0 ; j<N; j++){
        x_specul[j] = x[j];
	  for(int k = 0; k<N; k++){ // traverse each byte to see if it is necessary
          if(((y>>k) & 1) == 1){x_specul[j] += MT[k][j];}
        }
      }

    #pragma omp for schedule(static)
    for(long long int i = 1; i < (1<<(N-1));i++)
    {

      int y = (i>>1) ^ i; // gray-code order
      int yy = ( (i-1) >> 1 ) ^ ( i-1 ); // i-1's gray-code order
      int z = __builtin_ctz(y^yy) ; // get the changing bit
      int s = ( (y >> (z) ) & 1) == 1 ?  1:-1; // find changing bit
      int prodsign = (i & 1) == 0 ? 1:-1; // get the prodsign
      double dd = 1.0;

      #pragma omp simd reduction(*:dd)
      for(int j= 0; j< N; j++){
        x_specul[j] += (double)(s * MT[z][j]);
        dd *= x_specul[j];
      }
      p += (double)(prodsign*dd);
  }
  delete[] x_specul;
}
  double result = (4*(N & 1) - 2) * p;
  //// YOUR CODE ENDS HERE
  end = omp_get_wtime();
   cout << "Threads: " << t << "\tResult:" << result << "\tTime:" << end - start << " s" << endl;
  // cout << end-start << endl;
}
  return 0;
}
