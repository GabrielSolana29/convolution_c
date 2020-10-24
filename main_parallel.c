// Code by Gabriel Solana Lavalle ID 164796

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
// Library to load and save image stb_image
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
// load omp library
#include <omp.h>

// when running the code, a gray scale image, and a filtered image will be saved as .png files on the folder of the project

main(int argc,char*argv[]) {

	//
   // receive arguments from bash
   /*
   char *noThreads_a = argv[1];
   char *image_select_a = argv[2];
   char *kernel_select_a = argv[3];
   char *schedule_select_a = argv[4];
   char *noIterations_a = argv[5];
  // convert arguments from char to int
	// Parameters that can be changed in bash.
   int image_select, kernel_select,noThreads,schedule_select,noIterations;
	// image 1 = lena, image 2 = buildings, image 3 landscape
  image_select = atoi(image_select_a);
	// kernel 1 = random, kenrel 2 = laplacian, kernel 3 = low pass
	kernel_select = atoi(kernel_select_a);
	// Number of threads for the execution
	noThreads = atoi(noThreads_a);
  // Schedule select
  schedule_select = atoi(schedule_select_a);
  // No iterations
  noIterations = atoi(noIterations_a);
  // Set the number of threads
	omp_set_num_threads(noThreads);
*/

	int noThreads = 24;
	int image_select = 3;
	int kernel_select = 2;
    int schedule_select = 2;
    int noIterations = 10;
    omp_set_num_threads(noThreads);


	// LOAD IMAGE PROCESS
	int width, height, original_no_channels;
	int desired_no_channels = 3;

  // Select the name of the image
	char name[13];

	if (image_select == 1)
	{
		strcpy(name,"Lena123456.png");

	}else if (image_select == 2){
		strcpy(name,"train_rail.jpg");

	}else if(image_select == 3)
	{
		strcpy(name,"landscape.jpg");

	}

	unsigned char *image= stbi_load(name,
									&width,
									&height,
									&original_no_channels,
									desired_no_channels);

	if(image == NULL){printf("Error in loading the image \n");}



	int kernel_size = 3;
	int image_size = width * height * original_no_channels;
	int grey_image_size = width * height;
	unsigned char *grey_image = malloc(grey_image_size);

	if(grey_image == NULL){
		printf("\nUnable to allocate memory for the grey image \n");
		exit(1);
	}


	unsigned char *pg = grey_image;
	int grey_channels = 1;


	//LUMINOSITY METHOD TO CONVERT IMAGE TO GRAY LEVELS

	unsigned char *grey_image_luminosity = malloc(grey_image_size);
	unsigned char *pg_luminosity = grey_image_luminosity;
	unsigned char *p2 = image;

	for(p2,pg_luminosity;p2 != image + image_size; p2+= original_no_channels, pg_luminosity += grey_channels){
		*pg_luminosity = (uint8_t) (*p2 * .21 + *(p2+1) * .72 + *(p2+2) * .07);
	}

	//SAVE GRAY LEVEL IMAGE
	stbi_write_png("greyImg_luminosity.png", width, height, grey_channels, grey_image_luminosity, width * grey_channels);

	//CONVOLUTION PROCESS

		int width_L, height_L, no_channels_L;
		int desired_no_channels_L = 1;

		unsigned char *luminosity_image = stbi_load("greyImg_luminosity.png",
											&width_L,
											&height_L,
											&no_channels_L,
											desired_no_channels_L);

		float kernel[kernel_size][kernel_size];
	    int i, j;
	    srand(time(NULL));

	    if(kernel_select == 1){
	    	//CREATE A KERNEL WITH RANDOM VALUES
	    	for(i = 0; i<kernel_size; i++){
		        for(j = 0; j<kernel_size; j++){
		            kernel[i][j] = (rand()%100) /100.00;
		        }
		    }
		}else if(kernel_select ==2){
			//LAPLACIAN KERNEL

			float a_temp[3][3] ={{0,-1,0},
								{-1,4,-1},
								{0,-1,0}};

			memcpy(kernel, a_temp, sizeof(kernel));


		}else if(kernel_select == 3){
			//LOW PASS FILTER KERNEL

			float a_temp[3][3] ={{.1111,.1111,.1111},
								{.1111,.1111,.1111},
								{.1111,.1111,.1111}};
			memcpy(kernel, a_temp, sizeof(kernel));

		}

		//CONVOLUTION OPERATION

		int grey_image_size_L = width_L * height_L;
		unsigned char *filtered_img = malloc(grey_image_size_L);

		if(filtered_img == NULL){
            printf("\nUnable to allocate memory for the grey image \n");
            exit(1);
		}


		unsigned char *pg_filt_img = filtered_img;
		unsigned char *cont = luminosity_image;




    int l;
    double suma_iteration=0;
        // Beging for
      for(l = 0; l<noIterations;l++){
            //START COUNTING THE EXECUTION TIME
            double t0 = omp_get_wtime();
            #pragma omp parallel for shared(cont,pg_filt_img,width_L,kernel_size,kernel,luminosity_image) schedule(dynamic, 256)
            for(i=0; i<grey_image_size_L ; i++){

                    //if((int)(cont+i-1) < (luminosity_image + width_L) ||  luminosity_image + (grey_image_size_L - width_L-1) < (int)(cont+i-1) || (i) % width_L == 0 || (i) % width_L-1 == 0){
                    if((cont+i-1) < (luminosity_image + width_L) ||  luminosity_image + (grey_image_size_L - width_L-1) < (cont+i-1) || (i) % width_L == 0 || (i) % width_L-1 == 0){
                    // it is at the top, bottom, right or left edge of the image
                        *(pg_filt_img+i-1) = (uint8_t) 0;
                    }else
                    {
                    // perform convolution with the kernel here
                        int f,k;
                        float suma = 0;
                        for(f = 0; f<kernel_size; f+=1){
                            for(k =0;k<kernel_size;k+=1){

                                int centro_kernel = f/kernel_size;
                                int rowKernel = ((f+1) - (centro_kernel + 1))*width_L;
                                int colKernel = (k+1) - (centro_kernel + 1);
                                suma += kernel[f][k] * *((cont+i-1) + colKernel + rowKernel);

                            }
                        }
                        *(pg_filt_img+i-1) = (uint8_t) suma;
                    }
            }// End for pragma
            	// STOP THE COUNTER
	            double t1 = omp_get_wtime();
              suma_iteration += (t1-t0);
      }// End for iterations


	// PRINT THE ELAPSED TIME
	printf("Time_elapsed, %lf\n", (suma_iteration/noIterations));

	// SAVE FILTERED IMAGE
	stbi_write_png("Filtered_image.png", width_L, height_L, grey_channels, filtered_img, width_L * grey_channels);

	// FREE THE MEMORY USED BY THE IMAGES
	stbi_image_free(image);
	stbi_image_free(grey_image);
	stbi_image_free(grey_image_luminosity);
	stbi_image_free(filtered_img);

    return 0;
}
