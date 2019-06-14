/* 
 * trans.c - Matrix transpose B = A^T
 *
 * Each transpose function must have a prototype of the form:
 * void trans(int M, int N, int A[N][M], int B[M][N]);
 *
 * A transpose function is evaluated by counting the number of misses
 * on a 1KB direct mapped cache with a block size of 32 bytes.
 */ 
#include <stdio.h>
#include "cachelab.h"

int is_transpose(int M,int N,int A[N][M],int B[M][N]);
void transe32(int M,int N,int A[N][M],int B[M][N]);
void transe64(int M,int N,int A[N][M],int B[M][N]);
void transe61(int M,int N,int A[N][M],int B[M][N]);
/* 
 * transpose_submit - This is the solution transpose function that you
 *     will be graded on for Part B of the assignment. Do not change
 *     the description string "Transpose submission", as the driver
 *     searches for that string to identify the transpose function to
 *     be graded. 
 */
char transpose_submit_desc[] = "Transpose submission";
void transpose_submit(int M, int N, int A[N][M], int B[M][N])
{
	if(M == 32 && N == 32) transe32(M,N,A,B);
	else if(M == 64 && N == 64) transe64(M,N,A,B);
	else if(M == 61 && N == 67) transe61(M,N,A,B);
}

/* 
 * You can define additional transpose functions below. We've defined
 * a simple one below to help you get started. 
 */ 

/* 
 * trans - A simple baseline transpose function, not optimized for the cache.
 */
char trans_desc[] = "Simple row-wise scan transpose";
void trans(int M, int N, int A[N][M], int B[M][N])
{
	int i, j, tmp;

	for (i = 0; i < N; i++) {
		for (j = 0; j < M; j++) {
			tmp = A[i][j];
			B[j][i] = tmp;
		}
	}    

}

/*
 * registerFunctions - This function registers your transpose
 *     functions with the driver.  At runtime, the driver will
 *     evaluate each of the registered functions and summarize their
 *     performance. This is a handy way to experiment with different
 *     transpose strategies.
 */
void registerFunctions()
{
	/* Register your solution function */
	registerTransFunction(transpose_submit, transpose_submit_desc); 

	/* Register any additional transpose functions */
	registerTransFunction(trans, trans_desc); 

}

/* 
 * is_transpose - This helper function checks if B is the transpose of
 *     A. You can check the correctness of your transpose by calling
 *     it before returning from the transpose function.
 */
int is_transpose(int M, int N, int A[N][M], int B[M][N])
{
	int i, j;

	for (i = 0; i < N; i++) {
		for (j = 0; j < M; ++j) {
			if (A[i][j] != B[j][i]) {
				return 0;
			}
		}
	}
	return 1;
}

void transe32(int M, int N, int A[N][M], int B[M][N]){
	int blockSize = 8;
	int i,j,i1,j1;
	for(i=0;i<N;i+=blockSize){
		for(j=0;j<N;j+=blockSize){
			for(i1=i;i1<i+blockSize && i1<N;i1++){
				for(j1=j;j1<j+blockSize && j1<M;j1+=blockSize){
					int t1,t2,t3,t4,t5,t6,t7,t8;
					t1 = A[i1][j1+0];
					t2 = A[i1][j1+1];
					t3 = A[i1][j1+2];
					t4 = A[i1][j1+3];
					t5 = A[i1][j1+4];
					t6 = A[i1][j1+5];
					t7 = A[i1][j1+6];
					t8 = A[i1][j1+7];

					B[j1+0][i1] = t1;
					B[j1+1][i1] = t2;
					B[j1+2][i1] = t3;
					B[j1+3][i1] = t4;
					B[j1+4][i1] = t5;
					B[j1+5][i1] = t6;
					B[j1+6][i1] = t7;
					B[j1+7][i1] = t8;

				}
			}
		}
	}
}
void transe64(int M, int N, int A[N][M], int B[M][N]){
	int blockSize = 8;
	int i,j;
	int i1,j1;
	int t0,t1,t2,t3,t4,t5,t6,t7;
	for (i=0;i<N;i+=blockSize) {
		for (j=0;j<M;j+=blockSize) {

			if(i!=j){
				for(i1=i;i1<i+4;i1++){
					t0 = A[i1][j+0];
					t1 = A[i1][j+1];
					t2 = A[i1][j+2];
					t3 = A[i1][j+3];

					B[j+0][i1] = t0;
					B[j+1][i1] = t1;
					B[j+2][i1] = t2;
					B[j+3][i1] = t3;
				}
			}
			else{
				for (i1=i;i1<i+4;i1++) {
					for (j1=j;j1<j+4;j1++) {
						if (j1 == i1) {
							t0 = A[i1][i1];
						}
						else {
							B[j1][i1] = A[i1][j1];
						}
					}
					B[i1][i1] = t0;
				}
			}

			for (i1=i;i1<i+4;i1++) {
				for (j1=j;j1<j+4;j1++) {
					B[j1][i1+4] = A[i1][j1+4];
				}
			}

			for (i1=i;i1<i+4;i1++) {

				t0 = A[i+4][i1-i+j];
				t1 = A[i+5][i1-i+j];
				t2 = A[i+6][i1-i+j];
				t3 = A[i+7][i1-i+j];

				t4 = B[i1-i+j][i+4];
				t5 = B[i1-i+j][i+5];
				t6 = B[i1-i+j][i+6];
				t7 = B[i1-i+j][i+7];

				B[i1-i+j][i+4] = t0;
				B[i1-i+j][i+5] = t1;
				B[i1-i+j][i+6] = t2;
				B[i1-i+j][i+7] = t3;

				B[i1-i+j+4][i+0] = t4;
				B[i1-i+j+4][i+1] = t5;
				B[i1-i+j+4][i+2] = t6;
				B[i1-i+j+4][i+3] = t7;

			}
			if (i != j) {
				for (i1=i+4;i1<i+blockSize;i1++) {

					t0 = A[i1][j+4];
					t1 = A[i1][j+5];
					t2 = A[i1][j+6];
					t3 = A[i1][j+7];

					B[j+4][i1] = t0;
					B[j+5][i1] = t1;
					B[j+6][i1] = t2;
					B[j+7][i1] = t3;

				}
			}
			else {	
				for (i1 = i+4;i1<i+blockSize;i1++) {
					for (j1=j+4;j1<j+blockSize;j1++) {
						if (j1 == i1) {
							t0 = A[i1][i1];
						}
						else {
							B[j1][i1] = A[i1][j1];
						}
					}
					B[i1][i1] = t0;
				}
			}
		}
	}
}
void transe61(int M, int N, int A[N][M], int B[M][N]){
	int blockSize = 23;
	int i,j,i1,j1;
	for(i=0;i<N;i+=blockSize){
		for(j=0;j<M;j+=blockSize){
			for(i1=i;i1<i+blockSize && i1<N;i1++){
				for(j1=j;j1<j+blockSize && j1<M;j1++){
					B[j1][i1]= A[i1][j1];
				}
			}
		}
	}
}
