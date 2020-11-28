#include <stdio.h>
#include <stdlib.h>


// a contains the image, b contains the kernel filter, and r is the matrix where the answer will be returned
__global__ void kernelConvolution(double *a, double *b, double *r, int size_col, int size_row, int size, int kernel_size) {
    int i = threadIdx.x+blockDim.x*blockIdx.x;
    int j = threadIdx.y+blockDim.y*blockIdx.y;
    int k,l;        
    int pos = i+(j*size_col);
    double sum = 0;        
    int cont = 0;
    
    // Iterate throught the kernel filter
    for (l = 0; l < kernel_size; l++){              
        for (k = 0; k < kernel_size; k++){                              
            sum += a[(pos+l)+(k*size_row)] * b[cont];          
            cont++;
        }
    }    
    // Save the value of the pixel in the vector r "result"
    r[i+(j*size_row)] = sum;         
}

float convolutionDeviceKernel(double *a, double *b, double *r, int size_col, int size_row, int kernel_size) {
    double *aD, *bD,*rD;    
    int size=size_col*size_row*sizeof(double); 
    int size_k = kernel_size*kernel_size*sizeof(double);
    cudaEvent_t start, stop; 
    float t; 
    
    // Define the dimensions of the blocks and kernels
    dim3 bloques((int)floor((double)size_row/kernel_size*1.0),(int)floor((double)size_col/kernel_size*1.0));     
    dim3 hilos(kernel_size,kernel_size); 
    
    cudaEventCreate(&start); cudaEventCreate(&stop); 
    
    cudaMalloc(&aD, size);cudaMalloc(&rD, size); cudaMalloc(&bD, size_k); 
    
    // Obtain the values for a and b from the host
    cudaMemcpy(aD, a, size, cudaMemcpyDefault);    
    cudaMemcpy(bD, b, size_k , cudaMemcpyDefault);     
    
    // Start counting the execution time
    cudaEventRecord(start, 0); 
    
    // Convolucion
    kernelConvolution<<<bloques, hilos>>>(aD, bD, rD, size_col,size_row,size,kernel_size);
    
    // Stop counting the execution time       
    cudaEventRecord(stop, 0); 
    
    // Copy to host 
    cudaMemcpy(a, aD,size, cudaMemcpyDefault);
    cudaMemcpy(b, bD,size_k, cudaMemcpyDefault);
    cudaMemcpy(r, rD,size, cudaMemcpyDefault);
  
    // Free memory
    cudaFree(aD); cudaFree(bD); cudaFree(rD); 
     
    cudaEventSynchronize(stop);

    cudaEventElapsedTime(&t, start, stop); 
    cudaEventDestroy(start); cudaEventDestroy(stop); 
    // Return the time elapsed
    return t; 
}

////////////// MAIN ///////////////////////////////////

int main(int argc, char **argv) {
    int size_col;
    int size_row; 
    int d=0, kl=0;     
    int size_k = 9*sizeof(double);
    float t; 
  	int i,j;
  	double *tmp;
  	double *tmp2;
    // a=image, b=kernel filter, r = result image
    double *a,*b,*r;
   	// Select the parameters
	  int image_select = 2;
	  int kernel_select = 3;    
    int noIterations = 10;
      
   // Receive parameters from the console   
  	if (argc > 1){
     d = atoi(argv[1]);
     } 
    if (argc > 2){ 
     image_select = atoi(argv[2]);
     }	     
    if (argc > 3){ 
     kernel_select = atoi(argv[3]);
     }	
    
   // Select in which card the code is going to be executed
    cudaSetDevice(d);
    
   // Select the image    
   char name[13];
    if (image_select == 1){
	    strcpy(name,"lena.txt");
      size_col = 512;
      size_row = 512;
	  }else if (image_select == 2){
	  	strcpy(name,"tran.txt");
      size_col = 1200;
      size_row = 800;
	  }else if(image_select == 3){
	    strcpy(name,"land.txt");
      size_col = 3840;
      size_row = 2160;
	  }
             
    // Allocate memory in the device
    int size = size_col*size_row*sizeof(double); 
    
    // Image
    cudaHostAlloc(&a, size , cudaHostAllocDefault);
    // Kernel Filter
    cudaHostAlloc(&b, size_k , cudaHostAllocDefault);
    // Result
    cudaHostAlloc(&r, size , cudaHostAllocDefault);       
        
    // Load the kernel filter
    int kernel_size = 3;
		double kernel[kernel_size][kernel_size];       
    srand(time(NULL));
    
    if(kernel_select == 1){
    	//CREATE A KERNEL FILTER WITH RANDOM VALUES
     double *temp3=b;
    	for(i = 0; i<kernel_size; i++){
	        for(j = 0; j<kernel_size; j++){
	            *temp3 = (rand()%100) /100.00;
               temp3++;
	        }
	    }      
  	}else if(kernel_select ==2){
  		//LAPLACIAN KERNEL FILTER            
      double kernel_vec[9] ={0,-1,0,-1,4,-1,0,-1,0};
      double *temp3 =b;
      for(i =0;i<9;i++){
        *temp3 = kernel_vec[i];
        temp3++;
      }         
  	}else if(kernel_select == 3){
  		//LOW PASS KERNEL FILTER  
  		 double kernel_vec[9] ={.1111,.1111,.1111,.1111,.1111,.1111,.1111,.1111,.1111};
       double *temp3 = b;
       for(i =0;i<9;i++){
          *temp3 = kernel_vec[i];
           temp3++;
       }  		
  	}        
  	         	
  	// Reading the filter from file
    FILE *img;
  	img = fopen(name,"r");
  	
  	if(img==NULL){
  		printf("Error loading image\n");
  		return 0;
  	}
   
    // Scan image  
  	for(i=0, tmp=a; i<size_col*size_row;++i){		
  		fscanf(img,"%lf",tmp++);  		
  	}
  	
    // Kernel execution    
    float sum = 0;
    float s_time = 0;
    
    for (i=0; i<noIterations;i++){ 
  	  t = convolutionDeviceKernel(a,b,r,size_col,size_row,kernel_size);
       sum += t;
    }
    
    s_time = sum/noIterations;
    // Print the execution time
  	printf("El tiempo de ejecucion es %f sg\n", s_time/1000.0); 	    
    
  	// Write the resulting image in a CSV file
  	FILE *nuevo;
  	nuevo = fopen("filtered_image.csv","w");
  	
  	if(nuevo==NULL){
  		printf("Error loading filter\n");
  		return 0;
  	}
			
  	for(i=1, tmp2=r; i<size_col*size_row;++i){  		
  		if(i%size_col*size_row==0 && i>0){
  			fprintf(nuevo,"%lf",*tmp2);
  			fprintf(nuevo,"\n");												
  		}else
  		{
  			fprintf(nuevo,"%lf,",*tmp2);      
  		}  								  			
  		tmp2++;		
  	}	   
  	fprintf(nuevo,"%lf",*tmp2);
   
    // Close the files img and nuevo
  	fclose(img);
  	fclose(nuevo);	
    // Free memory from the device
    cudaFreeHost(a);    	
 		cudaFreeHost(b);
		cudaFreeHost(r);
}
