#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <time.h>


int main(int argc, char *argv[]){
    int id,p,complete;                                                                                                                                                                                       
    MPI_Init(&argc,&argv);                                                                                                  
	MPI_Comm_rank(MPI_COMM_WORLD,&id);                                                                                      
	MPI_Comm_size(MPI_COMM_WORLD,&p);

    int colA,rowA,colB,rowB;
    double **mA;
    double **mB;
    double **ans;

    FILE *fa,*fb;
	fa = fopen("matAlarge.txt","r");
	fb = fopen("matBlarge.txt","r");
	
	fscanf(fa,"%d",&colA);
	fscanf(fa,"%d",&rowA);
	
	fscanf(fb,"%d",&colB);
	fscanf(fb,"%d",&rowB);

    if(rowA != colB) return 0;

    int size = colA/p;
    int length = size*rowA;

    ans = (double**)malloc(colA * sizeof(double));
        for(int i = 0; i < colA; i++){
            ans[i] = (double (*))malloc(sizeof(double) * rowB);
        }  
    //printf("\ninit id:%d\n",id);

    if(id==0){
        double start,end;
        mA = (double**)malloc(colA * sizeof(double));
        for(int i = 0; i < colA; i++){
            mA[i] = (double (*))malloc(sizeof(double) * rowA);
            for(int j = 0; j < rowA; j++){
                fscanf(fa,"%lf",&mA[i][j]);
                
            }
        }
        fclose(fa);

        mB = (double**)malloc(colB * sizeof(double));
        for(int i = 0; i < colB; i++){
            mB[i] = (double (*))malloc(sizeof(double) * rowB);
            for(int j = 0; j < rowB; j++){
                fscanf(fb,"%lf",&mB[i][j]);
            }
        }
        fclose(fb);
        //printf("read file\n A:%lf  B:%lf",mA[0][0],mB[0][0]);

        start = MPI_Wtime();
        
        for(int i = 0 ; i < colB; i++){
            MPI_Bcast(&mB[i][0],rowB, MPI_DOUBLE, 0, MPI_COMM_WORLD);
        }
        //printf("\nbcast\n id:%d  B:%lf\n",id,mB[0][0]);

        int index;

        index = 0;
        for(int i = 1; i < p; i++){
            for(int j = 0; j < size; j++){
                MPI_Send(&mA[index][0], rowA, MPI_DOUBLE, i , 4, MPI_COMM_WORLD);
                index++;
            }
        }
        //printf("\nsend\n id:%d index:%d\n",id,index);

        for(int i = index; i < colA; i++){
            for(int j = 0; j < rowB; j++){
                ans[i][j] = 0;
                for(int k = 0; k < rowA; k++){
                    ans[i][j] += mA[i][k] * mB[k][j];
                }
            }
        }
        //printf("\ncal\n ans:%lf\n",ans[407][312]);

        index = 0;
        for(int i = 1; i < p; i++){
            for(int j = 0; j < size; j++){
                MPI_Recv(&ans[index][0], rowA, MPI_DOUBLE, i , 4, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                index++;
            }
        }
        //printf("\nrec\n id:%d  ans:%lf\n",id,ans[0][0]);

        end = MPI_Wtime();
        printf("\ntime = %lf sec\n",end-start);

        FILE *fp;
		fp=fopen("Lab4_outputLarge.txt","w");
		if(fp!=NULL){
			printf("\nStart Writing\n");
			fprintf(fp,"%d %d\n",colA ,rowB);
			for(int i=0; i<colA; i++){
                for(int j=0; j<rowB; j++){
                    fprintf(fp,"%.0lf ", ans[i][j]);
                }
				fprintf(fp,"\n");
			}
			printf("\nFinished\n");
		}
		fclose(fp);

    }
    else{
        mA = (double**)malloc(colA * sizeof(double));
        for(int i = 0; i < colA; i++){
            mA[i] = (double (*))malloc(sizeof(double) * rowA);
        }

        mB = (double**)malloc(colB * sizeof(double));
        for(int i = 0; i < colB; i++){
            mB[i] = (double (*))malloc(sizeof(double) * rowB);
        }

        for(int i = 0 ; i < colB; i++){
            MPI_Bcast(&mB[i][0],rowB, MPI_DOUBLE, 0, MPI_COMM_WORLD);
        }
        //printf("\nbcast\n id:%d  B:%lf\n",id,mB[0][0]);

        for(int i = 0; i < size; i++){
            MPI_Recv(&mA[i][0], rowA, MPI_DOUBLE, 0 , 4, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }
        //printf("\nrecv\n id:%d  A:%lf\n",id,mA[101][310]);

        for(int i = 0; i < size; i++){
            for(int j = 0; j < rowB; j++){
                ans[i][j] = 0;
                for(int k = 0; k < rowA; k++){
                    ans[i][j] += mA[i][k] * mB[k][j];
                }
            }
        }
        //printf("\ncalc\n id:%d  ans:%lf\n",id,ans[0][0]);

        for(int i = 0; i < size; i++){
            MPI_Send(&ans[i][0],rowA, MPI_DOUBLE, 0 , 4, MPI_COMM_WORLD);
        }
        //printf("\nsend\n id:%d  ans:%lf\n",id,ans[0][0]);

    }

    
    MPI_Finalize();

    return 0;   
}