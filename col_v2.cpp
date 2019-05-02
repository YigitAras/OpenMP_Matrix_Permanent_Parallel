#include <iostream>
#include <stdlib.h>
#include <string>
#include <fstream>
#include <sstream>
#include <omp.h>
#include <limits.h>
#include <vector>
#include <algorithm>
#include <cstring>
using namespace std;

int GraphColorChecker(int *row,int *col,int *colour,int N){
  int err_count = 0;
  for(int i=0;i<N;i++){
      int start = row[i];
      int end = row[i+1];

      for(int j=start; j<end; j++){
        if(colour[i] == colour[col[j]]) err_count++;
      }
  }
 // if(err_count>0) printf("Error, ");
 // else printf("Succs, ");
  int max = 0;

  for(int i=0;i<N;i++){
    if(colour[i]>max){
      max = colour[i];
    }
  }

  //printf(" Colour Count: %d\n",max);
  return max;
}

using namespace std;
int *row;
int *col;
int N;

int main(int argc, const char **argv) {

  if(argc != 2) {
            cout << "No input specified" << endl;
            return 0;
        }
        const char *filename = argv[1];

        //string filepath = "./data/" + filename;

        ifstream file;

        file.open(filename);
        if(file.fail()) {
            cout << "Cannot find given file" << endl;
            return 0;
        }

        string line;
        getline(file,line);

        stringstream s(line);
        bool symmetric = false;
        string word;
        while(s >> word) // check for symmetric
            if(word == "symmetric")
                symmetric = true;

        while(file.peek() == '%')
            file.ignore(2048, '\n');

        int N, M, edge;
        file >> N >> M >> edge;



        //cout << "Graph has " << N << " nodes and " << (1+symmetric*1)*edge << " edges and symmetric " << symmetric << endl;

        int n;
        bool based0 = false;
        while(file >> n) {
            if (n == 0) {
                based0 = true;
                break;
            }
        }
        file.close();
       /*
	 if(based0)
            cout << "Graph is 0 based" << endl;
        else
            cout << "Graph is 1 based and is being turned in to 0 base." << endl;
	*/
        file.open(filename);
        while(file.peek() == '%')
            file.ignore(2048, '\n');
        file >> n >> n >> n; //tmp

        // by default all the graphs are 0 based;

        int i,j;
        vector<vector<int>> v(N,vector<int>(0));
        int e = 0;
        while(!file.eof()) {
            file >> i >> j;
            if(!based0) {
                i--;
                j--;
            }
            if(i==j)
                continue;
            v[i].push_back(j);
            v[j].push_back(i);
        }

        for(int i = 0; i < N; i++) {
            if(!v[i].empty()) {
  	    sort(v[i].begin(), v[i].end(), greater<int>());
                int j = 0;
                while (j < v[i].size() - 1) {
                    if (v[i][j] == v[i][j + 1]) {
                        v[i].erase(v[i].begin() + j);
                    } else
                        j++;
                }
            }
        }
        file.close();

        for(int i = 0; i < v.size(); i++)
            e += v[i].size();


        edge = e;
        // switch to CRS
        // all non zeros are 1

        int *row = new int[N+1];
        int *col = new int[edge];



        // for errors
        int *errors = new int[N];
        int *colour = new int[N];
        for (int i=0;i<N;i++){
          errors[i] = i;
        }
        row[0] = 0;
        int index = 0;
        for(int i = 0; i < N; i++) {
            for(int j = 0; j < v[i].size(); j++) {
                col[index] = v[i][j];
                index++;
            }
            row[i+1] = index;
        }

    //cout << "Preprocessing complete " << endl;
double start, end;
// ========== ALGORITHM BELOW ========================

for(int t = 1; t <=16; t*=2) { //t is the number of threads
    omp_set_num_threads(t);
    int err = N;
    for(int i=0;i<N;i++) colour[i] = 0;
    for (int i=0;i<N;i++) errors[i] = i;
start = omp_get_wtime();
while(err != 0){
  #pragma omp parallel proc_bind(spread)
  {
      // ============ colour assignment area below ====================
      int *forbid = new int[N]();

      #pragma omp for schedule(guided)
      for(int i=0;i<=err;i++){
        int ind = errors[i];
        int start = row[ind];
        int end = row[ind + 1];
        for(int j=start ; j< end;j++){
          // Set the forbiddensssssss
          forbid[colour[col[j]]] = ind;
        }

        for(int k=0;k<N;k++){
          if(forbid[k] != ind){
            colour[ind] = k;
            break;
          }
        }
      }
     delete[] forbid;
  }
  err = 0;
  if(t>1){
  #pragma omp parallel proc_bind(spread)
  {
    #pragma omp for schedule(guided)
    for(int i=0;i < N;i++){
      int start = row[i];
      int end = row[i + 1];
      for(int j=start ; j< end;j++){
        // Set the forbiddensssssss
        if( (colour[col[j]] == colour[i]) && (i < col[j]) ){
          #pragma omp critical
          {
            errors[err] = i;
            err++;
          }
        }
      }
    }
  }
 }
}
// ===================  PARALLLEL REGION ENDS HERE   ===========================
end = omp_get_wtime();
int max =  GraphColorChecker(row,col,colour,N);
cout << filename  << "," << t <<","<<  end - start << "," << max << endl;
}
return 0;
}

